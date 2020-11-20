// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_transit_xdp.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief Implements the Transit XDP program (switching and routing logic)
 *
 * @copyright Copyright (c) 2019 The Authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <linux/bpf.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_vlan.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/pkt_cls.h>
#include <linux/socket.h>
#include <linux/tcp.h>
#include <linux/types.h>
#include <linux/udp.h>
#include <stddef.h>
#include <string.h>

#include "extern/bpf_endian.h"
#include "extern/bpf_helpers.h"

#include "trn_datamodel.h"
#include "trn_transit_xdp_maps.h"
#include "trn_kern.h"

int _version SEC("version") = 1;

static __inline int trn_rewrite_remote_mac(struct transit_packet *pkt)
{
	/* The TTL must have been decremented before this step, Drop the
	packet if TTL is zero */
	if (!pkt->ip->ttl)
		return XDP_DROP;

	struct endpoint_t *remote_ep;
	struct endpoint_key_t epkey;
	epkey.tunip[0] = 0;
	epkey.tunip[1] = 0;
	epkey.tunip[2] = pkt->ip->daddr;
	/* Get the remote_mac address based on the value of the outer dest IP */
	remote_ep = bpf_map_lookup_elem(&endpoints_map, &epkey);

	if (!remote_ep) {
		bpf_debug("[Transit:%d:] DROP: "
			  "Failed to find remote MAC address\n",
			  __LINE__);
		return XDP_DROP;
	}

	trn_set_src_mac(pkt->data, pkt->eth->h_dest);
	trn_set_dst_mac(pkt->data, remote_ep->mac);
	return XDP_TX;
}

static __inline void trn_update_ep_host_cache(struct transit_packet *pkt,
					      __be64 tunnel_id,
					      __u32 inner_src_ip)
{
	/* If RTS option is present, it always refer to the source endpoint's host.
	 * If the source endpoint is not known to this host, cache the host ip/mac in the
	 * en_host_cache.
	*/

	struct endpoint_t *src_ep;
	struct endpoint_key_t src_epkey;

	if (pkt->rts_opt->type == TRN_GNV_RTS_OPT_TYPE) {
		__builtin_memcpy(&src_epkey.tunip[0], &tunnel_id,
				 sizeof(tunnel_id));
		src_epkey.tunip[2] = inner_src_ip;
		src_ep = bpf_map_lookup_elem(&endpoints_map, &src_epkey);

		if (!src_ep) {
			/* Add the RTS info to the ep_host_cache */
			bpf_map_update_elem(&ep_host_cache, &src_epkey,
					    &pkt->rts_opt->rts_data.host, 0);
		}
	}
}

static __inline int trn_decapsulate_and_redirect(struct transit_packet *pkt,
						 int ifindex)
{
	int outer_header_size = sizeof(*pkt->geneve) + pkt->gnv_opt_len +
				sizeof(*pkt->udp) + sizeof(*pkt->ip) +
				sizeof(*pkt->eth);

	if (bpf_xdp_adjust_head(pkt->xdp, 0 + outer_header_size)) {
		bpf_debug(
			"[Transit:%d:0x%x] DROP: failed to adjust packet head.\n",
			__LINE__, bpf_ntohl(pkt->itf_ipv4));
		return XDP_DROP;
	}

	bpf_debug("[Transit:%d:0x%x] REDIRECT: itf=[%d].\n", __LINE__,
		  bpf_ntohl(pkt->itf_ipv4), ifindex);

	return bpf_redirect_map(&interfaces_map, ifindex, 0);
}

static __inline int trn_process_inner_ip(struct transit_packet *pkt)
{
	pkt->inner_ip = (void *)pkt->inner_eth + pkt->inner_eth_off;
	__u32 ipproto;

	if (pkt->inner_ip + 1 > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	/* For whatever compiler reason, we need to perform reverse flow modification
	 in this function instead of trn_switch_handle_pkt so we keep the orig_src_ip */
	__u32 orig_src_ip = pkt->inner_ip->saddr;

	pkt->inner_ipv4_tuple.saddr = pkt->inner_ip->saddr;
	pkt->inner_ipv4_tuple.daddr = pkt->inner_ip->daddr;
	pkt->inner_ipv4_tuple.protocol = pkt->inner_ip->protocol;
	pkt->inner_ipv4_tuple.sport = 0;
	pkt->inner_ipv4_tuple.dport = 0;

	if (pkt->inner_ipv4_tuple.protocol == IPPROTO_TCP) {
		pkt->inner_tcp = (void *)pkt->inner_ip + sizeof(*pkt->inner_ip);

		if (pkt->inner_tcp + 1 > pkt->data_end) {
			bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n",
				  __LINE__, bpf_ntohl(pkt->itf_ipv4));
			return XDP_ABORTED;
		}

		pkt->inner_ipv4_tuple.sport = pkt->inner_tcp->source;
		pkt->inner_ipv4_tuple.dport = pkt->inner_tcp->dest;
	}

	if (pkt->inner_ipv4_tuple.protocol == IPPROTO_UDP) {
		pkt->inner_udp = (void *)pkt->inner_ip + sizeof(*pkt->inner_ip);

		if (pkt->inner_udp + 1 > pkt->data_end) {
			bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n",
				  __LINE__, bpf_ntohl(pkt->itf_ipv4));
			return XDP_ABORTED;
		}

		bpf_debug("[Scaled_EP:%d:0x%x] Process UDP \n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));

		pkt->inner_ipv4_tuple.sport = pkt->inner_udp->source;
		pkt->inner_ipv4_tuple.dport = pkt->inner_udp->dest;
	}

	/* Lookup the source endpoint*/
	__be64 tunnel_id = trn_vni_to_tunnel_id(pkt->geneve->vni);
	struct endpoint_t *src_ep;
	struct endpoint_key_t src_epkey;

	__builtin_memcpy(&src_epkey.tunip[0], &tunnel_id, sizeof(tunnel_id));
	src_epkey.tunip[2] = pkt->inner_ip->saddr;
	src_ep = bpf_map_lookup_elem(&endpoints_map, &src_epkey);

	return XDP_TX;
}

static __inline int trn_process_inner_arp(struct transit_packet *pkt)
{
	unsigned char *sha;
	unsigned char *tha = NULL;
	struct endpoint_t *ep;
	struct endpoint_key_t epkey;
	struct endpoint_t *remote_ep;
	__u32 *sip, *tip;
	__u64 csum = 0;

	pkt->inner_arp = (void *)pkt->inner_eth + sizeof(*pkt->inner_eth);

	if (pkt->inner_arp + 1 > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	if (pkt->inner_arp->ar_pro != bpf_htons(ETH_P_IP) ||
	    pkt->inner_arp->ar_hrd != bpf_htons(ARPHRD_ETHER)) {
		bpf_debug("[Transit:%d:0x%x] DROP: ARP unsupported protocol"
			  " or Hardware type for inner packet!\n",
			  __LINE__, bpf_ntohl(pkt->itf_ipv4));
		return XDP_DROP;
	}

	if (pkt->inner_arp->ar_op != bpf_htons(ARPOP_REPLY) &&
	    pkt->inner_arp->ar_op != bpf_htons(ARPOP_REQUEST)) {
		bpf_debug(
			"[Transit:%d:0x%x] DROP:"
			"Only ARP REPLY and REQUEST are supported, received [0x%x]\n",
			pkt->inner_arp->ar_op);
		return XDP_DROP;
	}

	if ((unsigned char *)(pkt->inner_arp + 1) > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	sha = (unsigned char *)(pkt->inner_arp + 1);

	if (sha + ETH_ALEN > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	sip = (__u32 *)(sha + ETH_ALEN);

	if (sip + 1 > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	tha = (unsigned char *)sip + sizeof(__u32);

	if (tha + ETH_ALEN > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	tip = (__u32 *)(tha + ETH_ALEN);

	if ((void *)tip + sizeof(__u32) > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	__be64 tunnel_id = trn_vni_to_tunnel_id(pkt->geneve->vni);

	__builtin_memcpy(&epkey.tunip[0], &tunnel_id, sizeof(tunnel_id));
	epkey.tunip[2] = *tip;
	ep = bpf_map_lookup_elem(&endpoints_map, &epkey);

	bpf_debug("[Transit:%d:0x%x] respond to ARP\n", __LINE__,
		  bpf_ntohl(pkt->itf_ipv4));

	/* Respond to ARP */
	pkt->inner_arp->ar_op = bpf_htons(ARPOP_REPLY);
	trn_set_arp_ha(tha, sha);
	trn_set_arp_ha(sha, ep->mac);

	__u32 tmp_ip = *sip;
	*sip = *tip;
	*tip = tmp_ip;

	/* Set the sender mac address to the ep mac address */
	trn_set_src_mac(pkt->inner_eth, ep->mac);

	if (ep->eptype == TRAN_SIMPLE_EP) {
		/*Get the remote_ep address based on the value of the outer dest IP */
		epkey.tunip[0] = 0;
		epkey.tunip[1] = 0;
		epkey.tunip[2] = ep->remote_ips[0];
		remote_ep = bpf_map_lookup_elem(&endpoints_map, &epkey);

		if (!remote_ep) {
			bpf_debug(
				"[Transit:%d:] (BUG) DROP: "
				"Failed to find remote MAC address of ep: 0x%x @ 0x%x\n",
				__LINE__, bpf_ntohl(*tip),
				bpf_ntohl(ep->remote_ips[0]));
			return XDP_DROP;
		}

		/* For a simple endpoint, Write the RTS option on behalf of the target endpoint */
		pkt->rts_opt->rts_data.host.ip = ep->remote_ips[0];
		__builtin_memcpy(pkt->rts_opt->rts_data.host.mac,
				 remote_ep->mac, 6 * sizeof(unsigned char));
	} else {
		bpf_debug("[Transit:%d:0x%x] skip RTS writing!\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		trn_reset_rts_opt(pkt);
	}

	/* We need to lookup the endpoint again, since tip has changed */
	epkey.tunip[2] = *tip;
	ep = bpf_map_lookup_elem(&endpoints_map, &epkey);

	return XDP_TX;
}

static __inline int trn_process_inner_eth(struct transit_packet *pkt)
{
	pkt->inner_eth = (void *)pkt->geneve + pkt->gnv_hdr_len;
	pkt->inner_eth_off = sizeof(*pkt->inner_eth);

	if (pkt->inner_eth + 1 > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	/* ARP */
	if (pkt->inner_eth->h_proto == bpf_htons(ETH_P_ARP)) {
		bpf_debug("[Transit:%d:0x%x] Processing ARP \n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return trn_process_inner_arp(pkt);
	}

	if (pkt->eth->h_proto != bpf_htons(ETH_P_IP)) {
		bpf_debug(
			"[Transit:%d:0x%x] DROP: unsupported inner packet: [0x%x]\n",
			__LINE__, bpf_ntohl(pkt->itf_ipv4),
			bpf_ntohs(pkt->eth->h_proto));
		return XDP_DROP;
	}

	bpf_debug("[Transit:%d:0x%x] Processing IP \n", __LINE__,
		  bpf_ntohl(pkt->itf_ipv4));
	return trn_process_inner_ip(pkt);
}

static __inline int trn_process_geneve(struct transit_packet *pkt)
{
	pkt->geneve = (void *)pkt->udp + sizeof(*pkt->udp);
	if (pkt->geneve + 1 > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	if (pkt->geneve->proto_type != bpf_htons(ETH_P_TEB)) {
		bpf_debug(
			"[Transit:%d:0x%x] PASS: unrecognized geneve proto_type: [0x%x]\n",
			__LINE__, bpf_ntohl(pkt->itf_ipv4),
			pkt->geneve->proto_type);
		return XDP_PASS;
	}

	pkt->gnv_opt_len = pkt->geneve->opt_len * 4;
	pkt->gnv_hdr_len = sizeof(*pkt->geneve) + pkt->gnv_opt_len;
	pkt->rts_opt = (void *)&pkt->geneve->options[0];

	if (pkt->rts_opt + 1 > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	if (pkt->rts_opt->opt_class != TRN_GNV_OPT_CLASS) {
		bpf_debug(
			"[Scaled_EP:%d:0x%x] ABORTED: Unsupported Geneve option class\n",
			__LINE__, bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	// TODO: process options
	pkt->scaled_ep_opt = (void *)pkt->rts_opt + sizeof(*pkt->rts_opt);

	if (pkt->scaled_ep_opt + 1 > pkt->data_end) {
		bpf_debug("[Scaled_EP:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	if (pkt->scaled_ep_opt->opt_class != TRN_GNV_OPT_CLASS) {
		bpf_debug(
			"[Scaled_EP:%d:0x%x] ABORTED: Unsupported Geneve option class\n",
			__LINE__, bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	return trn_process_inner_eth(pkt);
}

static __inline int trn_process_udp(struct transit_packet *pkt)
{
	/* Get the UDP header */
	pkt->udp = (void *)pkt->ip + sizeof(*pkt->ip);

	if (pkt->udp + 1 > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	if (pkt->udp->dest != GEN_DSTPORT) {
		bpf_debug("[Transit:%d:0x%x] PASS non-geneve packet \n",
			  __LINE__, bpf_ntohl(pkt->itf_ipv4));
		return XDP_PASS;
	}

	return trn_process_geneve(pkt);
}

static __inline int trn_process_ip(struct transit_packet *pkt)
{
	/* Get the IP header */
	pkt->ip = (void *)pkt->eth + pkt->eth_off;

	if (pkt->ip + 1 > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	if (pkt->ip->protocol != IPPROTO_UDP) {
		return XDP_PASS;
	}

	if (!pkt->ip->ttl)
		return XDP_DROP;

	/* Only process packets designated to this interface!
	 * In functional tests - relying on docker0 - we see such packets!
	 */
	if (pkt->ip->daddr != pkt->itf_ipv4) {
		bpf_debug(
			"[Transit:%d:0x%x] DROP: packet dst address [0x%x] mismatch interface address.\n",
			__LINE__, bpf_ntohl(pkt->itf_ipv4),
			bpf_ntohl(pkt->ip->daddr));
		return XDP_DROP;
	}

	return trn_process_udp(pkt);
}

static __inline int trn_process_eth(struct transit_packet *pkt)
{
	pkt->eth = pkt->data;
	pkt->eth_off = sizeof(*pkt->eth);

	if (pkt->data + pkt->eth_off > pkt->data_end) {
		bpf_debug("[Transit:%d:0x%x] ABORTED: Bad offset\n", __LINE__,
			  bpf_ntohl(pkt->itf_ipv4));
		return XDP_ABORTED;
	}

	if (pkt->eth->h_proto != bpf_htons(ETH_P_IP)) {
		return XDP_PASS;
	}

	return trn_process_ip(pkt);
}

SEC("transit")
int _transit(struct xdp_md *ctx)
{
	struct transit_packet pkt;
	pkt.data = (void *)(long)ctx->data;
	pkt.data_end = (void *)(long)ctx->data_end;
	pkt.xdp = ctx;

	struct tunnel_iface_t *itf;

	int k = 0;
	itf = bpf_map_lookup_elem(&interface_config_map, &k);

	if (!itf) {
		bpf_debug("[Transit:%d:] ABORTED: Bad configuration\n",
			  __LINE__);
		return XDP_ABORTED;
	}

	pkt.itf_ipv4 = itf->ip;
	pkt.itf_idx = itf->iface_index;

	int action = trn_process_eth(&pkt);

	/* The agent may tail-call this program, override XDP_TX to
	 * redirect to egress instead */
	if (action == XDP_TX)
		action = bpf_redirect_map(&interfaces_map, pkt.itf_idx, 0);

	if (action == XDP_PASS) {
		__u32 key = XDP_PASS_PROC;
		bpf_tail_call(pkt.xdp, &jmp_table, key);
		return xdpcap_exit(ctx, &xdpcap_hook, XDP_PASS);
	}

	if (action == XDP_DROP) {
		__u32 key = XDP_DROP_PROC;
		bpf_tail_call(pkt.xdp, &jmp_table, key);
		return xdpcap_exit(ctx, &xdpcap_hook, XDP_DROP);
	}

	if (action == XDP_TX) {
		__u32 key = XDP_TX_PROC;
		bpf_tail_call(pkt.xdp, &jmp_table, key);
		return xdpcap_exit(ctx, &xdpcap_hook, XDP_TX);
	}

	if (action == XDP_ABORTED)
		return xdpcap_exit(ctx, &xdpcap_hook, XDP_ABORTED);

	if (action == XDP_REDIRECT) {
		__u32 key = XDP_REDIRECT_PROC;
		bpf_tail_call(pkt.xdp, &jmp_table, key);
		return xdpcap_exit(ctx, &xdpcap_hook, XDP_REDIRECT);
	}

	return xdpcap_exit(ctx, &xdpcap_hook, XDP_PASS);
}

char _license[] SEC("license") = "GPL";

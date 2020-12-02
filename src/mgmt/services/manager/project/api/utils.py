# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Rio Zhu <@zzxgzgz>
#

from project.api.settings import MAC_PRIVATE
from functools import reduce


# Convert to GW array
def int_to_mac(macint):
    return ':'.join(['{}{}'.format(a, b)
                     for a, b
                     in zip(*[iter('{:012x}'.format(macint))]*2)])

def getGWsFromIpRange(firstIp,lastIp):
    start= ip_to_int(firstIp)
    end = ip_to_int(lastIp)
    gws = []
    for ipint in range(start,end+1):
        gw = {}
        gw["ip"] = ".".join(map(lambda n: str(ipint>>n & 0xFF), [24,16,8,0]))
        gw["mac"] = int_to_mac(ipint | MAC_PRIVATE)
        gws.append(gw.copy())
    return gws

def get_mac_from_ip(ip_string):
    start= ip_to_int(ip_string)
    mac = int_to_mac(start | MAC_PRIVATE)
    return mac

def ip_to_int(ip_string):
    return reduce(lambda a,b: a<<8 | b, map(int, ip_string.split(".")))

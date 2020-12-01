from project.api import settings
from functools import reduce

MAC_PRIVATE = 0x828300000000

# Convert to GW array
def int_to_mac(macint):
    return ':'.join(['{}{}'.format(a, b)
                     for a, b
                     in zip(*[iter('{:012x}'.format(macint))]*2)])

def getGWsFromIpRange(firstIp,lastIp):
    start= reduce(lambda a,b: a<<8 | b, map(int, firstIp.split(".")))
    end = reduce(lambda a,b: a<<8 | b, map(int, lastIp.split(".")))
    gws = []
    for ipint in range(start,end+1):
        gw = {}
        gw["ip"] = ".".join(map(lambda n: str(ipint>>n & 0xFF), [24,16,8,0]))
        gw["mac"] = int_to_mac(ipint | MAC_PRIVATE)
        gws.append(gw.copy())
    return gws

def extendVpcResp(vpc):
    respond = vpc.to_json()
    respond['port_ibo'] = settings.activeZgc["port_ibo"]
    respond['gws'] = getGWsFromIpRange(settings.activeZgc["ip_start"], settings.activeZgc["ip_end"])
    return respond


def get_mac_from_ip(ip_string):
    start= reduce(lambda a,b: a<<8 | b, map(int, ip_string.split('.')))
    mac = int_to_mac(start | MAC_PRIVATE)
    return mac
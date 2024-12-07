#ifndef VPN_MGR_PPPD_H
#define VPN_MGR_PPPD_H

#include "common.h"
#include "vpn_ctx.h"

#define PPPD_OPTIONS "nodetach nodefaultroute ipcp-accept-remote lcp-echo-interval 30 lcp-echo-failure 4"


STREAM *PPPDLaunch(TVpn *Vpn);
void PPPDProcess(TVpn *Vpn, STREAM *RemotePeer);

#endif

#ifndef VPN_MGR_PPPD_H
#define VPN_MGR_PPPD_H

#include "common.h"
#include "vpn_ctx.h"

//bauds: 57600, 115200, 230400, 460800, 921600
#define PPPD_OPTIONS "921600 nodetach nodefaultroute ipcp-accept-remote lcp-echo-interval 30 lcp-echo-failure 4"


STREAM *PPPDLaunch(TVpn *Vpn);
void PPPDProcess(TVpn *Vpn, STREAM *RemotePeer);

#endif

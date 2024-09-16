#ifndef VPNMGR_UPDOWN_H
#define VPNMGR_UPDOWN_H

#include "common.h"
#include "vpn_ctx.h"

void VpnUp(TVpn *Vpn, const char *Dev);
void VpnDown(TVpn *Vpn, const char *Dev);

#endif

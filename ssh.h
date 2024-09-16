#ifndef VPN_MGR_SSH_H
#define VPN_MGR_SSH_H

#include "common.h"
#include "vpn_ctx.h"


char *SSHHostLookupIP(char *IP, const char *Host);
int SSHVpnUp(TVpn *Vpn);
void SSHVpnTerminate();
int SSHVpnValidate(TVpn *Vpn);
STREAM *SSHVpnConnect(TVpn *Vpn, const char *Command);
int SSHVpnRunCommand(STREAM *S, const char *Cmd, int Flags);

#endif


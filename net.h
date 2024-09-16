#ifndef VPN_MGR_NET_H
#define VPN_MGR_NET_H

#include "vpn_ctx.h"


int NetDevExists(const char *Name);
char *NetDevFindNext(char *RetStr, const char *Prefix);
char *NetGetDefaultDev(char *RetStr);
char *NetGetDefaultIP(char *RetStr);
char *NetDevFindForIP(char *RetStr, const char *Prefix, const char *IP);
char *NetDevFindNextIP(char *RetStr, const char *Prefix);
void NetSetup(TVpn *Vpn, const char *Dev);
void NetShutdown(TVpn *Vpn, const char *Dev);


#endif

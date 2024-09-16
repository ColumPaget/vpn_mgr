#ifndef VPN_MGR_CONFIG_FILE_H
#define VPN_MGR_CONFIG_FILE_H

#include "common.h"
#include "vpn_ctx.h"

void ReadConfig(TVpn *Vpn);
void WriteConfig(TVpn *Vpn);
void AddConfig(TVpn *Vpn);
void DeleteConfig(TVpn *Vpn);
void ListConfigs(TVpn *Info);

#endif

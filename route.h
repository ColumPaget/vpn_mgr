#ifndef VPN_MGR_ROUTE_H
#define VPN_MGR_ROUTE_H

#include "common.h"

char *RouteGetDefault(char *RetStr);
char *RouteGetDefaultDev(char *RetStr);
void RouteSetupVpnServer(const char *DestAddress);
void RouteSetup(const char *DestAddress, const char *Dev);
char *DefaultDevGetMTU(char *RetStr);

#endif

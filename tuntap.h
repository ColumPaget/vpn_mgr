#ifndef VPN_MGR_TUN_H
#define VPN_MGR_TUN_H

#include "common.h"

int TunAvailable();
int TunSetup(const char *Type, const char *Name, const char *Address);
void TunShutdown(const char *Dev);


#endif

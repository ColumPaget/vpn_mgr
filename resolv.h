#ifndef VPN_MGR_RESOLV_H
#define VPN_MGR_RESOLV_H

#include "common.h"

int ResolvChange(const char *Nameservers);
int ResolvRestore();

#endif

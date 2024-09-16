#ifndef VPN_CTX_H
#define VPN_CTX_H

#include "common.h"

#define VPN_UDP 1
#define VPN_TCP 2
#define VPN_VERIFY_PEER 4

#define ACT_CONFIG  1
#define ACT_ADD     2
#define ACT_DELETE  4
#define ACT_LIST    16
#define ACT_CONNECT 8192
#define ACT_SERVER  16394
#define ACT_PROTOCOLS 32788

typedef struct
{
    int Action;
    int Flags;
    char *Name;
    char *Dev;
    char *RemoteDev;
    char *Transport;
    char *Server;
    char *ServerAddress;
    char *ConfFile;
    char *UpFile;
    char *DownFile;
    char *LocalAddress;
    char *RemoteAddress;
    char *AllowedIPs;
    char *RemoteAllowedIPs;
    char *UserName;
    char *Password;
    char *ClientCert;
    char *ClientKey;
    char *VerifyCert;
    char *PreSharedKey;
    char *PrivSepUser;
    char *PrivSepGroup;
    char *Ciphers;
    char *ClientID;
    char *DNS;
    char *PPPAuth;
    unsigned int MTU;
} TVpn;

TVpn *VpnCtxCreate(const char *Type, const char *Name, const char *Server);
void VpnCtxDestroy(void *p_Vpn);
int VpnCtxLoadConfig(TVpn *Vpn, const char *Path);

#endif

#include "vpn_ctx.h"
#include "net.h"


TVpn *VpnCtxCreate(const char *Action, const char *Name, const char *Server)
{
    TVpn *Vpn;

    Vpn=(TVpn *) calloc(1, sizeof(TVpn));

    if (strcmp(Action, "conf")==0) Vpn->Action=ACT_CONFIG;
    else if (strcmp(Action, "config")==0) Vpn->Action=ACT_CONFIG;
    else if (strcmp(Action, "add")==0) Vpn->Action=ACT_ADD;
    else if (strcmp(Action, "del")==0) Vpn->Action=ACT_DELETE;
    else if (strcmp(Action, "delete")==0) Vpn->Action=ACT_DELETE;
    else if (strcmp(Action, "server")==0) Vpn->Action=ACT_SERVER;
    else if (strcmp(Action, "list")==0) Vpn->Action=ACT_LIST;
    else if (strcmp(Action, "protocols")==0) Vpn->Action=ACT_PROTOCOLS;
    else Vpn->Action=ACT_CONNECT;

    switch (Vpn->Action)
    {
    case ACT_ADD:
        if (! StrValid(Server))
        {
            printf("ERROR: no URL given for add command\n");
            return(NULL);
        }
    }

    Vpn->Name=CopyStr(Vpn->Server, Name);
    Vpn->Server=CopyStr(Vpn->Server, Server);

    //set some default values
    Vpn->LocalAddress=CopyStr(Vpn->LocalAddress, "");
    Vpn->RemoteAddress=CopyStr(Vpn->RemoteAddress, "");
    Vpn->PrivSepUser=CopyStr(Vpn->PrivSepUser, "nobody");
    Vpn->PrivSepGroup=CopyStr(Vpn->PrivSepGroup, "nobody");
    if (StrValid(Name)) Vpn->UpFile=MCopyStr(Vpn->UpFile, "/etc/vpn_mgr/", Name, ".up:", NULL);
    Vpn->UpFile=MCatStr(Vpn->UpFile, "/etc/vpn_mgr/default.up", NULL);
    if (StrValid(Name)) Vpn->DownFile=MCopyStr(Vpn->DownFile, "/etc/vpn_mgr/", Name, ".down:", NULL);
    Vpn->DownFile=MCatStr(Vpn->DownFile, "/etc/vpn_mgr/default.down", NULL);


    return(Vpn);
}


void VpnCtxDestroy(void *p_Vpn)
{
    TVpn *Vpn;

    if (! p_Vpn) return;
    Vpn=(TVpn *) p_Vpn;

    Destroy(Vpn->Name);
    Destroy(Vpn->Server);
    Destroy(Vpn->Dev);
    Destroy(Vpn->RemoteDev);
    Destroy(Vpn->ConfFile);
    Destroy(Vpn->UpFile);
    Destroy(Vpn->DownFile);
    Destroy(Vpn->VerifyCert);
    Destroy(Vpn->AllowedIPs);
    Destroy(Vpn->RemoteAllowedIPs);
    Destroy(Vpn->ClientCert);
    Destroy(Vpn->ClientKey);
    free(Vpn);
}


int VpnCtxLoadConfig(TVpn *Vpn, const char *Path)
{
    char *Tempstr=NULL, *Key=NULL;
    int result=FALSE;
    const char *ptr;
    STREAM *S;

    S=STREAMOpen(Path, "r");
    if (S)
    {
        Vpn=VpnCtxCreate("", "", "");

        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripLeadingWhitespace(Tempstr);
            StripTrailingWhitespace(Tempstr);

            ptr=GetToken(Tempstr, "=", &Key, 0);
            if (strcasecmp(Key, "name")==0) Vpn->Name=CopyStr(Vpn->Name, ptr);
            if (strcasecmp(Key, "server")==0) Vpn->Server=CopyStr(Vpn->Server, ptr);
            if (strcasecmp(Key, "upfile")==0) Vpn->UpFile=CopyStr(Vpn->UpFile, ptr);
            if (strcasecmp(Key, "downfile")==0) Vpn->DownFile=CopyStr(Vpn->DownFile, ptr);
            if (strcasecmp(Key, "servercert")==0) Vpn->VerifyCert=CopyStr(Vpn->VerifyCert, ptr);
            if (strcasecmp(Key, "clientcert")==0) Vpn->ClientCert=CopyStr(Vpn->ClientCert, ptr);
            if (strcasecmp(Key, "clientkey")==0) Vpn->ClientKey=CopyStr(Vpn->ClientKey, ptr);
            Tempstr=STREAMReadLine(Tempstr, S);
        }

        STREAMClose(S);
    }

    Destroy(Tempstr);
    Destroy(Key);

    return(result);
}


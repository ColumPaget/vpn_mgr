#include "ppp.h"
#include "run_command.h"
#include "ssh.h"
#include "net.h"
#include "route.h"
#include "updown.h"



int PPPDTryConfigureEndpoint(TVpn *Vpn)
{
    char *Dev=NULL;
    int RetVal=FALSE;

    Dev=NetDevFindForIP(Dev, "ppp", Vpn->LocalAddress);
    if (StrValid(Dev))
    {
        TerminalPrint(Terminal, "~cPPP:~0 link ~gUP~0 local dev %s\n", Dev);
        RetVal=TRUE;
        VpnUp(Vpn, Dev);
    }

    Destroy(Dev);

    return(RetVal);
}


STREAM *PPPDLaunch(TVpn *Vpn)
{
    STREAM *PPPD=NULL;
    char *Cmd=NULL, *Tempstr=NULL;

    Cmd=MCopyStr(Cmd, "pppd", " ", PPPD_OPTIONS, NULL);
    if (! StrValid(Vpn->LocalAddress))
    {
        Vpn->LocalAddress=CopyStr(Vpn->LocalAddress, "172.16.0.2");
        Cmd=CatStr(Cmd, " ipcp-accept-local ");
    }

    if (! StrValid(Vpn->RemoteAddress))
    {
        Cmd=CatStr(Cmd, " ipcp-accept-remote ");
    }

    Cmd=MCatStr(Cmd, Vpn->LocalAddress, ":", Vpn->RemoteAddress, " ", NULL);

    if (StrValid(Vpn->PPPAuth)) Cmd=MCatStr(Cmd, " name ", Vpn->PPPAuth, NULL);

    if (Vpn->Flags & VPN_VERIFY_PEER) Cmd=CatStr(Cmd, " auth ");
    else Cmd=CatStr(Cmd, " noauth ");


    if (StrValid(Vpn->DNS) && (strcmp(Vpn->DNS, "peer")==0) ) Cmd=CatStr(Cmd, " usepeerdns ");

    if (StrValid(Vpn->Name) > 0) Cmd=MCatStr(Cmd, " linkname ", Vpn->Name, NULL);
    if (Vpn->MTU > 0)
    {
        Tempstr=FormatStr(Tempstr, "mtu %d ", Vpn->MTU);
        Cmd=CatStr(Cmd, Tempstr);
    }

    PPPD=RunCommandOpen(Cmd, CMD_SUDO | CMD_SU | CMD_NO_STDERR);
    if (! PPPD) TerminalPrint(Terminal, "~rFATAL:~0 failed to launch local PPPD\n");

    Destroy(Tempstr);
    Destroy(Cmd);

    return(PPPD);
}



void PPPDProcess(TVpn *Vpn, STREAM *RemotePeer)
{
    char *Tempstr=NULL;
    ListNode *Connections=NULL;
    STREAM *PPPD=NULL, *S;
    int EndpointConfigured=FALSE;
    struct timeval tv;
    int result;

    printf("Launch LOCAL pppd\n");
    fflush(NULL);

    PPPD=PPPDLaunch(Vpn);
    if (PPPD)
    {
        Connections=ListCreate();
        ListAddItem(Connections, PPPD);
        ListAddItem(Connections, RemotePeer);
        Tempstr=SetStrLen(Tempstr, 4096);

        tv.tv_sec=0;
        tv.tv_usec=300000;
        while (TRUE)
        {
            S=STREAMSelect(Connections, &tv);
            if (GlobalFlags & FLAG_EXIT) break;

            if (S==PPPD)
            {
                result=STREAMReadBytes(PPPD, Tempstr, 4096);
                if (result < 1)
                {
                    printf("PPPD: local endpoint disconnected\n");
                    break;
                }
                if (GlobalFlags & FLAG_DEBUG) fprintf(stderr, "PPPD: read from local: %s\n", Tempstr);
                STREAMWriteBytes(RemotePeer, Tempstr, result);
                STREAMFlush(RemotePeer);
            }

            if (S==RemotePeer)
            {
                result=STREAMReadBytes(RemotePeer, Tempstr, 4096);
                if (result < 1)
                {
                    printf("PPPD: remote endpoint disconnected\n");
                    break;
                }
                if (GlobalFlags & FLAG_DEBUG) fprintf(stderr, "PPPD: read from remote: %s\n", Tempstr);
                STREAMWriteBytes(PPPD, Tempstr, result);
                STREAMFlush(PPPD);
            }

            if (! EndpointConfigured) EndpointConfigured=PPPDTryConfigureEndpoint(Vpn);

            if (tv.tv_usec==0)
            {
                tv.tv_sec=0;
                tv.tv_usec=300000;
            }

        }

    }

    STREAMClose(PPPD);

    Destroy(Tempstr);
}




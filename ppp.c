#include "ppp.h"
#include "run_command.h"
#include "ssh.h"
#include "net.h"
#include "route.h"
#include "updown.h"



static int PPPDTryConfigureEndpoint(TVpn *Vpn)
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


static void PPPDHandleExit(STREAM *S)
{
    const char *ptr;
    int status;

    /*
    1   Pppd has detached, or otherwise the connection was successfully established and terminated at the peer's request.
    2   An immediately fatal error of some kind occurred, such as an essential system call failing, or running out of virtual memory.
    3   An error was detected in processing the options given, such as two mutually exclusive options being used.
    4   Pppd is not setuid-root and the invoking user is not root.
    5   The kernel does not support PPP, for example, the PPP kernel driver is not included or cannot be loaded.
    6   Pppd terminated because it was sent a SIGINT, SIGTERM or SIGHUP signal.
    7   The serial port could not be locked.
    8   The serial port could not be opened.
    9   The connect script failed (returned a non-zero exit status).
    10  The command specified as the argument to the pty option could not be run.
    11  The PPP negotiation failed, that is, it didn't reach the point where at least one network protocol (e.g. IP) was running.
    12  The peer system failed (or refused) to authenticate itself.
    13  The link was established successfully and terminated because it was idle.
    14  The link was established successfully and terminated because the connect time limit was reached.
    15  Callback was negotiated and an incoming call should arrive shortly.
    16  The link was terminated because the peer is not responding to echo requests.
    17  The link was terminated by the modem hanging up.
    18  The PPP negotiation failed because serial loopback was detected.
    19  The init script failed (returned a non-zero exit status).
    20  We failed to authenticate ourselves to the peer.
    */

    RunCommandCleanUp(S, 0);
    ptr=STREAMGetValue(S, "ExitStatus");

    if (StrValid(ptr))
    {
        status=atoi(ptr);

        switch (status)
        {
        case 2:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "fatal error: pppd encountered a system error");
            break;
        case 3:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "bad command-line options to pppd");
            break;
        case 4:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "insufficient permission: we are not root, nor suid-root");
            break;
        case 5:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "kernel does not support PPP");
            break;
        case 7:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "unable to lock serial port");
            break;
        case 8:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "unable to open serial port");
            break;
        case 9:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "connect script failed");
            break;
        case 10:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "pty command cound not be run");
            break;
        case 11:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "cannot negotiate a network protocol");
            break;
        case 12:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "cannot authenticate peer");
            break;
        case 13:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "link closed, idle timeout");
            break;
        case 14:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "link closed, connect time limit");
            break;
        case 18:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "serial line is looped back");
            break;
        case 19:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "init script failed");
            break;
        case 20:
            LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPD", "cannot authenticate ourselves to peer");
            break;
        }

    }
}


STREAM *PPPDLaunch(TVpn *Vpn)
{
    STREAM *PPPD=NULL;
    char *Cmd=NULL, *Tempstr=NULL;

    Cmd=FormatStr(Cmd, "pppd %d %s ", Vpn->LineSpeed, PPPD_OPTIONS);

    if (! StrValid(Vpn->LocalAddress))
    {
        Vpn->LocalAddress=CopyStr(Vpn->LocalAddress, "172.16.0.2");
        Cmd=CatStr(Cmd, " ipcp-accept-local ");
    }

    if (! StrValid(Vpn->RemoteAddress))
    {
        Cmd=CatStr(Cmd, " ipcp-accept-remote ");
    }

    Cmd=MCatStr(Cmd, " ", Vpn->LocalAddress, ":", Vpn->RemoteAddress, " ", NULL);

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

    LogEvent(VPN_LOG_SYSLOG | VPN_LOG_ERROR, "PPPDLaunch", "[%s]", Cmd);
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
                    LogEvent(VPN_LOG_SYSLOG | VPN_LOG_INFO, "PPPD", "local endpoint disconnected");
                    PPPDHandleExit(PPPD);
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
                    LogEvent(VPN_LOG_SYSLOG | VPN_LOG_INFO, "PPPD", "remote endpoint disconnected");
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




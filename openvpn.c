#include "openvpn.h"
#include "run_command.h"
#include "route.h"
#include "net.h"
#include "tuntap.h"
#include "updown.h"

static void OpenVpnGenPasswordFile(TVpn *Vpn)
{
    char *Tempstr=NULL;
    STREAM *S;

    Tempstr=MCopyStr(Tempstr, GetCurrUserHomeDir(), "/.config/vpn_mgr/", Vpn->Name, "/", Vpn->Name, ".auth", NULL);
    S=STREAMOpen(Tempstr, "w");
    if (S)
    {
        Tempstr=MCopyStr(Tempstr, Vpn->UserName, "\n", Vpn->Password, NULL);
        STREAMWriteLine(Tempstr, S);
        STREAMClose(S);
    }

    Destroy(Tempstr);
}


static void OpenVpnShutDown(TVpn *Vpn, const char *Dev)
{
    char *Cmd=NULL, *Tempstr=NULL;

    Cmd=MCopyStr(Cmd, GetCurrUserHomeDir(), "/.config/vpn_mgr/", Vpn->Name, "/", Vpn->Name, ".pid", NULL);
    Tempstr=ReadFile(Tempstr, Cmd);
    StripTrailingWhitespace(Tempstr);
    StripTrailingWhitespace(Tempstr);
    if (StrValid(Tempstr))
    {
        Cmd=MCopyStr(Cmd, "kill ", Tempstr, NULL);
        RunCommand(Cmd, CMD_ASROOT);
    }

		TunShutdown(Dev);
    Destroy(Tempstr);
    Destroy(Cmd);
}


static const char *OpenVpnReadLineSkipDateTime(const char *Line)
{
char *Token=NULL;
const char *ptr;

ptr=GetToken(Line, "\\S", &Token, 0);
if (StrLen(Token) == 3) //1st token is 3-letter day name
{
        ptr=GetToken(ptr, "\\S", &Token, 0); //month
        ptr=GetToken(ptr, "\\S", &Token, 0); //day
        ptr=GetToken(ptr, "\\S", &Token, 0); //time
        ptr=GetToken(ptr, "\\S", &Token, 0); //year
}
else //date/time format is YYYY-mm-dd HH:MM:SS
{
        ptr=GetToken(ptr, "\\S", &Token, 0); //date
        ptr=GetToken(ptr, "\\S", &Token, 0); //time
}

Destroy(Token);

return(ptr);
}


static char *OpenVpnProcess(char *Dev, TVpn *Vpn, STREAM *S)
{
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;

		Dev=CopyStr(Dev, "????");
    Tempstr=STREAMReadLine(Tempstr, S);
    while (Tempstr)
    {
        if (GlobalFlags & (FLAG_DEBUG | FLAG_VERBOSE))
        {
            if (strncmp(Tempstr, "vpn-mgr-", 8) != 0) printf("%s", Tempstr);
        }
        StripTrailingWhitespace(Tempstr);

        if (GlobalFlags & FLAG_EXIT) break;

        ptr=OpenVpnReadLineSkipDateTime(Tempstr);
        ptr=GetToken(ptr, "\\S", &Token, 0);
        if (strcmp(Token, "net_iface_up:")==0)
        {
            ptr=GetToken(ptr, "\\S", &Token, 0);  //'set'
            ptr=GetToken(ptr, "\\S", &Dev, 0);  //device name
            VpnUp(Vpn, Dev);
        }
        else if (strcmp(Token, "TUN/TAP")==0) 
				{
            ptr=GetToken(ptr, "\\S", &Token, 0);  //'set'
						if (strcmp(Token, "device")==0) ptr=GetToken(ptr, "\\S", &Dev, 0);  //'set'
				}
        else if (strcmp(Token, "ERROR:")==0) TerminalPrint(Terminal, "~rERROR:~0 %s\n", ptr);
        else if (strcmp(Token, "WARNING:")==0) TerminalPrint(Terminal, "~yWARNING:~0 %s\n", ptr);
        else if (strcmp(Token, "Initialization")==0)
				{
            ptr=GetToken(ptr, "\\S", &Token, 0);  //'Sequence'
            ptr=GetToken(ptr, "\\S", &Token, 0);  //should be 'Completed'
            if (strcmp(Token, "Completed")==0) VpnUp(Vpn, Dev);
				}
        else if (strcmp(Token, "Exiting")==0)
        {
            if (! (GlobalFlags & (FLAG_DEBUG | FLAG_VERBOSE))) TerminalPrint(Terminal, "~rERROR:~0 openvpn exited unexpectedly. run vpn_mgr with -verbose or -debug to get more info.\n");
            else TerminalPrint(Terminal, "~rERROR:~0 openvpn exited unexpectedly.\n", ptr);
        }

        Tempstr=STREAMReadLine(Tempstr, S);
    }

    Destroy(Tempstr);
    Destroy(Token);

return(Dev);
}



void OpenVpnUp(TVpn *Vpn)
{
    char *Tempstr=NULL, *Host=NULL, *Port=NULL, *MTU=NULL, *Cmd=NULL, *Dev=NULL;
    const char *ptr;
    STREAM *S;

    if (TunAvailable())
    {
        MTU=DefaultDevGetMTU(MTU);
        if (StrValid(Vpn->ConfFile))
        {
            Cmd=MCopyStr(Cmd, "openvpn --conf ", Vpn->ConfFile, NULL);
        }
        else
        {
            ParseURL(Vpn->Server, NULL, &Host, &Port, NULL, NULL, NULL, NULL);
            Cmd=MCopyStr(Cmd, "openvpn --client --tls-client --dev tun --persist-tun --nobind --keepalive 10 30 --remote ", Host, " ", Port, NULL);

            if (Vpn->Flags & VPN_TCP) Cmd=CatStr(Cmd, " --proto tcp-client ");
            else Cmd=CatStr(Cmd, " --proto udp ");

            if (StrValid(Vpn->PreSharedKey)) Cmd=MCatStr(Cmd, " --secret ", Vpn->PreSharedKey, NULL);
            if (StrValid(Vpn->Ciphers)) Cmd=MCatStr(Cmd, " --cipher '", Vpn->Ciphers, "' ", NULL);

            if (StrValid(Vpn->UserName))
            {
                OpenVpnGenPasswordFile(Vpn);
                Cmd=MCatStr(Cmd,  " --auth-user-pass ", GetCurrUserHomeDir(), "/.config/vpn_mgr/", Vpn->Name, "/", Vpn->Name, ".auth ", NULL);
            }

            if (StrValid(Vpn->ClientCert))
            {
                Cmd=MCatStr(Cmd, " --cert ", Vpn->ClientCert, NULL);
                Cmd=MCatStr(Cmd, " --key ", Vpn->ClientKey, NULL);
            }

            if (StrValid(Vpn->VerifyCert)) Cmd=MCatStr(Cmd, " --ca ", Vpn->VerifyCert, NULL);
        }

        if (Vpn->MTU > 0)
        {
            Tempstr=FormatStr(Tempstr, " --tun-mtu %d ", Vpn->MTU);
            Cmd=CatStr(Cmd, Tempstr);
        }
        //else if (StrValid(MTU)) Cmd=MCatStr(Cmd, " --link-mtu ", MTU, NULL);

        Cmd=MCatStr(Cmd, " --writepid '", GetCurrUserHomeDir(), "/.config/vpn_mgr/", Vpn->Name, "/", Vpn->Name, ".pid' ", NULL);

        TerminalPrint(Terminal, "~cOPENVPN:~0 Launch openvpn client\n");
        S=RunCommandOpen(Cmd, CMD_ASROOT | CMD_KILL);
        if (S)
        {
            Dev=OpenVpnProcess(Dev, Vpn, S);
            TerminalPrint(Terminal, "~cOPENVPN:~0 ShuttingDown\n");
            OpenVpnShutDown(Vpn, Dev);
        }
    }

    Destroy(Tempstr);
    Destroy(Host);
    Destroy(Port);
    Destroy(MTU);
    Destroy(Cmd);
    Destroy(Dev);
}

#include "wireguard.h"
#include "tuntap.h"
#include "run_command.h"
#include "route.h"
#include "net.h"
#include "updown.h"

static int WireguardLaunchUserspace(TVpn *Vpn, const char *Dev, const char *UserspaceApp)
{
    char *Tempstr=NULL, *Cmd=NULL;
    const char *ptr;
    int status=-1;

    ptr=GetToken(UserspaceApp, "\\S", &Cmd, 0);

    if (StrValid(GetCommandPath(Cmd)))
    {
        if (StrValid(ptr)) TerminalPrint(Terminal, "~cWIREGUARD:~0 '%s' userspace program found. Attempting again using that with arguments: %s\n", Cmd, ptr);
        else TerminalPrint(Terminal, "~cWIREGUARD:~0 '%s' userspace program found. Attempting again using that..\n", Cmd);

        Tempstr=MCopyStr(Tempstr, UserspaceApp, " ", Dev, NULL);
        RunCommand(Tempstr, CMD_ASROOT);
        status=TunSetup("wireguard", Dev, Vpn->LocalAddress);
        if (status == RUN_CMD_OKAY)
        {
            if (! NetDevExists(Dev)) status=RUN_CMD_FAIL;
        }
    }

    Destroy(Tempstr);
    Destroy(Cmd);

    return(status);
}


static char *WireguardFormatCommand(char *RetStr, TVpn *Vpn, const char *Dev)
{
    char *Host=NULL, *Port=NULL;
    char *VerifyCert=NULL;

    if (StrValid(Vpn->ConfFile))
    {
        RetStr=MCopyStr(RetStr, "wg setconf ", Dev, " ", Vpn->ConfFile, NULL);
    }
    else
    {
        VerifyCert=ReadFile(VerifyCert, Vpn->VerifyCert);

        //sometimes the first strip doesn't do the job, but I'm not sure why
        StripTrailingWhitespace(VerifyCert);
        StripTrailingWhitespace(VerifyCert);

        UnpackURL(Vpn->Server, NULL, &Host, &Port, NULL, NULL, NULL, NULL);
        if (! StrValid(Port)) Port=CopyStr(Port, "51820");
        if (Vpn->Action == ACT_CONNECT) RouteSetupVpnServer(Host);

        RetStr=MCopyStr(RetStr, "wg set ", Dev, " private-key ", Vpn->ClientCert, NULL);
        if (Vpn->Action==ACT_SERVER) RetStr=MCatStr(RetStr, " listen-port ", Port, NULL);

        RetStr=MCatStr(RetStr, " peer ", VerifyCert, NULL);

        if (Vpn->Action==ACT_CONNECT) RetStr=MCatStr(RetStr, " endpoint ", Host, ":", Port, NULL);


        //wireguard filters IPs itself, independant of netfilter
        if (StrValid(Vpn->AllowedIPs))
        {
            if (strcasecmp(Vpn->AllowedIPs, "all")==0) RetStr=CatStr(RetStr, " allowed-ips 0.0.0.0/0");
            else RetStr=MCatStr(RetStr, " allowed-ips ", Vpn->AllowedIPs, NULL);
        }

    }

    Destroy(VerifyCert);
    Destroy(Host);
    Destroy(Port);

    return(RetStr);
}


int WireguardUp(TVpn *Vpn)
{
    char *Tempstr=NULL, *Dev=NULL;
    int status;

    TerminalPrint(Terminal, "~cWIREGUARD:~0 attempt 'modprobe wireguard' to ensure kernel module loaded if needed\n");
    RunCommand("modprobe wireguard", CMD_ASROOT);

    if (StrValid(Vpn->Dev)) Dev=CopyStr(Dev, Vpn->Dev);
    else Dev=NetDevFindNext(Dev, "wg");


    TerminalPrint(Terminal, "~cWIREGUARD:~0 using tunnel dev [%s]\n", Dev);
    if (StrValid(Dev))
    {
        status=TunSetup("wireguard", Dev, Vpn->LocalAddress);
        if (status != RUN_CMD_OKAY)
        {
            TerminalPrint(Terminal, "~cWIREGUARD:~0 Failed to create/setup interface. Kernel may not support wireguard.\n");
            if (WireguardLaunchUserspace(Vpn, Dev, "wireguard-go") == RUN_CMD_OKAY ) /*do nothing, wireguard-go now active*/ ;
            else if (WireguardLaunchUserspace(Vpn, Dev, "boringtun-cli") == RUN_CMD_OKAY) /*do nothing, boringtun now active*/ ;
            else if (WireguardLaunchUserspace(Vpn, Dev, "boringtun-cli --disable-drop-privileges") == RUN_CMD_OKAY) /*do nothing, boringtun now active*/ ;
            else TerminalPrint(Terminal, "~cWIREGUARD:~0 If your kernel does not support wireguard, consider installing boring-tun: https://https://github.com/cloudflare/boringtun or wireguard-go: https://github.com/WireGuard/wireguard-go userspace implementations.\n");
        }


        Tempstr=WireguardFormatCommand(Tempstr, Vpn, Dev);
        if (RunCommand(Tempstr, CMD_ASROOT) == RUN_CMD_OKAY)
        {
            VpnUp(Vpn, Dev);

						//keep reading stuff from stdin, so that the user can use
            //ctrl-c to shut vpn down
            Tempstr=STREAMReadLine(Tempstr, Terminal);
            while (Tempstr)
            {
                if (GlobalFlags & FLAG_EXIT) break;
                Tempstr=STREAMReadLine(Tempstr, Terminal);
            }

            TerminalPrint(Terminal, "~cWIREGUARD:~0 shutting down\n");
            TunShutdown(Dev);
        }
        else TerminalPrint(Terminal, "~rFATAL:~0 wireguard setup failed\n");
    }
    else TerminalPrint(Terminal, "~rFATAL:~0 no suitable tunnel device\n");

    Destroy(Tempstr);
    Destroy(Dev);
}


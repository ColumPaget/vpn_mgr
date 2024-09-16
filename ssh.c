#include "ssh.h"
#include "net.h"
#include "route.h"
#include "tuntap.h"
#include "run_command.h"
#include "updown.h"

STREAM *Active=NULL;




char *SSHHostLookupIP(char *IP, const char *Host)
{
    char *Tempstr=NULL, *CurrHost=NULL;
    const char *ptr;
    STREAM *S;

    IP=CopyStr(IP, "");
    CurrHost=CopyStr(CurrHost, "");

    Tempstr=MCopyStr(Tempstr, GetCurrUserHomeDir(), "/.ssh/config", NULL);
    S=STREAMOpen(Tempstr, "r");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripTrailingWhitespace(Tempstr);
            StripLeadingWhitespace(Tempstr);

            if (strncmp(Tempstr, "Host ", 5)==0)
            {
                ptr=Tempstr+5;
                while (isspace(*ptr)) ptr++;
                CurrHost=CopyStr(CurrHost, Tempstr+5);
            }
            else if (strncmp(Tempstr, "HostName ", 9)==0)
            {
                if (strcmp(Host, CurrHost)==0)
                {
                    ptr=Tempstr+9;
                    while (isspace(*ptr)) ptr++;
                    IP=CopyStr(IP, LookupHostIP(ptr));
                }
            }

            Tempstr=STREAMReadLine(Tempstr, S);
        }
        STREAMClose(S);
    }

    Destroy(Tempstr);
    Destroy(CurrHost);

    return(IP);
}


static char *SSHVpnSendSync(char *Key, STREAM *S)
{
    char *Tempstr=NULL;

    Key=GetRandomAlphabetStr(Key, 16);
    Tempstr=MCopyStr(Tempstr, "export PS1=\"\"; export PS2=\"\"; echo ", Key, "\n", NULL);
    STREAMWriteLine(Tempstr, S);
    STREAMFlush(S);
    if (GlobalFlags & FLAG_DEBUG) fprintf(stderr, "SSH SYNC: SEND: %s", Tempstr);

    Destroy(Tempstr);

    return(Key);
}


static void SSHVpnSync(STREAM *S)
{
    char *Tempstr=NULL, *Key=NULL;

    Key=SSHVpnSendSync(Key, S);

    STREAMSetTimeout(S, 100);
    Tempstr=STREAMReadToMultiTerminator(Tempstr, S, "\r\n");
    while (Tempstr)
    {
        if (! StrValid(Tempstr)) Key=SSHVpnSendSync(Key, S);
        else
        {
            StripTrailingWhitespace(Tempstr);
            StripLeadingWhitespace(Tempstr);
            if (GlobalFlags & FLAG_DEBUG) fprintf(stderr, "SSH SYNC: [%s] [%s]\n", Tempstr, Key);
            if (strcmp(Tempstr, Key)==0) break;
            if (GlobalFlags & FLAG_EXIT) break;
        }

        Tempstr=STREAMReadToMultiTerminator(Tempstr, S, "\r\n");
    }

    Tempstr=SetStrLen(Tempstr, 4096);
    STREAMReadBytes(S, Tempstr, 4096);
    if (GlobalFlags & FLAG_DEBUG) fprintf(stderr, "SYNC DONE [%s] [%s]\n", Tempstr, Key);

    Destroy(Tempstr);
    Destroy(Key);
}



int SSHVpnRunCommandTry(STREAM *S, const char *Cmd, int Flags)
{
    char *Tempstr=NULL, *Key=NULL;
    int result=FALSE;

    SSHVpnSync(S);
    if (StrValid(Cmd))
    {
        Tempstr=RunCmdFormat(Tempstr, Cmd, "", Flags | CMD_SSH);
        STREAMWriteLine(Tempstr, S);
        STREAMWriteLine("\n", S);
        STREAMFlush(S);
        TerminalPrint(Terminal, "~rSSH REMOTE RUN COMMAND:~0 [%s]\n", Tempstr);
        if (Flags & CMD_SUDO) result=RunCommandReadLoop(S, "Remote", "su/sudo");
        else if (Flags & CMD_SU) result=RunCommandReadLoop(S, "Remote", "su/sudo");
        else result=TRUE;
    }

    if (! (Flags & CMD_NOSYNC)) SSHVpnSync(S);

    Destroy(Tempstr);
    Destroy(Key);

    if (result==RUN_CMD_OKAY) return(TRUE);
    return(FALSE);
}



int SSHVpnRunCommand(STREAM *S, const char *Cmd, int Flags)
{
    int CmdFlags;

    CmdFlags=Flags & ~(CMD_SU | CMD_SUDO);
    if ((Flags & CMD_SU) && (GlobalFlags & FLAG_REMOTE_SU)) CmdFlags |= CMD_SU;
    if ((Flags & CMD_SUDO) && (GlobalFlags & FLAG_REMOTE_SUDO)) CmdFlags |= CMD_SUDO;

    if (SSHVpnRunCommandTry(S, Cmd, CmdFlags)) return(TRUE);

    TerminalPrint(Terminal, "~yremote sudo failed, attempting su~0\n");
//we always try SUDO first, so knock that out if it failed
    CmdFlags &= ~CMD_SUDO;

    return(SSHVpnRunCommandTry(S, Cmd, CmdFlags));
}



STREAM *SSHVpnConnect(TVpn *Vpn, const char *Command)
{
    STREAM *S;
    char *Tempstr=NULL;
    char *Proto=NULL, *Host=NULL, *Port=NULL, *User=NULL;

    UnpackURL(Vpn->Server, &Proto, &Host, &Port, &User, NULL, NULL, NULL);


    Tempstr=SSHHostLookupIP(Tempstr, Host);
    if (StrValid(Tempstr)) RouteSetupVpnServer(Tempstr);

    if (StrValid(User)) Tempstr=MCopyStr(Tempstr, "ssh -t ", User, "@", Host, NULL);
    else Tempstr=MCopyStr(Tempstr, "ssh -t ", Host, NULL);

    if (StrValid(Port)) Tempstr=MCatStr(Tempstr, " -p ", Port, NULL);
    if (StrValid(Vpn->Ciphers)) Tempstr=MCatStr(Tempstr, " -c '", Vpn->Ciphers, "' ", NULL);

    if (StrValid(Command)) Tempstr=MCatStr(Tempstr, " ", Command, NULL);

    S=RunCommandOpen(Tempstr, 0);
    if (S)
    {
        TerminalPrint(Terminal, "~cSSH VPN:~0 Setup remote end of connection\n");
        STREAMWriteLine("export LC_ALL=C; export LANG=C; export PS1=\"\"; export PS2=\"\"\n", S);

        //Used '\r\n' here to allow for all kinds of crap we can be sent at the start, before we've set PS1=""
        SSHVpnSync(S);
    }


    Destroy(Tempstr);
    Destroy(Proto);
    Destroy(Host);
    Destroy(Port);
    Destroy(User);

    return(S);
}


//Get the value of the 'SSH_TUNNEL' environment variable at the remote end
static char *SSHGetRemoteDev(char *Dev, STREAM *S)
{
    int i;

    Dev=CopyStr(Dev, "");
    STREAMWriteLine("echo $SSH_TUNNEL\n", S);
    STREAMFlush(S);

    //handle any crap produced by 'echo' etc
    for (i=0; i < 10; i++)
    {
        Dev=STREAMReadToMultiTerminator(Dev, S, "\r\n");
        StripTrailingWhitespace(Dev);
        if (GlobalFlags & FLAG_DEBUG) printf("SSH: SSHGetRemoveDev: readline [%s]\n", Dev);
        if (StrValid(Dev) && (strncmp(Dev, "tun", 3)==0)) break;
    }

    return(Dev);
}


static int SSHSetup(TVpn *Vpn, STREAM *S)
{
    char *Tempstr=NULL, *Dev=NULL;
    int RetVal=FALSE;

    STREAMWriteLine("stty -echo\n", S);
    SSHVpnSync(S);
    Dev=SSHGetRemoteDev(Dev, S);

    if (! StrValid(Dev))
    {
        TerminalPrint(Terminal, "~cSSH VPN: ~r~eFATAL: no 'tun' device associated with ssh connection~0\n");
    }
    else
    {
        TerminalPrint(Terminal, "~cSSH VPN:~0 ~gconnected~0 using remote tunnel dev: [%s]\n", Dev);

        //TUN-based SSH vpns are a bit of a special case, they don't need enpoint IP addresses at both ends of the vpn link
        //it's enough to route packets into the link for an IP that can be accessed at the other end.
        //Thus we use the 'DefaultIP', which is the IP associated with the default route, usually on eth0 or wlan0
        if (! StrValid(Vpn->LocalAddress)) Vpn->LocalAddress=NetGetDefaultIP(Vpn->LocalAddress);

        if (StrValid(Vpn->LocalAddress))
        {
            TerminalPrint(Terminal, "~cSSH VPN:~0 Adding route back to our local IP address '%s'\n", Vpn->LocalAddress);
            //cannot use RouteSetup here, as this is running a command remotely, not locally as RouteSetup does
            Tempstr=MCopyStr(Tempstr, "/sbin/route add -host ", Vpn->LocalAddress, " dev ", Dev, NULL);
            SSHVpnRunCommand(S, Tempstr, CMD_SU | CMD_SUDO);
        }
        RetVal=TRUE;
    }

    TerminalPrint(Terminal, "~cSSH VPN:~0 ~gSetup complete~0\n");
    fflush(NULL);

    Destroy(Tempstr);
    Destroy(Dev);

    return(RetVal);
}


static void SSHProcess(TVpn *Vpn, STREAM *S)
{
    char *Tempstr=NULL;

    if (GlobalFlags & FLAG_EXIT) return;

    Tempstr=STREAMReadLine(Tempstr, S);
    while (Tempstr)
    {
        StripTrailingWhitespace(Tempstr);
        if (StrValid(Tempstr)) TerminalPrint(Terminal, "%s\n", Tempstr);
        if (GlobalFlags & FLAG_EXIT) break;
        Tempstr=STREAMReadLine(Tempstr, S);
    }

    Destroy(Tempstr);
}


int SSHVpnValidate(TVpn *Vpn)
{
    char *Host=NULL, *Resolved=NULL, *Tempstr=NULL;
    int result=FALSE;
    STREAM *S;

    ParseURL(Vpn->Server, NULL, &Host, NULL, NULL, NULL, NULL, NULL);

    Tempstr=MCopyStr(Tempstr, "cmd:ssh -T -G ", Host, NULL);
    S=STREAMOpen(Tempstr, "");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripTrailingWhitespace(Tempstr);

            if (strncmp(Tempstr, "hostname ", 9) == 0)
            {
                Resolved=CopyStr(Resolved, Tempstr+9);
                //hostname we lookup should not match the ssh name
                if (strcmp(Host, Resolved) != 0) result=TRUE;
            }
            Tempstr=STREAMReadLine(Tempstr, S);
        }

        STREAMClose(S);
    }


    if (! result) TerminalPrint(Terminal, "~cSSH:~0 ~rERROR:~0 '%s' does not seem to be configured in the ~/.ssh/config file. Please setup this connection\n", Host);

    Destroy(Resolved);
    Destroy(Tempstr);
    Destroy(Host);

    return(result);
}


//this function is trigged by signals like SIGTERM sent by ctrl-c
void SSHVpnTerminate()
{
    pid_t pid;

    if (Active)
    {
        pid=atoi(STREAMGetValue(Active, "PeerPID"));
        if (pid > 1) kill(pid, SIGKILL);

        //Do not close this, it will be closed elsewhere
        //STREAMClose(Active);
        Active=NULL;
    }
}



int SSHVpnUp(TVpn *Vpn)
{
    char *Tempstr=NULL, *Dev=NULL, *RemoteDev=NULL;
    STREAM *S;

    if (! SSHVpnValidate(Vpn))
    {
        return(FALSE);
    }

    if (StrValid(Vpn->Dev)) Dev=CopyStr(Dev, Vpn->Dev);
    else Dev=NetDevFindNext(Dev, "tun");

    if (StrValid(Vpn->RemoteDev))
    {
        if (strncmp(Vpn->RemoteDev, "tun", 3)==0) RemoteDev=CopyStr(RemoteDev, Vpn->RemoteDev + 3);
        else RemoteDev=CopyStr(RemoteDev, Vpn->RemoteDev);
    }
    else RemoteDev=CopyStr(RemoteDev, "any");

    if (StrValid(Dev))
    {
        //setup the LOCAL end of the Vpn
        TunSetup("tun", Dev, Vpn->LocalAddress);
        //NetSetup(Vpn, Dev);
        Tempstr=MCopyStr(Tempstr, " -w ", Dev+3, ":", RemoteDev, NULL);

        S=SSHVpnConnect(Vpn, Tempstr);
        if (S)
        {
            //setup the REMOTE end of the vpn
            if (SSHSetup(Vpn, S))
            {
								VpnUp(Vpn, Dev);
                Active=S;
                TerminalPrint(Terminal, "~gVPN UP:~0 dev=%s\n", Dev);
                SSHProcess(Vpn, S);
            }
            RunCommandCleanUp(S, CMD_KILL);
            VpnDown(Vpn, Dev);
        }
    }

    Destroy(RemoteDev);
    Destroy(Tempstr);
    Destroy(Dev);
}


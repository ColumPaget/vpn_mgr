#include "common.h"
#include "ssh.h"

STREAM *Terminal=NULL;
int GlobalFlags=0;

void SignalHandler(int sig)
{
    switch(sig)
    {
    case SIGKILL:
    case SIGTERM:
    case SIGINT:
        GlobalFlags |= FLAG_EXIT;
        SSHVpnTerminate();
        //	kill(0-getpid(), SIGINT);
        break;
    }

}


char *ReadFile(char *RetStr, const char *Path)
{
    STREAM *S;

    RetStr=CopyStr(RetStr, "");
    if (! StrValid(Path)) return(RetStr);

    S=STREAMOpen(Path, "r");
    if (S)
    {
        RetStr=STREAMReadDocument(RetStr, S);
        STREAMClose(S);
    }

    return(RetStr);
}


void LogEvent(int LogFlags, const char *Title, const char *Fmt, ...)
{
    char *Tempstr=NULL;
    const char *Prefix="", *Postfix="";
    va_list args;
    int SyslogFlags=0;

    va_start(args, Fmt);
    Tempstr=VFormatStr(Tempstr, Fmt, args);


    if (LogFlags & VPN_LOG_ERROR)
    {
        Prefix="~r";
        Postfix="~0";
    }
    else if (LogFlags & VPN_LOG_OKAY)
    {
        Prefix="~g";
        Postfix="~0";
    }
    else if (LogFlags & VPN_LOG_INFO)
    {
        Prefix="~c";
        Postfix="~0";
    }


    if (Terminal) TerminalPrint(Terminal, "%s%s:%s %s\n", Prefix, Title, Postfix, Tempstr);

    if (LogFlags & VPN_LOG_SYSLOG)
    {
        SyslogFlags |= LOG_PID;
        if (LogFlags & VPN_LOG_ERROR) SyslogFlags |= LOG_ERR;
        else if (LogFlags & VPN_LOG_INFO) SyslogFlags |= LOG_INFO;
        else SyslogFlags |= LOG_NOTICE;
        syslog(SyslogFlags, "%s: %s", Title, Tempstr);
    }
    va_end(args);

    Destroy(Tempstr);
}

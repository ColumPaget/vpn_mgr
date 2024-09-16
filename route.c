#include "route.h"
#include "run_command.h"

char *RouteParseHexAddress(char *RetStr, const char *Addr)
{
    int len, val;
    char *Tempstr=NULL;
    const char *ptr;

    RetStr=CopyStr(RetStr, "");
    len=StrLen(Addr);
    for (ptr=Addr + len -2;  ptr >= Addr; ptr -=2)
    {
        Tempstr=CopyStrLen(Tempstr, ptr, 2);
        val=strtol(Tempstr, NULL, 16);
        Tempstr=FormatStr(Tempstr, "%d", val);
        if (StrValid(RetStr)) RetStr=CatStr(RetStr, ".");
        RetStr=CatStr(RetStr, Tempstr);
    }

    Destroy(Tempstr);

    return(RetStr);
}


char *RouteGetDefault(char *RetStr)
{
    STREAM *S;
    char *Tempstr=NULL, *Dev=NULL, *Token=NULL, *Address=NULL;
    const char *ptr;

    RetStr=CopyStr(RetStr, "");
    S=STREAMOpen("/proc/net/route", "r");
    if (S)
    {
//first line is a header
        Tempstr=STREAMReadLine(Tempstr, S);
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            ptr=GetToken(Tempstr, "\\S", &Dev, 0);
            ptr=GetToken(ptr, "\\S", &Token, 0);
            Address=RouteParseHexAddress(Address, Token);
            if (strcmp(Address, "0.0.0.0")==0)
            {
                ptr=GetToken(ptr, "\\S", &Token, 0);
                RetStr=RouteParseHexAddress(RetStr, Token);
            }

            Tempstr=STREAMReadLine(Tempstr, S);
        }
        STREAMClose(S);
    }

    Destroy(Tempstr);
    Destroy(Token);
    Destroy(Dev);

    return(RetStr);
}

char *RouteGetDefaultDev(char *RetStr)
{
    STREAM *S;
    char *Tempstr=NULL, *Dev=NULL, *Token=NULL, *Address=NULL;
    const char *ptr;

    RetStr=CopyStr(RetStr, "");
    S=STREAMOpen("/proc/net/route", "r");
    if (S)
    {
//first line is a header
        Tempstr=STREAMReadLine(Tempstr, S);
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            ptr=GetToken(Tempstr, "\\S", &Dev, 0);
            ptr=GetToken(ptr, "\\S", &Token, 0);
            if (strcmp(Token, "00000000")==0) RetStr=CopyStr(RetStr, Dev);

            Tempstr=STREAMReadLine(Tempstr, S);
        }
        STREAMClose(S);
    }

    Destroy(Tempstr);
    Destroy(Address);
    Destroy(Token);
    Destroy(Dev);

    return(RetStr);
}




void RouteSetupVpnServer(const char *DestAddress)
{
    char *Router=NULL, *Tempstr=NULL;

    Router=RouteGetDefault(Router);
    TerminalPrint(Terminal, "~mROUTE:~0 Add route for vpn server [%s] via gateway [%s]\n", DestAddress, Router);

    Tempstr=MCopyStr(Tempstr, "route add -host ", DestAddress, " gw ", Router, NULL);
    RunCommand(Tempstr, CMD_ASROOT);

    Destroy(Tempstr);
    Destroy(Router);
}


void RouteSetup(const char *DestAddressList, const char *Dev)
{
    char *Router=NULL, *Address=NULL, *Tempstr=NULL;
    const char *ptr;

    ptr=GetToken(DestAddressList, ",", &Address, GETTOKEN_QUOTES);
    while (ptr)
    {
        StripLeadingWhitespace(Address);
        StripTrailingWhitespace(Address);
        TerminalPrint(Terminal, "~mROUTE:~0 Add route for [%s] via the VPN\n", Address);

        if ( (strcmp(Address, "default")==0) || (strcmp(Address, "all")==0) ) Tempstr=MCopyStr(Tempstr, "route add default ", Dev, NULL);
        else if (strchr(Address, '/')) Tempstr=MCopyStr(Tempstr, "route add -net ", Address, " ", Dev, NULL);
        else Tempstr=MCopyStr(Tempstr, "route add -host ", Address, " ", Dev, NULL);

        RunCommand(Tempstr, CMD_ASROOT);
        ptr=GetToken(ptr, ",", &Address, GETTOKEN_QUOTES);
    }

    Destroy(Tempstr);
    Destroy(Address);
    Destroy(Router);
}


char *DefaultDevGetMTU(char *RetStr)
{
    char *DefaultDev=NULL, *Tempstr=NULL;

    DefaultDev=RouteGetDefaultDev(DefaultDev);
    Tempstr=MCopyStr(Tempstr, "/sys/class/net/", DefaultDev, "/mtu", NULL);
    RetStr=ReadFile(RetStr, Tempstr);

    Destroy(DefaultDev);
    Destroy(Tempstr);

    return(RetStr);
}

#include "net.h"
#include "run_command.h"
#include "route.h"
#include "resolv.h"

ListNode *NetLoadDevices()
{
    STREAM *S;
    char *Tempstr=NULL, *Name=NULL;
    ListNode *Devs;

    Devs=ListCreate();

    S=STREAMOpen("/proc/net/dev", "r");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripLeadingWhitespace(Tempstr);
            StripTrailingWhitespace(Tempstr);
            GetToken(Tempstr, ":", &Name, 0);
            ListAddNamedItem(Devs, Name, CopyStr(NULL, GetInterfaceIP(Name)));
            Tempstr=STREAMReadLine(Tempstr, S);
        }
        STREAMClose(S);
    }

    Destroy(Tempstr);
    Destroy(Name);

    return(Devs);
}


int NetDevExists(const char *Name)
{
    ListNode *Devs;
    int result=FALSE;

    Devs=NetLoadDevices();
    if (ListFindNamedItem(Devs, Name)) result=TRUE;
    ListDestroy(Devs, Destroy);

    return(result);
}



char *NetDevFindNext(char *RetStr, const char *Prefix)
{
    int i;

    for (i=0; i < 999; i++)
    {
        RetStr=FormatStr(RetStr, "%s%d", Prefix, i);
        if (! NetDevExists(RetStr)) return(RetStr);
    }

    return(CopyStr(RetStr, ""));
}

char *NetGetDefaultIP(char *RetStr)
{
    char *Dev=NULL;

    Dev=RouteGetDefaultDev(Dev);
    RetStr=CopyStr(RetStr, GetInterfaceIP(Dev));

    Destroy(Dev);
    return(RetStr);
}


char *NetDevFindForIP(char *RetStr, const char *Prefix, const char *IP)
{
    ListNode *Devs, *Curr;

    RetStr=CopyStr(RetStr, "");
    Devs=NetLoadDevices();
    Curr=ListGetNext(Devs);
    while (Curr)
    {
        if (strncmp(Curr->Tag, Prefix, StrLen(Prefix))==0)
        {
            if (strcmp((const char *) Curr->Item, IP)==0)
            {
                RetStr=CopyStr(RetStr, Curr->Tag);
                break;
            }
        }
        Curr=ListGetNext(Curr);
    }

    ListDestroy(Devs, Destroy);
    return(RetStr);
}


static int DevsIPExists(ListNode *Devs, const char *IP)
{
    ListNode *Curr;

    Curr=ListGetNext(Devs);

    while (Curr)
    {
        if (strcmp((const char *) Curr->Item, IP)==0) return(TRUE);
        Curr=ListGetNext(Curr);
    }

    return(FALSE);
}


char *NetDevFindNextIP(char *RetStr, const char *Prefix)
{
    ListNode *Devs;
    char *Tempstr=NULL;
    int count;

    RetStr=CopyStr(RetStr, "");
    Devs=NetLoadDevices();

    for (count=2; count < 255; count++)
    {
        Tempstr=FormatStr(Tempstr, "%s.%d", Prefix, count);
        if (! DevsIPExists(Devs, Tempstr)) break;
    }

    if (count < 255) RetStr=CopyStr(RetStr, Tempstr);

    ListDestroy(Devs, Destroy);
    Destroy(Tempstr);

    return(RetStr);
}




void NetSetup(TVpn *Vpn, const char *Dev)
{
    char *Tempstr=NULL;

    if (StrValid(Vpn->LocalAddress))
    {
        TerminalPrint(Terminal, "~mNET:~0 set IP address of local end of connection to [%s]\n", Vpn->LocalAddress);
        Tempstr=MCopyStr(Tempstr, "ifconfig ", Dev, " ", Vpn->LocalAddress, NULL);
        if (StrValid(Vpn->RemoteAddress)) Tempstr=MCatStr(Tempstr, " pointopoint ", Vpn->RemoteAddress, NULL);
        RunCommand(Tempstr, CMD_ASROOT);
    }

    //setup routes for remote IPs we are allowed to connect to via this link
    if ((Vpn->Action==ACT_CONNECT) && StrValid(Vpn->AllowedIPs)) RouteSetup(Vpn->AllowedIPs, Dev);
    if ((! (GlobalFlags & FLAG_NODNS)) && StrValid(Vpn->DNS)) ResolvChange(Vpn->DNS);

    Destroy(Tempstr);
}

void NetShutdown(TVpn *Vpn, const char *Dev)
{
    char *Tempstr=NULL;

    TerminalPrint(Terminal, "~mNET:~0 Shutdown local device %s\n", Dev);
    Tempstr=MCopyStr(Tempstr, "ifconfig ", Dev, " down", NULL);
    RunCommand(Tempstr, CMD_ASROOT);
    TerminalPrint(Terminal, "~mNET:~0 Remove local device %s\n", Dev);
    Tempstr=MCopyStr(Tempstr, "ip link delete dev ", Dev,  NULL);
    RunCommand(Tempstr, CMD_ASROOT);

    Destroy(Tempstr);
}

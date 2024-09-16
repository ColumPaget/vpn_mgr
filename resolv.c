#include "resolv.h"
#include "run_command.h"

int ResolvChange(const char *Nameservers)
{
    char *Path=NULL, *Token=NULL, *Tempstr=NULL;
    const char *ptr;
    STREAM *S;

    Path=FormatStr(Path, "/tmp/vpn_mgr-%d.resolv.conf", getpid());
    S=STREAMOpen(Path, "w");
    if (S)
    {
        ptr=GetToken(Nameservers, ",", &Token, 0);
        while (ptr)
        {
            Tempstr=MCopyStr(Tempstr, "nameserver ", Token, "\n", NULL);
            STREAMWriteLine(Tempstr, S);
            ptr=GetToken(ptr, ",", &Token, 0);
        }
        STREAMClose(S);
    }

    RunCommand("mv /etc/resolv.conf /etc/vpn_mgr.resolv.save", CMD_ASROOT);
    Tempstr=MCopyStr(Tempstr, "mv -f ", Path, " /etc/resolv.conf", NULL);
    RunCommand(Tempstr, CMD_ASROOT);

    Destroy(Tempstr);
    Destroy(Token);
    Destroy(Path);
}

int ResolvRestore()
{
    char *Tempstr=NULL;

    RunCommand("mv /etc/vpn_mgr.resolv.save /etc/resolv.conf", CMD_ASROOT);

    Destroy(Tempstr);
}

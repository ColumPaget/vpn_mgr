#include "updown.h"
#include "run_command.h"
#include "net.h"

static void UpDownRunFiles(TVpn *Vpn, const char *Files, const char *Dev)
{
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;

        ptr=GetToken(Files, ":", &Token, 0);
        while (ptr)
        {
            if (access(Token, X_OK)==0)
            {
                Tempstr=MCopyStr(Tempstr, Token, " '", Dev, "' '", Vpn->Server, "' '", Vpn->LocalAddress, "' '", Vpn->RemoteAddress, "' '", Vpn->Name, "'", NULL);
                RunCommand(Tempstr, CMD_ASROOT);
            }
            ptr=GetToken(ptr, ":", &Token, 0);
        }

    Destroy(Tempstr);
    Destroy(Token);
}


void VpnUp(TVpn *Vpn, const char *Dev)
{
    NetSetup(Vpn, Dev);

    if (StrValid(Vpn->UpFile)) UpDownRunFiles(Vpn, Vpn->UpFile, Dev);
    TerminalPrint(Terminal, "~gVPN UP:~0 dev=%s\n", Dev);
}


void VpnDown(TVpn *Vpn, const char *Dev)
{
    NetShutdown(Vpn, Dev);
    if (StrValid(Vpn->DownFile)) UpDownRunFiles(Vpn, Vpn->DownFile, Dev);
    TerminalPrint(Terminal, "~rVPN DOWN:~0 dev=%s\n", Dev);
}

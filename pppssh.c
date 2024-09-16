#include "pppssh.h"
#include "net.h"
#include "run_command.h"
#include "ppp.h"
#include "ssh.h"


int PPPSSHVpnUp(TVpn *Vpn)
{
    char *Tempstr=NULL, *ID=NULL;
    int Flags=0;
    STREAM *S;

    if (! SSHVpnValidate(Vpn))
    {
        printf("'%s' does not seem to be configured in the ~/.ssh/config file. Please setup this connection\n", Vpn->Server);
        return(FALSE);
    }


    S=SSHVpnConnect(Vpn, "");
    if (S)
    {
        //assume remote is the server
        if (! StrValid(Vpn->RemoteAddress)) Vpn->RemoteAddress=CopyStr(Vpn->RemoteAddress, "172.16.0.1");

        if (StrValid(Vpn->ClientID)) ID=CopyStr(ID, Vpn->ClientID);
        else
        {
            ID=CopyStr(ID, LookupUserName(getuid()));
            ID=MCatStr(ID, "@", OSSysInfoString(OSINFO_HOSTNAME), NULL);
        }


        Flags |= CMD_NOSYNC | CMD_SSH;
        if (GlobalFlags & FLAG_REMOTE_SU) Flags |= CMD_SU;
        if (GlobalFlags & FLAG_REMOTE_SUDO) Flags |= CMD_SUDO;

        Tempstr=MCopyStr(Tempstr, "/sbin/pppd ipparam '", ID, "' ", Vpn->RemoteAddress, ": ", PPPD_OPTIONS, NULL);

        if (Flags & (CMD_SU | CMD_SUDO)) Tempstr=CatStr(Tempstr, " noauth");

        SSHVpnRunCommand(S, Tempstr,  Flags);
        PPPDProcess(Vpn, S);
    }

    Destroy(Tempstr);
    Destroy(ID);
}


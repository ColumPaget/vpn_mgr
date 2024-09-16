#include "common.h"
#include "vpn_ctx.h"
#include "vpn_config.h"
#include "command_line.h"
#include "run_command.h"
#include "wireguard.h"
#include "openvpn.h"
#include "ssh.h"
#include "pppssh.h"
#include "ssl_server.h"
#include "ssl_client.h"
#include "route.h"

void DisplayProtocolSupport(const char *Name, const char *ReqProgs)
{
    char *Item=NULL, *Missing=NULL;
    const char *ptr;
    int result=TRUE;

    ptr=GetToken(ReqProgs, ",", &Item, 0);
    while (ptr)
    {
        if (! StrValid(GetCommandPath(Item)))
        {
            result=FALSE;
            Missing=MCatStr(Missing, Item, " ", NULL);
        }
        ptr=GetToken(ptr, ",", &Item, 0);
    }

    if (result) printf("  %s supported\n", Name);
    else printf("  %s NOT supported. %s missing\n", Name, Missing);

    Destroy(Missing);
    Destroy(Item);
}


void DisplaySupportedProtocols()
{
    printf("Available protocols:\n");
    DisplayProtocolSupport("wireguard", "ip,wg,route");
    DisplayProtocolSupport("wireguard via boringtun", "ip,wg,route,boringtun-cli");
    DisplayProtocolSupport("wireguard via wireguard-go", "ip,wg,route,wireguard-go");
    DisplayProtocolSupport("openvpn", "openvpn");
    DisplayProtocolSupport("ssh-native", "ip,ssh,route");
    DisplayProtocolSupport("ssh-ppp", "pppd,ssh,route");
    DisplayProtocolSupport("tls/ssl-ppp", "pppd,route");
}


int main(int argc, char *argv[])
{
    TVpn *Vpn;
    char *Tempstr=NULL;

    setpgid(0, getpid());

    //Assume we will need sudo/su at the remote end
    GlobalFlags=FLAG_REMOTE_SUDO | FLAG_REMOTE_SU;

    //if we are not root or suid root, then assume we will need local su/sudo
    if (geteuid() != 0) GlobalFlags |= FLAG_SUDO | FLAG_SU;

    Terminal=STREAMFromDualFD(0,1);
    TerminalInit(Terminal, TERM_RAWKEYS | TERM_SAVEATTRIBS);

//    signal(SIGKILL, SignalHandler);
//    signal(SIGTERM, SignalHandler);
    signal(SIGINT, SignalHandler);

    Vpn=ParseCommandLine(argc, argv);
    FindCommands("su,sudo,modprobe,ip,wg,ifconfig,route,ssh,wg,openvpn,pppd,chmod,boringtun-cli,wireguard-go,mv,kill");

    if (Vpn)
    {
        switch (Vpn->Action)
        {
        case ACT_CONFIG:
            WriteConfig(Vpn);
            break;

        case ACT_PROTOCOLS:
            DisplaySupportedProtocols();
            break;

        case ACT_LIST:
            ListConfigs(Vpn);
            break;

        case ACT_ADD:
            AddConfig(Vpn);
            break;

        case ACT_DELETE:
            DeleteConfig(Vpn);
            break;

        case ACT_CONNECT:
            if (! StrValid(Vpn->Server)) ReadConfig(Vpn);

            if (strncasecmp(Vpn->Server, "wg:", 3)==0) WireguardUp(Vpn);
            else if (strncasecmp(Vpn->Server, "openvpn:", 8)==0) OpenVpnUp(Vpn);
            else if (strncasecmp(Vpn->Server, "ovpn:", 5)==0) OpenVpnUp(Vpn);
            else if (strncasecmp(Vpn->Server, "ssh:", 4)==0) SSHVpnUp(Vpn);
            else if (strncasecmp(Vpn->Server, "pssh:", 5)==0) PPPSSHVpnUp(Vpn);
            else if (strncasecmp(Vpn->Server, "pppssh:", 7)==0) PPPSSHVpnUp(Vpn);
            else if (strncasecmp(Vpn->Server, "pppssl:", 7)==0) PPPSSLClientVpnUp(Vpn);
            else if (strncasecmp(Vpn->Server, "ppptls:", 7)==0) PPPSSLClientVpnUp(Vpn);
            break;

        case ACT_SERVER:
            if (strncasecmp(Vpn->Server, "wg:", 3)==0) WireguardUp(Vpn);
            else SSLServer(Vpn);
            break;
        }

    }

    TerminalReset(Terminal);
    Destroy(Tempstr);

    //make sure any programs we launched that are still running are shut down
    //but we don't want to send SIGTERM to ourselves again and trigger some kind of loop
    signal(SIGTERM, SIG_IGN);
    kill(0-getpid(), SIGTERM);

    return(0);
}

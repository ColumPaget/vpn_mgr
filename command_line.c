#include "command_line.h"
#include "vpn_ctx.h"
#include "help.h"

static int IsServerURL(const char *Server)
{
    if (strncmp(Server, "wg:", 3)==0) return(TRUE);
    if (strncmp(Server, "ovpn:", 5)==0) return(TRUE);
    if (strncmp(Server, "openvpn:", 8)==0) return(TRUE);
    if (strncmp(Server, "ssh:", 4)==0) return(TRUE);
    if (strncmp(Server, "tls:", 4)==0) return(TRUE);
    if (strncmp(Server, "ssl:", 4)==0) return(TRUE);
    if (strncmp(Server, "pssh:", 5)==0) return(TRUE);
    if (strncmp(Server, "pppssh:", 7)==0) return(TRUE);
    if (strncmp(Server, "pssl:", 5)==0) return(TRUE);
    if (strncmp(Server, "pppssl:", 7)==0) return(TRUE);
    if (strncmp(Server, "ptls:", 5)==0) return(TRUE);
    if (strncmp(Server, "ppptls:", 7)==0) return(TRUE);



    return(FALSE);
}


int CommandLineValid(const char *Action, const char *Name, const char *Server)
{
    const char *CommandsNeedingServer[]= {"add", "connect", "server", NULL};

    if (MatchTokenFromList(Action, CommandsNeedingServer, 0) > -1)
    {
        if (StrValid(Server) && (! IsServerURL(Server)) )
        {
            if (IsServerURL(Name)) printf("ERROR: Invalid command line: 'name' seems to be the service url.\n");
            else printf("Invalid command line: no server url\n");
            return(FALSE);
        }
    }

    return(TRUE);
}


TVpn *ParseCommandLine(int argc, char *argv[])
{
    CMDLINE *Cmd;
    const char *Arg;
    char *Action=NULL, *Name=NULL, *Server=NULL;
    TVpn *Ctx=NULL;

    Cmd=CommandLineParserCreate(argc, argv);

    Action=CopyStr(Action, CommandLineNext(Cmd));
    if (strcasecmp(Action, "add")==0)
    {
        Name=CopyStr(Name, CommandLineNext(Cmd));
        Server=CopyStr(Server, CommandLineNext(Cmd));
    }
    else if (strcasecmp(Action, "conf")==0)
    {
        Name=CopyStr(Name, CommandLineNext(Cmd));
        Server=CopyStr(Server, CommandLineNext(Cmd));
    }
    else if (strcasecmp(Action, "config")==0)
    {
        Name=CopyStr(Name, CommandLineNext(Cmd));
        Server=CopyStr(Server, CommandLineNext(Cmd));
    }
    else if (strcasecmp(Action, "list")==0) Name=CopyStr(Name, CommandLineNext(Cmd));
    else if (strcasecmp(Action, "del")==0) Name=CopyStr(Name, CommandLineNext(Cmd));
    else if (strcasecmp(Action, "delete")==0) Name=CopyStr(Name, CommandLineNext(Cmd));
    else if (strcasecmp(Action, "protocols")==0) Name=CopyStr(Name, CommandLineNext(Cmd));
    else if (strcasecmp(Action, "server")==0) Server=CopyStr(Server, CommandLineNext(Cmd));
    else if (strcasecmp(Action, "connect")==0)
    {
        Server=CopyStr(Server, CommandLineNext(Cmd));
        if (! IsServerURL(Server))
        {
            Name=CopyStr(Name, Server);
            Server=CopyStr(Server, "");
        }
    }
    else PrintHelp();


    if (CommandLineValid(Action, Name, Server))
    {
        Ctx=VpnCtxCreate(Action, Name, Server);

        if (Ctx)
        {
            Arg=CommandLineNext(Cmd);
            while (Arg)
            {
                if (strcmp(Arg, "-c")==0) Ctx->ConfFile=CopyStr(Ctx->ConfFile, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-conf")==0) Ctx->ConfFile=CopyStr(Ctx->ConfFile, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-a")==0) Ctx->AllowedIPs=CopyStr(Ctx->AllowedIPs, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-A")==0) Ctx->RemoteAllowedIPs=CopyStr(Ctx->RemoteAllowedIPs, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-d")==0) Ctx->Dev=CopyStr(Ctx->Dev, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-dev")==0) Ctx->Dev=CopyStr(Ctx->Dev, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-ldev")==0) Ctx->Dev=CopyStr(Ctx->Dev, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-local-dev")==0) Ctx->Dev=CopyStr(Ctx->Dev, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-D")==0) Ctx->RemoteDev=CopyStr(Ctx->RemoteDev, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-rdev")==0) Ctx->RemoteDev=CopyStr(Ctx->RemoteDev, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-remote-dev")==0) Ctx->RemoteDev=CopyStr(Ctx->RemoteDev, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-L")==0) Ctx->LocalAddress=CopyStr(Ctx->LocalAddress, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-R")==0) Ctx->RemoteAddress=CopyStr(Ctx->RemoteAddress, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-local-address")==0) Ctx->LocalAddress=CopyStr(Ctx->LocalAddress, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-remote-address")==0) Ctx->RemoteAddress=CopyStr(Ctx->RemoteAddress, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-laddr")==0) Ctx->LocalAddress=CopyStr(Ctx->LocalAddress, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-raddr")==0) Ctx->RemoteAddress=CopyStr(Ctx->RemoteAddress, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-cid")==0) Ctx->ClientID=CopyStr(Ctx->ClientID, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-id")==0) Ctx->ClientID=CopyStr(Ctx->ClientID, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-user")==0) Ctx->UserName=CopyStr(Ctx->UserName, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-pass")==0) Ctx->Password=CopyStr(Ctx->Password, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-passwd")==0) Ctx->Password=CopyStr(Ctx->Password, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-password")==0) Ctx->Password=CopyStr(Ctx->Password, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-pw")==0) Ctx->Password=CopyStr(Ctx->Password, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-psk")==0) Ctx->PreSharedKey=CopyStr(Ctx->PreSharedKey, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-cert")==0) Ctx->ClientCert=CopyStr(Ctx->ClientCert, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-key")==0) Ctx->ClientKey=CopyStr(Ctx->ClientKey, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-k")==0) Ctx->ClientKey=CopyStr(Ctx->ClientKey, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-server-key")==0) Ctx->VerifyCert=CopyStr(Ctx->VerifyCert, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-ca")==0) Ctx->VerifyCert=CopyStr(Ctx->VerifyCert, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-ciphers")==0) Ctx->Ciphers=CopyStr(Ctx->Ciphers, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-dns")==0) Ctx->DNS=CopyStr(Ctx->DNS, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-nodns")==0) GlobalFlags |= FLAG_NODNS;
                else if (strcmp(Arg, "-mtu")==0) Ctx->MTU=atoi(CommandLineNext(Cmd));
                else if (strcmp(Arg, "-ppp-auth")==0) Ctx->PPPAuth=CopyStr(Ctx->PPPAuth, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-up")==0) Ctx->UpFile=CopyStr(Ctx->UpFile, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-down")==0) Ctx->DownFile=CopyStr(Ctx->DownFile, CommandLineNext(Cmd));
                else if (strcmp(Arg, "-local-sudo")==0) GlobalFlags= (GlobalFlags & ~FLAG_SU) | FLAG_SUDO;
                else if (strcmp(Arg, "-local-su")==0) GlobalFlags= (GlobalFlags & ~FLAG_SUDO) | FLAG_SU;
                else if (strcmp(Arg, "-remote-sudo")==0) GlobalFlags = (GlobalFlags & ~FLAG_REMOTE_SU) | FLAG_REMOTE_SUDO;
                else if (strcmp(Arg, "-remote-su")==0) GlobalFlags = (GlobalFlags & ~FLAG_REMOTE_SUDO) | FLAG_REMOTE_SU;
                else if (strcmp(Arg, "-V")==0) Ctx->Flags |= VPN_VERIFY_PEER;
                else if (strcmp(Arg, "-verify")==0) Ctx->Flags |= VPN_VERIFY_PEER;
                else if (strcmp(Arg, "-tcp")==0) Ctx->Flags |= VPN_TCP;
                else if (strcmp(Arg, "-s")==0) GlobalFlags |= FLAG_REMOTE_SU;
                else if (strcmp(Arg, "-S")==0) GlobalFlags |= FLAG_REMOTE_SUDO;
                else if (strcmp(Arg, "-N")==0) GlobalFlags &= ~(FLAG_REMOTE_SU |  FLAG_REMOTE_SUDO);
                else if (strcmp(Arg, "-debug")==0) GlobalFlags |= FLAG_DEBUG;
                else if (strcmp(Arg, "-verbose")==0) GlobalFlags |= FLAG_VERBOSE;


                Arg=CommandLineNext(Cmd);
            }
        }
    }

    free(Cmd);

    Destroy(Action);
    Destroy(Server);
    Destroy(Name);

    return(Ctx);
}

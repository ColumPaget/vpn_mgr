#include "ssl_server.h"
#include "run_command.h"
#include "ppp.h"
#include "net.h"

static int SSLAuth(TVpn *Vpn, STREAM *S)
{
    const char *ptr;

    ptr=STREAMGetValue(S,"SSL:CertificateVerify");
    if (strcmp(ptr, "OK")==0) return(TRUE);

    return(FALSE);
}


static void SSLShutdownConnection(ListNode *Connections, STREAM *Con, STREAM *Peer)
{
    ListDeleteItem(Connections, Con);
    ListDeleteItem(Connections, Peer);

    STREAMClose(Peer);
    STREAMClose(Con);
}


static void SSLProcess(ListNode *Connections, STREAM *S)
{
    int result;
    char *Buffer=NULL;
    STREAM *Peer;

    Peer=STREAMGetItem(S, "Peer");
    Buffer=SetStrLen(Buffer, 4096);
    result=STREAMReadBytes(S, Buffer, 4096);
    if (Peer && (result > 0)) STREAMWriteBytes(Peer, Buffer, result);
    else SSLShutdownConnection(Connections, S, Peer);

    Destroy(Buffer);
}



static void SSLAcceptClient(TVpn *Vpn, ListNode *Connections, STREAM *Server)
{
    STREAM *PPPD, *Client;
    char *Tempstr=NULL;

    Client=STREAMServerAccept(Server);
    if (SSLAuth(Vpn, Client))
    {
        Vpn->RemoteAddress=NetDevFindNextIP(Vpn->RemoteAddress, "172.16.0");
        PPPD=PPPDLaunch(Vpn);

        ListAddItem(Connections, PPPD);
        STREAMSetItem(PPPD, "Peer", Client);

        ListAddItem(Connections, Client);
        STREAMSetItem(Client, "Peer", PPPD);
    }
    else STREAMClose(Client);

    Destroy(Tempstr);
}



void SSLServer(TVpn *Vpn)
{
    STREAM *Server, *S;
    ListNode *Connections;
    char *Host=NULL, *Port=NULL, *URL=NULL, *Tempstr=NULL;

    if (! StrValid(Vpn->ClientCert))
    {
        fprintf(stderr, "No TLS/SSL certificate supplied\n");
        return;
    }

    if (! StrValid(Vpn->ClientKey))
    {
        fprintf(stderr, "No TLS/SSL key supplied\n");
        return;
    }


    ParseURL(Vpn->Server, NULL, &Host, &Port, NULL, NULL, NULL, NULL);

    Connections=ListCreate();
    URL=MCopyStr(URL, "tls:", Host, ":", Port, NULL);
    Tempstr=MCopyStr(Tempstr, "rw SSL:VerifyFile=", Vpn->VerifyCert, NULL);
    Tempstr=MCatStr(Tempstr, " SSL:CertFile=", Vpn->ClientCert, " SSL:KeyFile=", Vpn->ClientKey,  NULL);
    Server=STREAMServerNew(URL, Tempstr);
    ListAddItem(Connections, Server);

    while (! (GlobalFlags & FLAG_EXIT))
    {
        S=STREAMSelect(Connections, NULL);
        if (S)
        {
            if (S==Server) SSLAcceptClient(Vpn, Connections, Server);
            else SSLProcess(Connections, S);
        }
    }

    Destroy(Tempstr);
    Destroy(Host);
    Destroy(Port);
    Destroy(URL);
}


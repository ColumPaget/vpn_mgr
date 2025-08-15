#include "ssl_server.h"
#include "run_command.h"
#include "ppp.h"
#include "net.h"


#define SSL_AUTH_CERTIFICATE 1
#define SSL_AUTH_USER 2
#define SSL_AUTH_OKAY 3

static int SSLAuth(TVpn *Vpn, STREAM *S)
{
    const char *ptr, *p_UserName, *p_PeerIP;
    char *Token=NULL;
    int AuthFlags=0;
    int RetVal=FALSE;

    p_PeerIP=STREAMGetValue(S, "PeerIP");

    //did we specify the need for a specific username?
    p_UserName=STREAMGetValue(S, "SSL:CertificateCommonName");
    if (StrValid(p_UserName))
		{
    // has the certificate been verified?
    ptr=STREAMGetValue(S,"SSL:CertificateVerify");
    if (StrValid(ptr) && (strcmp(ptr, "OK")==0) ) AuthFlags |= SSL_AUTH_CERTIFICATE;

    LogEvent(VPN_LOG_SYSLOG| VPN_LOG_OKAY, "NEW CLIENT:", "%s@%s  CertificateStatus: %s. CertificateIssuer: %s. Encryption: %s. RequiredAuthLevel: %s",  p_UserName, p_PeerIP, ptr, STREAMGetValue(S, "SSL:CertificateIssuer"), STREAMGetValue(S, "SSL:CipherDetails"), Vpn->ServerAuth);
    }

    if (StrValid(Vpn->ServerAuth))
    {
        if (CompareStrNoCase(Vpn->ServerAuth, "open")==0) AuthFlags |= SSL_AUTH_OKAY;

        if (StrValid(p_UserName))
        {
            // 'cert' means that just having a recognized certificate is enough
            if (CompareStrNoCase(Vpn->ServerAuth, "cert")==0) AuthFlags |= SSL_AUTH_USER;
            // 'system' means the username must exist as a real user in /etc/passwd or equivalent
            else if (CompareStrNoCase(Vpn->ServerAuth, "system")==0)
            {
                if (StrValid(p_UserName) && getpwnam(p_UserName)) AuthFlags |= SSL_AUTH_USER;
            }
            else if (strncasecmp(Vpn->ServerAuth, "users:", 6)==0)
            {
                ptr=Vpn->ServerAuth+6;
                while (isspace(*ptr)) ptr++;
 	               if (InStringList(p_UserName, ptr, ",")) AuthFlags |= SSL_AUTH_USER;
                else LogEvent(VPN_LOG_SYSLOG| VPN_LOG_OKAY, "CLIENT AUTH", "[%s] not in list of allowed users: %s\n", p_UserName, ptr);
					}
            else if (strncasecmp(Vpn->ServerAuth, "ips:", 4)==0)
            {
                ptr=Vpn->ServerAuth+4;
                while (isspace(*ptr)) ptr++;
                if (InStringList(p_PeerIP, ptr, ",")) AuthFlags |= SSL_AUTH_USER;
                else LogEvent(VPN_LOG_SYSLOG| VPN_LOG_OKAY, "CLIENT AUTH", "[%s] not in list of allowed ips: %s\n", p_PeerIP, ptr);
            }
            else if (strncasecmp(Vpn->ServerAuth, "ip-only:", 8)==0)
            {
                ptr=Vpn->ServerAuth+8;
                while (isspace(*ptr)) ptr++;
                if (InStringList(p_PeerIP, ptr, ",")) AuthFlags |= SSL_AUTH_OKAY;
                else LogEvent(VPN_LOG_SYSLOG| VPN_LOG_OKAY, "CLIENT AUTH", "[%s] not in list of allowed ips: %s\n", p_PeerIP, ptr);
            }
        }
        else LogEvent(VPN_LOG_SYSLOG| VPN_LOG_OKAY, "NEW CLIENT:", "%s Lacks Certificate. RequiredAuthLevel: %s", STREAMGetValue(S, "PeerIP"), Vpn->ServerAuth);
    }
    else AuthFlags |= SSL_AUTH_USER;


    if (AuthFlags == SSL_AUTH_OKAY)
    {
        LogEvent(VPN_LOG_SYSLOG| VPN_LOG_OKAY, "CONNECTED", "%s@%s authenticated by certificate authority %s. Encryption: %s.", STREAMGetValue(S, "SSL:CertificateCommonName"), STREAMGetValue(S, "PeerIP"), STREAMGetValue(S, "SSL:CertificateIssuer"), STREAMGetValue(S, "SSL:CipherDetails"));
        RetVal=TRUE;
    }
    else LogEvent(VPN_LOG_SYSLOG| VPN_LOG_ERROR, "CONNECT FAIL", "%s@%s unauthorized", STREAMGetValue(S, "SSL:CertificateCommonName"), STREAMGetValue(S, "PeerIP"));

    Destroy(Token);
    return(RetVal);
}


static void SSLShutdownConnection(ListNode *Connections, STREAM *Con, STREAM *Peer)
{
    LogEvent(VPN_LOG_SYSLOG, "DISCONNECT", "%s@%s connection closed", STREAMGetValue(Peer, "SSL:CertificateCommonName"), STREAMGetValue(Peer, "PeerIP"));

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
        LogEvent(VPN_LOG_SYSLOG| VPN_LOG_ERROR, "ERROR", "No TLS/SSL certificate supplied");
        return;
    }

    if (! StrValid(Vpn->ClientKey))
    {
        LogEvent(VPN_LOG_SYSLOG| VPN_LOG_ERROR, "ERROR", "No TLS/SSL key supplied");
        return;
    }

    if (! StrValid(Vpn->VerifyCert))
    {
        LogEvent(VPN_LOG_SYSLOG| VPN_LOG_ERROR, "ERROR", "No TLS/SSL user verfication C.A. certificate supplied");
        return;
    }


    if (! StrValid(Vpn->LocalAddress)) Vpn->LocalAddress=CopyStr(Vpn->LocalAddress, "172.16.0.1");

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


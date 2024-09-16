#include "ssl_client.h"
#include "ppp.h"
#include "route.h"
#include "net.h"

void PPPSSLClientVpnUp(TVpn *Vpn)
{
    STREAM *S;
    char *Tempstr=NULL;
    char *Proto=NULL, *Host=NULL, *Port=NULL, *User=NULL, *URL=NULL;
    const char *ptr;
    int result;

    UnpackURL(Vpn->Server, &Proto, &Host, &Port, &User, NULL, NULL, NULL);
    RouteSetupVpnServer(Host);

    URL=MCopyStr(URL, "tls:", Host, ":", Port, NULL);
    Tempstr=MCopyStr(Tempstr, "rw SSL:CertFile=", Vpn->ClientCert, NULL);
    if (StrValid(Vpn->ClientKey)) Tempstr=MCatStr(Tempstr, " SSL:KeyFile=", Vpn->ClientKey, NULL);
    else Tempstr=MCatStr(Tempstr, " SSL:KeyFile=", Vpn->ClientCert, NULL);

    if (StrValid(Vpn->VerifyCert)) Tempstr=MCatStr(Tempstr, " SSL:VerifyFile=", Vpn->VerifyCert, NULL);
    if (StrValid(Vpn->Ciphers)) Tempstr=MCatStr(Tempstr, " SSL:PermittedCiphers=", Vpn->Ciphers, NULL);

    S=STREAMOpen(URL, Tempstr);
    if (S)
    {
        TerminalPrint(Terminal, "~mTLS/SSL:~0 ~gConnected~0 encryption: [%s %s bit]\n", STREAMGetValue(S, "SSL:CipherDetails"), STREAMGetValue(S, "SSL:CipherBits"));
        TerminalPrint(Terminal, "~mTLS/SSL:~0 Server Certificate: issued by: [%s] for [%s] from [%s] to [%s]\n", STREAMGetValue(S, "SSL:CertificateIssuer"), STREAMGetValue(S, "SSL:CertificateCommonName"), STREAMGetValue(S, "SSL:CertificateNotBefore"), STREAMGetValue(S, "SSL:CertificateNotAfter"));


        ptr=STREAMGetValue(S, "SSL:CertificateVerify");
        if ( StrValid(ptr) && (strcmp(ptr, "OK")==0) )
        {
            TerminalPrint(Terminal, "~mTLS/SSL:~0 ~gCertificate Valid: This link is protected from man-in-the-middle attacks~0\n");
            Vpn->Flags &= ~VPN_VERIFY_PEER; //we no longer need to verify the peer by other means, like ppp etc.
        }
        else TerminalPrint(Terminal, "~mTLS/SSL:~0 ~rCANNOT CONFIRM SERVER CERTIFICATE: %s. This link is vulnerable to man-in-the-middle-attacks~0\n", ptr);

        PPPDProcess(Vpn, S);
        STREAM(S);
    }
    else TerminalPrint(Terminal, "~rFATAL:~0 Can't connect to [%s]\n", URL);

    Destroy(Tempstr);
    Destroy(Proto);
    Destroy(Host);
    Destroy(Port);
    Destroy(User);
    Destroy(URL);
}


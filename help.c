#include "help.h"

void PrintHelp()
{
    printf("Usage:\n");
    printf("  vpn_mgr protocols                        check which VPN protocols we can use on the local machine\n");
    printf("  vpn_mgr add <name> <url>                 add/update a VPN configuration\n");
    printf("  vpn_mgr list                             list configured VPNs\n");
    printf("  vpn_mgr config <name> <url>\n");
    printf("  vpn_mgr server <url>                     run as a VPN server (ppptls/pppssl mode only)\n");
    printf("  vpn_mgr connect <name>                   connect to VPN configured under name <name>\n");
    printf("  vpn_mgr connect <url>                    connect to VPN by url (no use of config files, everything on command-line)\n");
    printf("\n");
    printf("Options:\n");
    printf("  -c <path>             path to vendor-supplied vpn (wireguard or openvpn) config file\n");
    printf("  -conf <path>          path to vendor-supplied vpn (wireguard or openvpn) config file\n");
    printf("  -d <dev>              name of LOCAL network device (e.g. tun0) to use\n");
    printf("  -dev <dev>            name of LOCAL network device (e.g. tun0) to use\n");
    printf("  -ldev <dev>           name of LOCAL network device (e.g. tun0) to use\n");
    printf("  -local-dev <dev>      name of LOCAL network device (e.g. tun0) to use\n");
    printf("  -D <dev>              name of REMOTE network device (e.g. tun0) to use\n");
    printf("  -rdev <dev>           name of REMOTE network device (e.g. tun0) to use\n");
    printf("  -remote-dev <dev>     name of REMOTE network device (e.g. tun0) to use\n");
    printf("  -a <ip list>          list of IPs accessible at the remote end of the vpn connection\n");
    printf("  -A <ip list>          list of remote IPs to be routed DOWN the vpn connection\n");
    printf("  -L <ip address>       ip address of LOCAL end of tunnel\n");
    printf("  -laddr <ip address>   ip address of LOCAL end of tunnel\n");
    printf("  -local-address <ip address>   ip address of LOCAL end of tunnel\n");
    printf("  -R <ip address>       ip address of REMOTE end of tunnel\n");
    printf("  -raddr <ip address>   ip address of REMOTE end of tunnel\n");
    printf("  -remote-address <ip address>   ip address of REMOTE end of tunnel\n");
    printf("  -tcp                  use tcp, not udp for connection (this only applies to openvpn which can use either\n");
    printf("  -cert <path>          path to file holding the client certificate for a vpn connection\n");
    printf("  -key <path>           path to file holding the client private key for a vpn connection (if not supplied, assumed to be stored in -cert file along with certificate)\n");
    printf("  -k <path>             path to file holding the client private key for a vpn connection\n");
    printf("  -server-key <path>    path to file holding the CA certificates for server verification (or server public key for wireguard)\n");
    printf("  -psk <path>           path to file holding the pre-shared-key for vpns using this instead of certificates\n");
    printf("  -ca <path>            path to file holding the CA certificates for server verification (or server public key for wireguard)\n");
    printf("  -ciphers <list>       comma-seperated list of ciphers to use\n");
    printf("  -up <file list>       colon-seperated list of scripts to run when vpn comes up (default /etc/vpn_mgr/<name>.up:/etc/vpn_mgr/default.up)\n");
    printf("  -down <file list>     colon-seperated list of scripts to run when vpn goes down (default /etc/vpn_mgr/<name>.down:/etc/vpn_mgr/default.down)\n");
    printf("  -V                    verify peer at the ppp level on ppp based vpns. Requires the server to identify itself with a username/pasword\n");
    printf("  -verify               verify peer at the ppp level on ppp based vpns. Requires the server to identify itself with a username/pasword\n");
    printf("  -user <username>      username for vpns that support username/password authentication\n");
    printf("  -pass <password>      password/preshared key for vpns that support username/password or PSK authentication\n");
    printf("  -passwd <password>    password/preshared key for vpns that support username/password or PSK authentication\n");
    printf("  -password <password>  password/preshared key for vpns that support username/password or PSK authentication\n");
    printf("  -pw <password>        password/preshared key for vpns that support username/password or PSK authentication\n");
    printf("  -local-sudo           force use of sudo locally, rather than trying both sudo and su\n");
    printf("  -local-su             force use of su locally, rather than trying both sudo and su\n");
    printf("  -remote-sudo          force use of sudo at remote end, rather than trying both sudo and su\n");
    printf("  -remote-su            force use of su at remote end, rather than trying both sudo and su\n");
    printf("  -N                    use neither su or sudo at the remote end, assume remote is setup to run with root permission\n");
    printf("  -dns <ip list>        comma-separated list of dns servers to use with this vpn\n");
    printf("  -dns peer             use dns server list supplied by ppp based vpns\n");
    printf("  -nodns                do not use dns servers saved in vpn config\n");
    printf("  -ppp-auth             use ppp-chap authentication\n");
    printf("  -id <string>          set a 'client id' string. Currently only used to set the ipparam of ppp-based networks in order to identify who/what is connecting and allow setting per-connection rules on the server.\n");
    printf("  -cid <string>         set a 'client id' string. Currently only used to set the ipparam of ppp-based networks in order to identify who/what is connecting and allow setting per-connection rules on the server.\n");
    printf("  -mtu <mtu>            mtu value for openvpn and ppp based vpns\n");
    printf("  -verbose              output more info about what vpn_mgr is doing\n");
    printf("  -debug                spew lots of debugging info\n");


    printf("\n\n");
}

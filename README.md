SYNOPSIS
========

vpn_mgr is a command-line application to manage the client-side of site-to-site vpns on unix, and particularly linux. 


DESCRIPTION
===========

vpn_mgr supports wireguard, openvpn, ssh, ppp-over-ssh and ppp-over-tls (aka ppp-over ssl) vpns. It's particularly focused on 'poor mans vpn' solutions to lash up one-way connections between sites, rather than on being a client to commercial vpn solutions. In most cases is can only handle the client side of the connection, requiring a site administrator to have set up the server using other tools. The exception to this is ppp-over-tls, where vpn_mgr can act as the server.

While vpn_mgr has been seen to work with commercial vpn providers (e.g. protonvpn) this is not it's focus. It can suffer from issues like DNS leakage, as apps running before the vpn is activated sometimes cache their DNS settings internally, and continue to use local DNS rather than DNS via the vpn.

vpn_mgr tries to present a common, simplified interface to all supported protocols. This means many features of advanced protocols like openvpn are not directly supported. However, such features can be utilized through a config-file that is opaque to vpn_mgr, but stored by it and passed directly to the vpn software.

USAGE
=====

```
  vpn_mgr protocols                        check which VPN protocols we can use on the local machine
  vpn_mgr connect <url>  <options>         connect to VPN by url (no use of config files, everything on command-line)
  vpn_mgr connect <name> <options>         connect to VPN configured under name <name>
  vpn_mgr list                             list VPNs in the saved configurations
  vpn_mgr add <name> <url> <options>       add/update a VPN in the saved configs
  vpn_mgr delete <name>                    delete a VPN from the saved configs
  vpn_mgr config <name> <url>
  vpn_mgr server <url>                     run as a VPN server (ppptls/pppssl mode only)
```

OPTIONS
=======

```
  -c <path>             path to vendor-supplied vpn (wireguard or openvpn) config file
  -conf <path>          path to vendor-supplied vpn (wireguard or openvpn) config file
  -d <dev>              name of LOCAL network device (e.g. tun0) to use
  -dev <dev>            name of LOCAL network device (e.g. tun0) to use
  -ldev <dev>           name of LOCAL network device (e.g. tun0) to use
  -local-dev <dev>      name of LOCAL network device (e.g. tun0) to use
  -D <dev>              name of REMOTE network device (e.g. tun0) to use
  -rdev <dev>           name of REMOTE network device (e.g. tun0) to use
  -remote-dev <dev>     name of REMOTE network device (e.g. tun0) to use
  -a <ip list>          list of IPs accessible at the remote end of the vpn connection
  -A <ip list>          list of remote IPs to be routed DOWN the vpn connection
  -L <ip address>       ip address of LOCAL end of tunnel
  -laddr <ip address>   ip address of LOCAL end of tunnel
  -local-address <ip address>   ip address of LOCAL end of tunnel
  -R <ip address>       ip address of REMOTE end of tunnel
  -raddr <ip address>   ip address of REMOTE end of tunnel
  -remote-address <ip address>   ip address of REMOTE end of tunnel
  -tcp                  use tcp, not udp for connection (this only applies to openvpn which can use either
  -cert <path>          path to file holding the client certificate for a vpn connection
  -key <path>           path to file holding the client private key for a vpn connection (if not supplied, assumed to be stored in -cert file along with certificate)
  -k <path>             path to file holding the client private key for a vpn connection
  -server-key <path>    path to file holding the CA certificates for server verification (or server public key for wireguard)
  -psk <path>           path to file holding the pre-shared-key for vpns using this instead of certificates
  -ca <path>            path to file holding the CA certificates for server verification (or server public key for wireguard)
  -ciphers <list>       comma-seperated list of ciphers to use
  -up <file list>       colon-seperated list of scripts to run when vpn comes up (default /etc/vpn_mgr/<name>.up:/etc/vpn_mgr/default.up)
  -down <file list>     colon-seperated list of scripts to run when vpn goes down (default /etc/vpn_mgr/<name>.down:/etc/vpn_mgr/default.down)
  -V                    verify peer at the ppp level on ppp based vpns. Requires the server to identify itself with a username/pasword
  -verify               verify peer at the ppp level on ppp based vpns. Requires the server to identify itself with a username/pasword
  -user <username>      username for vpns that support username/password authentication
  -pass <password>      password/preshared key for vpns that support username/password or PSK authentication
  -passwd <password>    password/preshared key for vpns that support username/password or PSK authentication
  -password <password>  password/preshared key for vpns that support username/password or PSK authentication
  -pw <password>        password/preshared key for vpns that support username/password or PSK authentication
  -local-sudo           force use of sudo locally, rather than trying both sudo and su
  -local-su             force use of su locally, rather than trying both sudo and su
  -remote-sudo          force use of sudo at remote end, rather than trying both sudo and su
  -remote-su            force use of su at remote end, rather than trying both sudo an[?9l[?1000ld su
  -N                    use neither su or sudo at the remote end, assume remote is setup to run with root permission
  -dns <ip list>        comma-separated list of dns servers to use with this vpn
  -dns peer             use dns server list supplied by ppp based vpns
  -nodns                do not use dns servers saved in vpn config
  -ppp-auth             use ppp-chap authentication
  -id <string>          set a 'client id' string. Currently only used to set the ipparam of ppp-based networks in order to identify who/what is connecting and allow setting per-connection rules on the server.
  -cid <string>         set a 'client id' string. Currently only used to set the ipparam of ppp-based networks in order to identify who/what is connecting and allow setting per-connection rules on the server.
  -mtu <mtu>            mtu value for openvpn and ppp based vpns
  -verbose              output more info about what vpn_mgr is doing
  -debug                spew lots of debugging info
```



SAVED CONFIGS
=============

The 'add' command can be used to save a named configuration, which can then be simply called with 'connect'. However, there are two ways that configs can be saved. 

1) All options supplied on the command-line. In this case copies are taken of all keys and certificates, so that the original certificate/key files can be deleted and vpn_mgr still has it's private copy.

2) Using the '-c' or '-conf' command-line options. In this case a copy is taken of the config file, but it is not parsed, and no copies of any other files referenced in it are taken. This option targets wireguard or openvpn configs, where the entire setup, including certificates, can be provided 'inline' in a single file. This supplied file is then passed directly to wireguard or openvpn. This also allows use of advanced features of these vpns that vpn_mgr isn't aware of.


UP/DOWN SCRIPTS AND FIREWALLS
=============================

Whilst vpn_mgr makes some effort to setup routes, it does not touch firewalls, as linux netfilter is a moving target. Is it ipfwadm, ipchains, iptables, xftables, or some kind of service/daemon, possibly something systemd? However, it does run 'up' and 'down' scripts when a vpn becomes active or shuts down. These scripts can be specified with the '-up' and '-down' command-line options, which accept a list of filenames in 'PATH' format (seperated by ':'). The default setting for these is '/etc/vpn_mgr/<name>.up:/etc/vpn_mgr/default.up' where <name> is the name of the connection. Firewall and route commands can be added to these scripts as needed.

Alternatively iptables allows the use of wildcards for interface names. e.g.: `ppp+` and `tun+` that allow setting long-lived rules for, in this case, all 'ppp' or 'tun' interfaces. This can be used to create firewall scripts that are run at system startup, but can setup rules that apply to all future ppp or tun interfaces as they are created.


VPN URL TYPES
=============

The following url types are supported:

  * ssh:      -  native ssh 'tunnel' vpn
  * pppssh:   -  ppp over ssh vpn
  * wg:       -  wireguard vpn
  * ovpn:     -  openvpn vpn
  * ppptls:   -  ppp over tls/ssl vpn
  * pppssl:   -  ppp over tls/ssl vpn


SSH VPNS
========

VPNs using openssh's native 'tunnel' system are supported. All that's needed at the server end is a 'tun' device to use. Unfortuantely this usually has to be setup and agreed in advance. 

As we will not usually be logging into the ssh server as root, so sshd cannot setup a fresh tun device. Thus one has to be set up in advance. e.g. with `ip tuntap add tun3 mode tun` and then, when we connect with vpn_mgr, we would have to give the argument `-rdev 3`.

If we are logging into an ssh server as root, then we can specify 'any' as the remote `-rdev any` and sshd should set up or find a tun device it can use.



WIREGUARD VPNS
==============

Wireguard VPNs are supported via either:

 1) wireguard support in the linux kernel, 
 2) wireguard support via the 'boring-tun' userspace app
 3) wireguard support via the 'wireguard-go' userspace app

vpn_mgr can use a standard wireguard config file via the '-c' or -conf' options.

Wireguard is essentially a symmetric protocol, without a 'client' and 'server' as we'd normally think of them. Both ends have a private key that they use to encrypt traffic, and a public key that is used both to authenticate and decrypt traffic at the other end.


OPENVPN VPNS
============

Openvpn is one of the most confusing vpn solutions, due to it's wide range of options and configurations, vpn_mgr can use a vendor-supplied config file with the '-c' or '-conf' options. vpn_mgr only supports openvpn's tls based encryption, not it's other encryption options. It can support either UDP connections (the default) or TCP connections using the '-tcp' option.


PPP based VPNS
==============

pppd is a program that talks a protocol that can send IP traffic over any kind of point-to-point connections, be that serial cables, IRDA links, bluetooth, or tcp socket connections, or anything else where bytes go in one end and come out the other end. If the connection is encrypted, e.g. using ssh or tls/ssl, then we have a VPN. However, there's normally a fair bit of 'plumbing' to hook ppp up to the chosen transport method. 


PPP over SSH VPNS
=================

pppd is a program that creates a tcp/ip network over any link that can carry a stream of characters: serial links, ssh connections, tls connections, etc, etc. ppp-over-ssh is one of the easiest systems to get working with vpn_mgr. However, ppp requires superuser/root access in order to create network 'devices' at both ends. By default vpn_mgr assumes it has to use 'su' or 'sudo' at both ends to run pppd. If it finds it's running as root at the local end, then it won't attempt to use su/sudo. vpn_mgr assumes it's not being run as root at the remote end, and attempts to use 'su' or 'sudo' there too. However, it's possible for the server adminstrator to setup pppd to be setuid, so that using su/sudo is unnecessary. In this situation one can use the '-N' command-line option to tell vpn_mgr not to use su/sudo at the remote end.

By default vpn_mgr passes the 'noauth' option to pppd, disabling ppp-level authentication at both ends. In the case of PPP-over-SSH vpns there's no reason to use ppp-level authentication, as ssh already provides superior authentication systems.

ppp-over-ssh does require the server administrator to have setup a firewall that allows traffic to pass through ppp connections.

Sometimes it's desirable for a ppp link to identify where it's coming from or what class of connection it is, perhaps to apply different routing or firewall rules to the link. pppd provides the 'ipparam' option, which is passed to 'ip-up' and 'ip-down' scripts, allowing the server to add specific routes/rules/logging etc to a given link. vpn_mgr supports this system via the '-cid' or '-id' options, whose argument is passed as the 'ipparam' to pppd.





PPP OVER TLS/SSL VPNS
=====================

vpn_mgr can connect to vpns that run ppp over a tls (aka ssl) connection. There's numerous ways of setting up a ppp-over-tls server, using stunnel, ncat or vpn_mgr itself. vpn_mgr can be used at the server end with a command like so:

```
vpn_mgr server tls:0.0.0.0:9999
```

The above example runs a tls/ssl server on port 9999. This server will be available on all network interfaces, due to the '0.0.0.0' address component. In order to only run the server on a specific network interface supply that interface's IP address, like so:


```
vpn_mgr server tls:192.168.1.1:9999
```


Other methods of creating a tls server include stunnel. Here is an example stunnel config file:

```
[ppp-vpn]
client=no
accept=9999
cert=/etc/tls-server/server.crt
key=/etc/tls-server/server.key
exec=/usr/sbin/pppd
execArgs=pppd 921600 noauth nodetach nodefaultroute lcp-echo-interval 30 lcp-echo-failure 4 172.16.0.2:
pty=yes
verifyPeer = yes
CAfile = /etc/tls-server/ca.crt
```

this config launches a pppd process after a client has connected to port 9999 and been verified using a client certificate. The cert and key arguments supply the certificate and private key that the server will use, whereas the 'CAfile' argument supplies a Certificate Authority certificate file, which allows verifying the certificate sent by the client. The 'verifyPeer' argument specifies that the client must authenticate using a client certificate and the 'pty' argument is needed because pppd usually expects to be run on a pty or serial device.


Another program that can be used to create a tls server is the 'ncat' program that ships with the 'nmap' network tool.

```
#!/bin/sh

ncat --listen 9999 --ssl-cert /etc/tls-server/server.crt --ssl-key /etc/tls-server/server.key --exec "/sbin/pppd notty 921600 require-chap nodetach nodefaultroute lcp-echo-interval 30 lcp-echo-failure 4 172.16.0.3: "

```

This setup requires using pppd's own authentication system, because ncat doesn't support checking/authenticating client certificates when it's acting as a server (the --ssl-verify option only relates to ncat as a client). Thus the pppd command in this script contains the 'require-chap' option. The 'notty' pppd option tells ppp that it's standard input won't be a pty or serial device as it would normally expect.

Whilst this setup works, it is perhaps risky as it exposes ppp traffic to anyone who connects to the port, and is dependant on password-based, rather than certificate-based, authentication.

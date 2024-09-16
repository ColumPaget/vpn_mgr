OBJ=common.o route.o run_command.o command_line.o vpn_ctx.o vpn_config.o updown.o tuntap.o net.o resolv.o ppp.o wireguard.o openvpn.o ssh.o ssl_client.o ssl_server.o pppssh.o help.o
LIBS=libUseful-5/libUseful.a -lz -lcap -lcrypto -lcrypto -lssl -lssl  
FLAGS=-g -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -D_FILE_OFFSET_BITS=64 -DHAVE_LIBSSL=1 -DHAVE_LIBSSL=1 -DHAVE_LIBCRYPTO=1 -DHAVE_LIBCRYPTO=1 -DHAVE_LIBCAP=1 -DHAVE_LIBZ=1 -DHAVE_STDIO_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_STRINGS_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_UNISTD_H=1 -DSTDC_HEADERS=1 -DHAVE_CAPABILITIES=1

all: $(OBJ) libUseful-5/libUseful.a
	gcc $(FLAGS) -ovpn_mgr main.c $(OBJ) $(LIBS)

libUseful-5/libUseful.a:
	$(MAKE) -C libUseful-5

common.o: common.h common.c
	gcc $(FLAGS) -c common.c

route.o: route.h route.c
	gcc $(FLAGS) -c route.c

updown.o: updown.h updown.c
	gcc $(FLAGS) -c updown.c

run_command.o: run_command.h run_command.c
	gcc $(FLAGS) -c run_command.c

command_line.o: command_line.h command_line.c
	gcc $(FLAGS) -c command_line.c

vpn_ctx.o: vpn_ctx.h vpn_ctx.c
	gcc $(FLAGS) -c vpn_ctx.c

vpn_config.o: vpn_config.h vpn_config.c
	gcc $(FLAGS) -c vpn_config.c

tuntap.o: tuntap.h tuntap.c
	gcc $(FLAGS) -c tuntap.c

net.o: net.h net.c
	gcc $(FLAGS) -c net.c

wireguard.o: wireguard.h wireguard.c
	gcc $(FLAGS) -c wireguard.c

openvpn.o: openvpn.h openvpn.c
	gcc $(FLAGS) -c openvpn.c

ppp.o: ppp.h ppp.c
	gcc $(FLAGS) -c ppp.c

ssh.o: ssh.h ssh.c
	gcc $(FLAGS) -c ssh.c

pppssh.o: pppssh.h pppssh.c
	gcc $(FLAGS) -c pppssh.c

ssl_client.o: ssl_client.h ssl_client.c
	gcc $(FLAGS) -c ssl_client.c

ssl_server.o: ssl_server.h ssl_server.c
	gcc $(FLAGS) -c ssl_server.c


resolv.o: resolv.h resolv.c
	gcc $(FLAGS) -c resolv.c

help.o: help.h help.c
	gcc $(FLAGS) -c help.c



clean:
	rm -f *.o *.orig vpn_mgr .*.swp */*.o */*.so */*.a

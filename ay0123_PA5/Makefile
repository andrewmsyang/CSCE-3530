all: dclient dserver
dclient: DHCP_Client.c util.c util.h
	gcc DHCP_Client.c util.c -o dclient -Werror -g
dserver: DHCP_Server.c util.c util.h
	gcc DHCP_Server.c util.c -o dserver -Werror -g -lm
clean:
	rm dclient dserver
	rm client.out server.out

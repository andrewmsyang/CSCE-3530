all: uclient userver
uclient: UDP_Client.c
	gcc UDP_Client.c -o uclient
userver: UDP_Server.c
	gcc UDP_Server.c -o userver
clean:
	rm uclient userver
	rm server.out
	rm client.out

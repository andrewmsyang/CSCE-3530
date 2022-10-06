CC=gcc

main: pyserver client
	echo

pyserver: server.c
	$(CC) server.c -o pyserver

client: client.c
	$(CC) client.c -o client

clean:
	rm pyserver &
	rm client &

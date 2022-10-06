all: tclient tserver
tclient: TCP_Client.c util.c util.h
	gcc TCP_Client.c util.c -o tclient -Werror -g
tserver: TCP_Server.c util.c util.h
	gcc TCP_Server.c util.c -o tserver -Werror -g
clean:
	rm tclient tserver
	rm client.out server.out

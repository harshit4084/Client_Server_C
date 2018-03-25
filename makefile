all:
	
	gcc client.c -o client -lpthread
	gcc server.c -o server -lpthread
	gcc pthread.c -o pthread -lpthread
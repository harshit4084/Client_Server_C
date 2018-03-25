#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
struct node
{
	int id;
	char const **ar;
};

void *myThreadFun(void *a)//char const *argv[])
{
	struct node *MyNode = a;
	int ID = MyNode->id;
	char const **ar = MyNode->ar;
	printf("%s\n", ar[1] );
	int i = 0;
	while (1)
	{
		printf("thread id = %d\n", ID);
		i++;
		printf(" %d\n", i );
		char path[1024];
		//char s="/";
		char workdir[1024];
		int status;
		getcwd(path, 1024);
		getcwd(workdir, 1024);
		strcat(path, "/");
		strcat(path, ar[2]);

		pid_t pid = fork();
		if (pid == -1)
		{
			printf("can't fork, error occured\n");
		}
		else if (pid == 0)
		{
			execl(path, ar[2],  workdir, NULL);
			printf("Unable to find SERVER file in the directory--%s\n", workdir);
			exit(0);
		}
		else
		{
			waitpid(pid, &status, 0);
		}
		sleep(5);
	}
	return NULL;
}
int main(int argc, char const *argv[])
{
	if (argc != 3)
	{
		printf("please enter max number of clients and SERVER file name\n");
		exit(0);
	}

	int max = atoi(argv[1]);
	struct node MyNode[max];
	pthread_t tid[max];
	int i = 0;
	for ( i = 0; i < max; i++)
	{
		MyNode[i].id = i;
		MyNode[i].ar = argv;
		pthread_create(&tid[i], NULL, myThreadFun, &MyNode[i]);
	}
	for ( i = 0; i < max; i++)
	{
		pthread_join(tid[i], NULL);
	}
	exit(0);
}
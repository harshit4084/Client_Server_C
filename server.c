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

//#define SERVERPORT_NUM 9876
#define BUFSIZE 1024

void *myThreadFun(void *a);
void putfile(char *transmit , int ID);// transfer file
void newconnection ( int ServerSocketNUM);
void getfile(char *transmit , int ID);// transfer file

void *exit_function(void *a)
{
    char status[50];
    while (1)
    {
        scanf("%s", status);
        if (strcmp(status, "exit") == 0 || strcmp(status, "quit") == 0)
        {
            exit(0);
        }
    }
    pthread_exit(0);
}

void *myThreadFun(void *a)
{
    int *aa = a;
    pthread_t tid;
    int ServerSocketNUM = *aa, status;

    struct sockaddr_in ClientAddressStruct;
    int Socket_ID, ServerAddrStructlen = sizeof(ClientAddressStruct);
    if ((Socket_ID = accept(ServerSocketNUM, (struct sockaddr *)&ClientAddressStruct, (socklen_t*)&ServerAddrStructlen)) < 0)
    {
        printf("the %d,%d\n", getpid(), getppid() );
        perror("accept ");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        printf("can't fork, error occured\n");
    }
    else if (pid == 0)
    {
        close(ServerSocketNUM);
        newconnection(Socket_ID);
        printf("Unable enter newconnection\n");
        exit(0);
    }
    else
    {
        close(Socket_ID);
        waitpid(pid, &status, 0);
        pthread_create(&tid, NULL, myThreadFun, &ServerSocketNUM);
        pthread_exit(0);
    }
}
void newconnection ( int Socket_ID)
{
    int  c = 0, valread = 0;
    char buffer[BUFSIZE];
    char transmit[BUFSIZE];
    char name[50];
    send(Socket_ID, "ack", strlen("ack"), 0);
    valread = read( Socket_ID , name, 1024);
    send(Socket_ID, "ack", strlen("ack"), 0);
    printf("The client '%s' has connected\n", name );
    while (1)
    {
        c++;
        printf("QUERY %d from client-ID %s : ", c, name);
        valread = read( Socket_ID , buffer, 1024);
        if (valread == 0)
        {
            close(Socket_ID);
            printf("\nConnection lost with %s\n", name);
            exit(0);

        }
        if (valread >= 0)
            buffer[valread] = '\0';
        printf("%s\n", buffer);
        if (strcmp(buffer, "ls") == 0)
        {
            DIR *dp;
            char cwd[1024];
            struct dirent *ep;

            getcwd(cwd, sizeof(cwd));

            send(Socket_ID, cwd, strlen(cwd), 0);
            valread = read( Socket_ID , buffer, 1024);
            dp = opendir(cwd);
            while (ep = readdir(dp))
            {
                if (strcmp(ep->d_name , ".") != 0 && strcmp(ep->d_name , "..") != 0 && strcmp(ep->d_name , "...") != 0 )
                {
                    send(Socket_ID, ep->d_name, strlen(ep->d_name), 0);
                    valread = read( Socket_ID , buffer, 1024);
                }
            }
            send(Socket_ID, "end", 3, 0);
            closedir(dp);
        }
        else if (strcmp(buffer, "cd") == 0)
        {
            send(Socket_ID, "ack", strlen("ack"), 0);
            valread = read( Socket_ID , buffer, 1024);
            if (valread >= 0)
                buffer[valread] = '\0';
            int s = chdir(buffer);
            if (s != 0)
            {
                printf("improper arguments\n");
                send(Socket_ID, "false", strlen("false"), 0);
            }
            else
                send(Socket_ID, "true" , strlen("true"), 0);
            valread = read( Socket_ID , buffer, 1024);//ack
            getcwd(transmit, 1024);
            send(Socket_ID, transmit, strlen(transmit), 0);
        }
        else if (strcmp(buffer, "cwd") == 0)
        {
            getcwd(transmit, 1024);
            send(Socket_ID, transmit, strlen(transmit), 0);
        }
        else if (strcmp(buffer, "close") == 0)
        {
            close(Socket_ID);
            printf("Disconnected from '%s'.\n", name);
            exit(0);
        }
        else if (strcmp(buffer, "chmod") == 0)
        {
            send(Socket_ID, "ack", strlen("ack"), 0);
            valread = read( Socket_ID , buffer, 1024);
            system(buffer);
            send(Socket_ID, "ack", strlen("ack"), 0);
        }
        else if (strcmp(buffer, "put") == 0)
        {
            putfile(transmit, Socket_ID); // transfer file
        }
        else if (strcmp(buffer, "get") == 0)
        {
            getfile(transmit, Socket_ID); // transfer file
        }
        else
        {
            printf("Invalid QUERY : %s\n", buffer);
        }
    }
}

void putfile(char *transmit , int ID)// transfer file
{
    send(ID , "ack" , strlen("ack") , 0 );
    int file, n = 1, m;
    int valread = read( ID , transmit, 1024);//file name

    if (valread != BUFSIZE)
    {
        valread = read( ID , transmit+valread, 1024-valread);//file name
    }
    file = creat(transmit, 0666);

    unsigned long int sz, count = 0;
    char buf[BUFSIZE];

    char *temp = NULL;
    sz = strtol(transmit + (BUFSIZE / 2), &temp, 10);
    if (file == -1)
    {
        send(ID , "nok" , strlen("nok") , 0 );
        printf("failed to create desti\n");
        return;
    }
    else
        send(ID , "ok" , strlen("ok") , 0 );

    while (1)
    {
        n = read( ID , buf, BUFSIZE);
        count += (unsigned long int)n;
        m = write(file, buf, n);
        if (count == sz)
        {
            break;
        }
    }
    send(ID , "ack" , strlen("ack") , 0 );
    close(file);
    return ;
}

void getfile(char *transmit , int ID)// transfer file
{
    send(ID , "ack" , strlen("ack") , 0 );
    int valread = read( ID , transmit, 1024);//file name
    transmit[valread] = '\0';
    int file, n = 1, m;
    char buf[BUFSIZE];
    file = open(transmit, O_RDONLY, 0);
    FILE *fpointer;
    fpointer = fopen( transmit, "r");
    unsigned long int sz, sz2;
    if (file == -1 || fpointer == NULL)
    {
        send(ID , "nok" , strlen("nok") , 0 );
        if (fpointer != NULL)
            fclose (fpointer);
        if (file != -1)
            close(file);
        printf("Source can't be found\n");
        return;
    }
    else
    {
        fseek(fpointer, 0L, SEEK_END);
        sz = ftell(fpointer);
        fclose (fpointer);
        sprintf (transmit, "%lu" , sz );
        send(ID , transmit , strlen(transmit) , 0 );
    }
    valread = read( ID , buf, BUFSIZE);// ok nok
    buf[valread] = '\0';
    if (strcmp(buf, "nok") == 0)
    {
        return;
    }
    else
    {
        char *temp = NULL;
        sz2 = strtol(buf, &temp, 10);
        if (sz != sz2)
        {
            send(ID , "nok" , strlen("nok") , 0 );
            printf("file size ..\n");
            return;
        }
        else
            send(ID , "ok" , strlen("ok") , 0 );
        valread = read( ID , buf, BUFSIZE);// ok nok
    }
    while (1)
    {
        n = read(file, buf, BUFSIZE);
        if (n <= 0)
            break;
        m = send(ID , buf , n , 0 );
    }
    close(file);
    return ;
}

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("please enter max number of clients followed by PORT\n");
        exit(0);
    }
    struct sockaddr_in ServerAddressStruct;
    int ServerSocketNUM, ServerAddrStructlen = sizeof(ServerAddressStruct);

    if ((ServerSocketNUM = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket not created ");
        exit(EXIT_FAILURE);
    }
    ServerAddressStruct.sin_family = AF_INET;
    ServerAddressStruct.sin_addr.s_addr = INADDR_ANY;
    ServerAddressStruct.sin_port = htons( atoi(argv[2]) );
    /* if (setsockopt(ServerSocketNUM, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0) //4084
     {
         printf("Reuse failed");
         exit(1);
     }*/
    if (bind(ServerSocketNUM, (struct sockaddr *)&ServerAddressStruct, ServerAddrStructlen) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    int max = atoi(argv[1]);
    pthread_t tid[max], t;
    if (listen(ServerSocketNUM, max) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    pthread_create(&t, NULL, exit_function, NULL);
    int i = 0;
    for ( i = 0; i < max; i++)
    {
        pthread_create(&tid[i], NULL, myThreadFun, &ServerSocketNUM);
    }
    pthread_exit(0);
}
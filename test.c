#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>


int main(int argc, char const *argv[])
{
    //send(ID , "ack" , strlen("ack") , 0 );
    //int valread = read( ID , transmit, 1024);//file name
    //transmit[valread] = '\0';
    int file, n = 1, m;
    char buf[150];
printf("test1\n");
    file = open( "test.txt"  , O_RDONLY, 0);
    FILE *fpointer;
    fpointer = fopen( "test.txt", "r");
    unsigned long int sz, sz2;
printf("test2 %d\n",n);
    if (file == -1 || fpointer == NULL)
    {
        if(fpointer != NULL)
           { printf("null %d\n", file);}
        //send(ID , "nok" , strlen("nok") , 0 );
        //fclose (fpointer);
        close(file);
        printf("Source can't be found\n");
        return 0;
    }

/*    while (1)
    {
        n = read(file, buf, BUFSIZE);
        if (n <= 0)
            break;
        m = send(ID , buf , n , 0 );
    }
    m = send(ID , "[end}" , strlen("[end}") , 0 );
    close(file);
    valread = read( ID , buf, BUFSIZE);// ack*/
    return 0;
}

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main (int argc, char *argv[] )
{

    if ( argc < 2 )
    {
        error("ERROR, no port provided\n");
    }

    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    int cli_len, portno;
    char message[256];
    pid_t pid;
    int n;

    portno = atoi( argv[1] );
    bzero( (char*)&serv_addr, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons( portno );

    sockfd = socket( AF_INET, SOCK_STREAM, 0 );

    n =  bind ( sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr) );
    if ( n < 0 )
    {
        error("ERROR on binding\n");
    }

    listen( sockfd, 5 );

    do {
        cli_len = sizeof(cli_addr);

        int newsockfd;
        newsockfd = accept( sockfd, (struct sockaddr*)&cli_addr, (socklen_t *)&cli_len);
        if ( newsockfd < 0 )
        {
            printf("%i", newsockfd);
            error("ERROR on accept\n");

        }

        pid = fork();
        if ( pid == 0 )
        {

            close( sockfd );
            sockfd = -1;

            n = send( newsockfd, message, strlen(message), 0);
            if ( n < 0 ) error("ERROR sending\n");

            bzero(message, 256);

            n = recv( newsockfd, message, 256, 0);
            if ( n < 0 ) error("ERROR receiving\n");

            close( newsockfd );
            newsockfd = -1;

        }

    }
    while ( 1 );

    return 0;
}

*/
/*Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux
#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define TRUE   1
#define FALSE  0
#define PORT 8888

int main(int argc , char *argv[])
{
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] ,
          max_clients = 30 , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[1025];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;

    //a message
    char *message = "ECHO Daemon v1.0 \r\n";

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(TRUE)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d
                  \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs
                  (address.sin_port));

            //send new connection greeting message
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )
            {
                perror("send");
            }

            puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);

                    break;
                }
            }
        }

        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }

                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
                    send(sd , buffer , strlen(buffer) , 0 );
                }
            }
        }
    }

    return 0;
}

*/
/*
// Server side C/C++ program to demonstrate Socket programming
#include <sys/socket.h>
#include <stdio.h>
#define PORT 5960
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>

int main()//int argc, char const *argv[])
{
    struct sockaddr_in address;
    int ServerSocketID, new_socket, valread;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024];
    char *hello = "Hello from server";
    printf("%s\n",buffer );
    printf("%s\n",hello );
    // Creating socket file descriptor
    if ((ServerSocketID = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket not created ");
        exit(EXIT_FAILURE);
    }
    printf("%d\n",ServerSocketID );
    return 0;
}*/
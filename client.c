// Client side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define BUFSIZE 1024

void putfile(char **args , int ID)
{
    int file, n = 1, m=0;
    file = open( args[1], O_RDONLY, 0);

    if (file == -1)
    {
        printf("Source file can't be found/read at LOCAL SERVER\n");
        return;
    }
    char buf[BUFSIZE];

    send(ID , args[0] , strlen(args[0]) , 0 );//put

    int valread = read( ID , buf, BUFSIZE);// ack

    FILE *fpointer;
    fpointer = fopen( args[1], "r");
    unsigned long int sz;
    fseek(fpointer, 0L, SEEK_END);
    sz = ftell(fpointer);
    fclose (fpointer);
    sprintf (buf + (BUFSIZE / 2), "%lu" , sz );
    strcpy(buf, args[1]);

    send(ID , buf , BUFSIZE , 0 ); //file name

    valread = read( ID , buf, BUFSIZE);// ok nok
    buf[valread] = '\0';
    if (strcmp(buf, "nok") == 0)
    {
        printf("\n The SERVER says ..\n");
        printf("failed to create desti\n");
        return;
    }
    while (1)
    {
        n = read(file, buf, BUFSIZE);
        if (n <= 0)
            break;
        m = send(ID , buf , n , 0 );
    }
    close(file);
    printf("transmited size is %ld Mb \n", sz/1000);
    valread = read( ID , buf, BUFSIZE);// ack
    return;
}

void getfile(char **args , int ID)
{
    send(ID , args[0] , strlen(args[0]) , 0 );
    int file, n = 1, m;
    char buf[BUFSIZE];
    int valread = read( ID , buf, BUFSIZE);// ack
    send(ID , args[1] , strlen(args[1]) , 0 );//file name

    valread = read( ID , buf, BUFSIZE);// ok nok
    buf[valread] = '\0';
    unsigned long int sz, count = 0;
    if (strcmp(buf, "nok") == 0)
    {
        printf("\nThe SERVER says..\n");
        printf("Source can't be found\n");
        return;
    }
    else
    {
        char *temp = NULL;
        sz = strtol(buf, &temp, 10);
    }
    file = creat(args[1], 0666);
    if (file == -1)
    {
        send(ID , "nok" , strlen("nok") , 0 );
        printf("failed to create desti\n");
        return;
    }
    else
    {
        sprintf (buf, "%lu" , sz );
        send(ID , buf , strlen(buf) , 0 );
        valread = read( ID , buf, BUFSIZE);// ok nok
        if (strcmp(buf, "nok") == 0)
        {
            printf("file size mismatch between YOU and SERVER..."
                   "\nPls contact administrator\n");
            return;
        }
        send(ID , "ok" , strlen("ok") , 0 );
    }
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
    printf("the received size is %ld Mb\n", sz / 1000);
    close(file);
    return;
}

char** parse_line(char *str, int *z)
{
    int parse_max = 25;
    int temp_max = 25;
    char **parsed = (char **)malloc(parse_max * sizeof(char*));
    int n = 0;
    int i = 0;
    int j = 0;
    char *temp = (char *)malloc(temp_max * sizeof(char));
    while (str[i] != '\0')
    {
        if (str[i] != ' ')
        {
            if (j >= temp_max)
            {
                temp_max += 25;
                temp = (char *)realloc(temp, temp_max * sizeof(char));
            }
            temp[j++] = str[i++];
        }
        else
        {
            if (n >= parse_max)
            {
                parse_max += 25;
                parsed = (char**)realloc(parsed , parse_max * sizeof(char*));
            }
            while (str[i] == ' ')
            {
                temp[j] = '\0';
                parsed[n] = temp;
                i++;
            }
            n++;
            j = 0;
            temp = (char *)malloc(temp_max * sizeof(char));
        }
    }
    temp[j] = '\0';
    parsed[n] = temp;
    parsed[n + 1] = NULL;
    if (str[i - 1] != ' ')
        n++;
    *z = n;
    return parsed;
}

char* read_line()
{
    int str_max = 25;
    char *str = malloc(str_max * sizeof(char));
    int index = 0;
    char ch, prev = ' ';
    while (1)
    {
        ch = getchar();
        if (prev == ' ' && ch == ' ')
        {}
        else
        {
            if (ch == '\n' || ch == EOF)
            {
                str[index] = '\0';
                if (str[index - 1] == ' ')
                    str[index] = '\0';
                return str;
            }
            else
            {
                str[index++] = ch;
            }
            if (index >= str_max)
            {
                str_max += 25;
                str = realloc(str, str_max * sizeof(char));
            }
        }
        prev = ch;
    }
}

void freeme(char **args, int n)
{
    int k;
    if (args != NULL)
        for (k = 0; k < n; k++)
        {
            free(args[k]);
        }
    free(args);
    return;
}
int main(int argc, char const *argv[])
{
    struct sockaddr_in serv_addr;
    int ClientSocketNUM = 0, valread, n = 0;
    char *ptr;
    char **args;
    char buffer[1024];
    char transmit[1024];

    if (argc != 3)
    {
        printf("Invalid arguments(give server IPv4 and port)\n");
        exit(0);
    }
    if ((ClientSocketNUM = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address\n");
        return -1;
    }
    if (connect(ClientSocketNUM, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    int stat = 1;
    char name[50];
    printf("Waiting for Connection from Server\n");
    read( ClientSocketNUM , buffer, 1024);//ack
    while (1)
    {
        if (stat == 1)
        {
            printf("Connected to the Server\n");
            printf("Please enter the user name : ");
            scanf("%s", name);
            while ((getchar()) != '\n');
            send(ClientSocketNUM , name , strlen(name) , 0 );
            read( ClientSocketNUM , buffer, 1024);
            stat = 0;
        }
        printf("%sEnter Command $ >> %s", KYEL, KNRM );
        ptr = read_line();
        args = parse_line(ptr , &n);
        if (strcmp(args[0], "ls") == 0)
        {
            if (n != 1)
            {   //4084
                printf("Too many arguments\n");
                continue;
            }
            else
            {
                send(ClientSocketNUM , args[0] , strlen(args[0]) , 0 );
                valread = read( ClientSocketNUM , buffer, 1024);
                buffer[valread] = '\0';
                printf("\nin SERVER, at '%s'\n\n", buffer );
                send(ClientSocketNUM , "." , 1 , 0 );
                while (1)
                {
                    valread = read( ClientSocketNUM , buffer, 1024);
                    buffer[valread] = '\0';
                    if (strcmp(buffer, "end") != 0)
                    {
                        send(ClientSocketNUM , "." , 1 , 0 );
                        printf("%s\n", buffer);
                    }
                    else
                        break;
                }
                printf("\n" );
            }
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            if (n != 2)
            {
                printf("enter the destination directory in SERVER\n");
                continue;
            }
            send(ClientSocketNUM , args[0] , strlen(args[0]) , 0 );
            valread = read( ClientSocketNUM , buffer, 1024);//ack
            send(ClientSocketNUM , args[1] , strlen(args[1]) , 0 );
            valread = read( ClientSocketNUM , buffer, 1024);//true of false
            if (valread >= 0)
                buffer[valread] = '\0';
            if (strcmp(buffer, "false") == 0)
            {
                printf("The server says, 'check path'\n");
            }
            send(ClientSocketNUM , "ack" , strlen("ack") , 0 );
            valread = read( ClientSocketNUM , buffer, 1024);//true of false
            if (valread >= 0)
                buffer[valread] = '\0';
            printf("In SERVER cwd ='%s'\n", buffer);
        }
        else if (strcmp(args[0], "cwd") == 0)
        {
            send(ClientSocketNUM , args[0] , strlen(args[0]) , 0 );
            valread = read( ClientSocketNUM , buffer, 1024);
            if (valread >= 0)
                buffer[valread] = '\0';
            printf("In SERVER cwd ='%s'\n", buffer);
        }
        else if (strcmp(args[0], "lcwd") == 0)
        {
            getcwd(transmit, 1024);
            printf("In LOCAL cwd ='%s'\n", transmit);
        }
        else if (strcmp(args[0], "lls") == 0)
        {
            if (n != 1) { //4084
                printf("Too many arguments\n");
                continue;
            }
            else
                system("ls");
        }
        else if (strcmp(args[0], "lcd") == 0)
        {
            int s = chdir(args[1]);
            if (s != 0)
            {
                printf("improper arguments\n");
            }
            getcwd(transmit, 1024);
            printf("The LOCAL cwd = '%s'\n", transmit );
        }
        else if (strcmp(args[0], "close") == 0)
        {
            send(ClientSocketNUM , args[0] , strlen(args[0]) , 0 );
            exit(0);
        }
        else if (strcmp(args[0], "put") == 0)
        {
            if (n == 2)
            {
                putfile(args , ClientSocketNUM);
            }
            else
                printf("improper arguments. Try 'put [FileName]'\n");
        }
        else if (strcmp(args[0], "get") == 0)
        {
            if (n == 2)
            {
                getfile(args , ClientSocketNUM);
            }
            else
                printf("improper arguments. Try 'get [FileName]'\n\n");
        }
        else if (strcmp(args[0], "chmod") == 0)
        {
            send(ClientSocketNUM , args[0] , strlen(args[0]) , 0 );
            valread = read( ClientSocketNUM , buffer, 1024);//ack
            send(ClientSocketNUM , ptr , strlen(ptr) , 0 );
            valread = read( ClientSocketNUM , buffer, 1024);//ack
        }
        else if (strcmp(args[0], "lchmod") == 0)
        {
            system(ptr + 1);
        }
        else if (strcmp(args[0], "help") == 0)
        {   //4084
            printf("SUPPORTED ARGUMENTS:\n\n");
            printf("1.ls (To View Server directory)\n");
            printf("2.lls (To View this Client's directory)\n");
            printf("3.cd [path] (To change Server directory)\n");
            printf("4.lcd [path] (To change this Client's directory)\n");
            printf("5.chmod [octal-mode] [FileName] (To change file permission on Server)\n");
            printf("6.lchmod [octal-mode] [FileName] (To change file permission on this Client's)\n");
            printf("7.put [FileName] (To upload file to Server directory)\n");
            printf("8.get [FileName] (To download file from Server directory)\n");
            printf("9.close (To disconnet from server)\n");
        }
        else if (strcmp(args[0], "") == 0)
        {
        }
        else
            printf("Invalid Command(Try 'help')\n");
        freeme(args, n);
        free(ptr);
    }
    return 0;
}
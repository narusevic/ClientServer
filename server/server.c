/*
    Server
    Lukas Narusevicius
*/
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
 
void *connection_handler(void *);
char *itoa(int val, int base);
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);

    if (socket_desc == -1)
    {
        printf("Negalima sukurti socket'o");
    }

    puts("Socket'as sukurtas");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);
     
    //Bind
    if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        //print the error message
        perror("Bind'inimas nuluzo");
        return 1;
    }

    puts("PriBindinta");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Laukiama prisijungimu...");
    c = sizeof(struct sockaddr_in);
    
    while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Prisijunta");
         
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
         
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("Negalima sukurti gijos");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        pthread_join(sniffer_thread , NULL);
        puts("Prisijungta prie gijos");
    }
     
    if (client_sock < 0)
    {
        perror("Neprisijungta");
        return 1;
    }
     
    return 0;
}
 
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    int receive_size = 2000;
    char client_message[receive_size];
    char* sum_message;
    int sum = 0;
    char* error_message = "Ivedete 0 arba ne skaiciu";

    //Receive a message from client
    while((read_size = recv(sock, client_message, receive_size, 0)) > 0)
    {
        int number = atoi(client_message);
        memset(client_message,0,strlen(client_message));

        if (number == 0)
        {
            write(sock, error_message, strlen(error_message));
            continue;
        }

        sum += number;
        sum_message = itoa(sum, 10);

        write(sock, sum_message, strlen(sum_message));

        memset(sum_message,0,strlen(sum_message));
    }
     
    if(read_size == 0)
    {
        puts("Klientas atsijunge");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("Klaida atsakyme");
    }
         
    //Free the socket pointer
    close(sock);
    free(socket_desc);
     
    return 0;
}

char* itoa(int val, int base){
    
    static char buf[32] = {0};
    
    int i = 30;
    
    for(; val && i ; --i, val /= base)
    
        buf[i] = "0123456789abcdef"[val % base];
    
    return &buf[i+1];
    
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include<pthread.h>
#include<stdbool.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif
#define PORT 12345

void listenAndPrint(int client_socket)
{
    char buffer[1024];

    while(true)
    {
        ssize_t amountRecieved = recv(client_socket,buffer,1024,0);

        if(amountRecieved>0)
        {
            buffer[amountRecieved] = 0;
            printf("%s\n",buffer);
        }

        if(amountRecieved==0)break;
    }
    close(client_socket);
}

int main()
{

    // Initialize Winsock on Windows
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        perror("[-] WSAStartup failed\n");
        exit(1);
    }
#endif
    char *ip = "10.70.44.0";
    // create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Socket creation failed\n");
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Connection Failed");
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }
    printf("Connected\n");
    pthread_t id;
    pthread_create(&id,NULL,(void *)listenAndPrint,(void *)(intptr_t)client_socket);

    //Reading Console Entry
    char name[1024];
    size_t nameSize = 0;
    printf("Please Enter your Name: \n");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    char line[1024];
    printf("Start chatting or Enter exit for exiting the chat\n");
    char buffer[2048];

    while(true)
    {
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0';
        snprintf(buffer,sizeof(buffer),"%s:%s",name,line);

        if (strcmp(line, "exit") == 0)
            break;
        ssize_t amountWasSent = send(client_socket,buffer,strlen(buffer),0);
        
    }
    // Close socket
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
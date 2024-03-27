#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif
int PORT;


struct AcceptedSocket
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

int acceptedSocketCount = 0;
struct AcceptedSocket acceptedSockets[10];

struct AcceptedSocket *acceptIncomingConnection(int server_sock)
{
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(struct sockaddr_in);
    int clientSocketFD = accept(server_sock, &clientAddress, &clientAddressSize);

    struct AcceptedSocket *acceptedSocket = malloc(sizeof(struct AcceptedSocket));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD > 0;

    if (!acceptedSocket->acceptedSuccessfully)
    {
        acceptedSocket->error = clientSocketFD;
    }
    return acceptedSocket;
}
void sendReceivedMessageToOtherClients(char *buffer, int server_sock)
{
    for (int i = 0; i < acceptedSocketCount; i++)
    {
        if (acceptedSockets[i].acceptedSocketFD != server_sock)
        {
            send(acceptedSockets[i].acceptedSocketFD, buffer, strlen(buffer), 0);
        }
    }
}
void receiveAndPrintIncomingData(int server_sock)
{
    char buffer[1024];
    while (true)
    {
        ssize_t amountRecieved = recv(server_sock, buffer, 1024, 0);

        if (amountRecieved > 0)
        {
            buffer[amountRecieved] = 0;
            printf("%s\n", buffer);
            sendReceivedMessageToOtherClients(buffer, server_sock);
        }
        if (amountRecieved == 0)
            break;

        close(server_sock);
    }
}

int main()
{

    int server_sock, client_socket;
    struct sockaddr_in client_addr;

    // create socket
    struct sockaddr_in server_addr;
    socklen_t addr_size;
    char buffer[] = "Hello";
    int n;
    char *ip = "10.70.44.0";

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        perror("[-] WSAStartup failed\n");
        exit(1);
    }
#endif

    // create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Socket Error\n");
        exit(1);
    }
    printf("TCP server socket is created\n");
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Bind the socket to the specified port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen to the incoming connection
    if (listen(server_sock, 10) == -1)
    {
        perror("Listen Failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Server Listening on port 8080\n");

    // Accept the connection from the client
    while (true)
    {
        struct AcceptedSocket *client_socket = acceptIncomingConnection(server_sock);
        acceptedSockets[acceptedSocketCount++] = *client_socket;

        pthread_t id;
        pthread_create(&id, NULL, (void *)receiveAndPrintIncomingData, client_socket->acceptedSocketFD);
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
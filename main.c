#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>


#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif


#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif


#include <stdio.h>
#include <string.h>

#if defined _WIN32
#include <conio.h>
#endif // defined

#define MAX_LEN_MSG 512
#define MAX_LEN_INIT 50

#define PASSWORDS "y3_ch1p.\n"

#include <ctype.h>

void getline(char *input, int maxlen);

int main() {

#if defined(_WIN32)
    WSADATA d;
    if(WSAStartup(MAKEWORD(2, 2), &d)) {
        fprintf(stderr, "Could not initiate Winsocks\n");
        return(-1);
    }
#endif // defined

    char service[MAX_LEN_INIT];
    printf("Enter Port Number: ");
    getline(service, MAX_LEN_INIT);

    struct addrinfo hints, *peer_address;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags  = AI_PASSIVE;
    if(getaddrinfo(0, service, &hints, &peer_address)) {
        fprintf(stderr, "Failed to get address info. %d\n", GETSOCKETERRNO());
        return(-1);
    }

    char address_buffer[MAX_LEN_INIT], service_buffer[MAX_LEN_INIT];
    if(getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
                   address_buffer, sizeof(address_buffer),
                   service_buffer, sizeof(service_buffer), NI_NUMERICHOST)) {
                    fprintf(stderr, "Failed to Get Name Information %d", GETSOCKETERRNO());
                    return(-1);
    }
    printf("\nAttempting to bind socket at IP: %s at Port/Service: %s\n", address_buffer, service_buffer);

    SOCKET server_sock = socket(peer_address->ai_family, peer_address->ai_socktype, 0);
    if(!ISVALIDSOCKET(server_sock)) {
        fprintf(stderr,"Failed to Create Socket (%d)\n", GETSOCKETERRNO());
        return(-1);
    }

    if(bind(server_sock, peer_address->ai_addr, peer_address->ai_addrlen)) {
        fprintf(stderr, "Failed to Bind the Socket to %s at port %s (%d)\n", address_buffer, service_buffer, GETSOCKETERRNO());
        return(-1);
    }

    if(listen(server_sock, 3) < 0) {
        fprintf(stderr, "Failed to initiate Listen (%d\n", GETSOCKETERRNO());
        return(-1);
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(server_sock, &master);
    SOCKET max_socket = server_sock;

    printf("Waiting for connections...\n");


    while(1) {
        fd_set reads;
        reads = master;
        if (select(max_socket+1, &reads, 0, 0, 0) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        SOCKET i;
        for(i = 1; i <= max_socket; ++i) {
            if (FD_ISSET(i, &reads)) {

                if (i == server_sock) {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    SOCKET socket_client = accept(server_sock,
                            (struct sockaddr*) &client_address,
                            &client_len);
                    if (!ISVALIDSOCKET(socket_client)) {
                        fprintf(stderr, "accept() failed. (%d)\n",
                                GETSOCKETERRNO());
                        return 1;
                    }

                    //Handling of Password
                    char request_password[MAX_LEN_INIT] = "Please Enter a Password: ";
                    size_t length = strlen(request_password);
                    char password[MAX_LEN_INIT] = PASSWORDS;
                    char pass_init[MAX_LEN_INIT];
                    if(send(socket_client, request_password, length, 0) == -1) {
                        fprintf(stderr, "Password Request Error (%d)\n",GETSOCKETERRNO());
                        return(-1);
                    }
                    if(recv(socket_client, pass_init, MAX_LEN_INIT, 0) == -1) {
                        fprintf(stderr, "Password Request Error (%d)\n", GETSOCKETERRNO());
                        return(-1);
                    }
                    if(strcmp(pass_init, password)) {
                        printf("Failed attempt to connect. Password Not Valid. Closing connection\n");
                    }
                    //End handling of Password

                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket)
                        max_socket = socket_client;

                    char address_buffer[100];
                    getnameinfo((struct sockaddr*)&client_address,
                            client_len,
                            address_buffer, sizeof(address_buffer), 0, 0,
                            NI_NUMERICHOST);
                    printf("New connection from %s\n", address_buffer);

                } else {
                    char read[1024];
                    int bytes_received = recv(i, read, 1024, 0);
                    if (bytes_received < 1) {
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }

                    SOCKET j;
                    for (j = 1; j <= max_socket; ++j) {
                        if (FD_ISSET(j, &master)) {
                            if (j == server_sock || j == i)
                                continue;
                            else
                                send(j, read, bytes_received, 0);
                        }
                    }
                }
            } //if FD_ISSET
        } //for i to max_socket
    } //while(1)



    printf("Closing listening socket...\n");
    CLOSESOCKET(server_sock);

#if defined(_WIN32)
    WSACleanup();
#endif


    printf("Finished.\n");

    return 0;
}

void getline(char *input, int maxlen) {
    int c, i;

    for(i = 0; (c = getchar()) != '\n' && i < maxlen; i++)
        input[i] = c;

    input[i] = '\0';

}


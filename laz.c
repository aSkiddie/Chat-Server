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

void getline(char *input, int maxlen);

int main(void) {

#if defined(_WIN32)
    WSADATA d;
    if(WSAStartup(MAKEWORD(2,2), &d)) {
        fprintf(stderr,"WinSocks Initiation Failure");
        return(-1);
    }
#endif // defined

    char input[MAX_LEN_INIT], service[MAX_LEN_INIT];
    printf("Enter an IP Address or Domain Name i.e. www.google.com: ");
    getline(input, MAX_LEN_INIT);
    printf("Enter in a service or port number i.e. http or 80: ");
    getline(service, MAX_LEN_INIT);

    struct addrinfo hints, *peer_address;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    if(getaddrinfo(input, service, &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo failed. (%d)\n", GETSOCKETERRNO());
        return(-1);
    }

    char addressbuffer[MAX_LEN_INIT], service_buffer[MAX_LEN_INIT];
    if(getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
       addressbuffer, sizeof(addressbuffer), service_buffer, sizeof(service_buffer),
                   NI_NUMERICHOST)) {
                    fprintf(stderr,"getnameinfo failed. (%d)\n", GETSOCKETERRNO());
                    return(-1);
        }
    printf("Attempting to connect to %s %s\n",addressbuffer, service_buffer);
    SOCKET socket_descriptor = socket(peer_address->ai_family, peer_address->ai_socktype, 0);
    if(!ISVALIDSOCKET(socket_descriptor)) {
        fprintf(stderr, "Socket Initialization has failed. (%d)\n",GETSOCKETERRNO());
        return(-1);
    }
    if(connect(socket_descriptor, peer_address->ai_addr, peer_address->ai_addrlen)){
        fprintf(stderr,"connect failed. (%d)\n", GETSOCKETERRNO());
        return(-1);
    }
    printf("Connection Established. You are now connected to %s using %s\n", addressbuffer, service_buffer);

    while(1) {







    }




    return 0;
}

void getline(char *input, int maxlen) {
    int c, i;

    for(i = 0; (c = getchar()) != '\n' && i < maxlen; i++)
        input[i] = c;

    input[i] = '\0';

}


// Roll: 24105142049

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define CLOSE closesocket
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int socket_t;
#define CLOSE close
#endif

#define PORT 30049

int main(int argc, char *argv[])
{
    socket_t s;
    struct sockaddr_in server;
    socklen_t len = sizeof(server);
    char input[1024], msg[1100], res[1100];
    int n;

#ifdef _WIN32
    WSADATA w;
    WSAStartup(MAKEWORD(2,2), &w);
#endif

    if (argc != 3) {
        printf("Usage: %s <IP> <roll>\n", argv[0]);
        return 1;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, argv[1], &server.sin_addr);

    while (1) {
        printf("Enter operation (SUM/MAX/MIN/AVG) or EXIT: ");

        if (!fgets(input, sizeof(input), stdin))
            strcpy(input, "EXIT");

        input[strcspn(input, "\r\n")] = '\0';

        snprintf(msg, sizeof(msg), "%s#%s", argv[2], input);

        sendto(s, msg, strlen(msg), 0,
               (struct sockaddr *)&server, len);

        n = recvfrom(s, res, sizeof(res)-1, 0,
                     (struct sockaddr *)&server, &len);

        if (n < 0) break;

        res[n] = '\0';
        printf("Server response: %s\n", res);

        if (strcmp(input, "EXIT") == 0) break;
    }

    CLOSE(s);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}

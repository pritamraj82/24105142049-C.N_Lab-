
// Roll: 24105142049 Service Type: B d = 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#define CLOSE closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define CLOSE close
#endif

#define PORT 30049
#define ROLL "24105142049"

int main()
{
    int s, n, a[100], i, count;
    long long sum;
    double avg;
    char buf[4096], res[100], *op, *p;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

#ifdef _WIN32
    WSADATA w;
    WSAStartup(MAKEWORD(2,2), &w);
#endif

    s = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Bind failed\n");
        CLOSE(s);
        return 1;
    }

    printf("Server running on port %d\n", PORT);

    while (1) {
        n = recvfrom(s, buf, sizeof(buf)-1, 0,
                     (struct sockaddr *)&client, &len);

        if (n < 0) continue;
        buf[n] = '\0';

        printf("\nClient: %s:%u\n",
               inet_ntoa(client.sin_addr),
               (unsigned int)ntohs(client.sin_port));

        if (strncmp(buf, ROLL "#", strlen(ROLL)+1) != 0) {
            strcpy(res, "ERROR: Unauthorized client");
        }
        else {
            p = buf + strlen(ROLL) + 1;
            printf("Received: %s\n", p);

            if (strcmp(p, "EXIT") == 0) {
                strcpy(res, "Server shutting down");
                sendto(s, res, strlen(res), 0,
                       (struct sockaddr *)&client, len);
                printf("Sent: %s\n", res);
                break;
            }

            op = strtok(p, "|");
            char *num = strtok(NULL, "|");
            char *values = strtok(NULL, "|");

            if (!op || !num || !values) {
                strcpy(res, "ERROR: Invalid format");
            }
            else {
                count = atoi(num);
                if (count < 1 || count > 100) {
                    strcpy(res, "ERROR: Invalid number of integers");
                }
                else {
                    char *token = strtok(values, " ");
                    i = 0;

                    while (token && i < count) {
                        a[i++] = atoi(token);
                        token = strtok(NULL, " ");
                    }

                    if (i != count || token != NULL) {
                        strcpy(res, "ERROR: Invalid integer list");
                    }
                    else {
                        sum = 0;
                        int max = a[0], min = a[0];

                        for (i = 0; i < count; i++) {
                            sum += a[i];
                            if (a[i] > max) max = a[i];
                            if (a[i] < min) min = a[i];
                        }

                        avg = (double)sum / count;

                        if (strcmp(op, "SUM") == 0)
                            sprintf(res, "%lld", sum);
                        else if (strcmp(op, "MAX") == 0)
                            sprintf(res, "%d", max);
                        else if (strcmp(op, "MIN") == 0)
                            sprintf(res, "%d", min);
                        else if (strcmp(op, "AVG") == 0)
                            sprintf(res, "%.2f", avg);
                        else
                            strcpy(res, "ERROR: Invalid operation");
                    }
                }
            }
        }

        printf("Sent response: %s\n", res);

        sendto(s, res, strlen(res), 0,
               (struct sockaddr *)&client, len);
    }

    CLOSE(s);

#ifdef _WIN32
    WSACleanup();
#endif

    printf("Server closed.\n");
    return 0;
}

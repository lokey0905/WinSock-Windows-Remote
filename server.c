#define _CRT_SECURE_NO_WARNINGS
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFSIZE 1024*1024
#define MAX_CLIENTS 100
#define PORT 7

WSADATA wsa;
int sockfd, new_fd;
int client_sockets[MAX_CLIENTS];
int num_clients = 0;
struct sockaddr_in my_addr;
struct sockaddr_in client_addr;
char buf[BUFSIZE];
char buf2[BUFSIZE];
int numbytes;
int sin_size;
int retVal;
int showChoice = 0;

void _recv(void* param) {
    int index = (int)param;
    int new_fd = client_sockets[index];

    while (1)
    {
        ZeroMemory(buf2, BUFSIZE);
        int retVal = recv(new_fd, buf2, BUFSIZE, 0);
        if (retVal <= 0) {
            break;
        }
        else {
            if(showChoice == index)
				printf("%s", buf2);
            /*buf2[retVal] = '\0';
            printf("%s", buf2);*/
        }
    }
}

void _accept(void* param) {
    while (1)
    {
        new_fd = accept(sockfd, (struct sockaddr*)&client_addr, &sin_size);
        if (new_fd < 0) {
            printf("\naccept failed\n");
            closesocket(sockfd);
            WSACleanup();
            return -1;
        }

        if (num_clients < MAX_CLIENTS) {
            client_sockets[num_clients] = new_fd;
            _beginthread(_recv, 0, (void*)num_clients);
            num_clients++;
            printf("\nClient connected: %d\n", num_clients);
        }
        else {
            printf("\nMaximum clients reached. Connection rejected.\n");
            closesocket(new_fd);
        }
    }
}

int main()
{
    while (1) {
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        {
            printf("WSAStartup failed\n");
            return -1;
        }

        sockfd = socket(AF_INET, SOCK_STREAM, 0); //server等待連線的socket
        if (sockfd == -1)
        {
            printf("socket error\n");
            WSACleanup();
            return -1;
        }
        my_addr.sin_family = AF_INET; //IPv4
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY); //接受任何IP的連線
        my_addr.sin_port = htons((short)PORT); //port 
        sin_size = sizeof(my_addr);

        retVal = bind(sockfd, (struct sockaddr*)&my_addr, sin_size); //將socket與特定的IP及port綁定
        if (retVal < 0) {
            printf("binding failed\n");
            closesocket(sockfd);
            WSACleanup();
            return -1;
        }

        retVal = listen(sockfd, 10); //開始監聽是否有client連線
        if (retVal < 0) {
            printf("listening failed\n");
            closesocket(sockfd);
            WSACleanup();
            return -1;
        }

        printf("Server starts\n");
        _beginthread(_accept, 0, "accept");

        while (1) {
            ZeroMemory(buf, BUFSIZE);
            fgets(buf, BUFSIZE, stdin);
            buf[strlen(buf) - 1] = '\0';
            if (buf[0] == '/') {
                if(strcmp(buf, "/show") == 0) {
					printf("Choose a client to show message: ");
					scanf("%d", &showChoice);
                    showChoice--;
                    if(showChoice < 0 || showChoice >= num_clients) {
						printf("Invalid client\n");
						showChoice = 0;
					}
					getchar();
				}
				else if(strcmp(buf, "/exit") == 0) {
					closesocket(sockfd);
					WSACleanup();
					return 0;
				}
				else {
					printf("Invalid command\n");
				}
            }
            else {
                for (int i = 0; i < num_clients; i++) {
                    retVal = send(client_sockets[i], buf, strlen(buf), 0);
                    if (SOCKET_ERROR == retVal)
                    {
                        printf("send failed!\n");
                        closesocket(sockfd);
                        WSACleanup();
                        return -1;
                    }
                }
            }
        }

        printf("Client disconnected\n");
        closesocket(sockfd);
        closesocket(new_fd);
        WSACleanup();
        printf("\n---------------------\n");
    }

    return 0;
}

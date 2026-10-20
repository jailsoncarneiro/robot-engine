#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

static pthread_t server_thread;
static volatile int running = 1;
static volatile RobotCommand current_cmd = CMD_IDLE;
static volatile unsigned long packet_counter = 0;

static void *udp_listener_worker(void *arg)
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[256];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("[IPC Server] Erro socket UDP");
        return NULL;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(UDP_PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("[IPC Server] Erro bind UDP");
        close(sockfd);
        return NULL;
    }

    printf("[IPC Server] Escutando porta %d...\n", UDP_PORT);

    while (running)
    {
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, (char *)buffer, sizeof(buffer) - 1,
                         MSG_DONTWAIT, (struct sockaddr *)&cliaddr, &len);

        if (n > 0)
        {
            buffer[n] = '\0';
            if (buffer[n - 1] == '\n')
                buffer[n - 1] = '\0';

            packet_counter++; // Incrementa ID único do comando

            printf("[IPC Server] [#%lu] Comando recebido: %s\n", packet_counter, buffer);

            if (strcmp(buffer, "TAKEOFF") == 0)
            {
                current_cmd = CMD_TAKEOFF;
            }
            else if (strcmp(buffer, "HOVER") == 0)
            {
                current_cmd = CMD_HOVER;
            }
            else if (strcmp(buffer, "BACKFLIP") == 0)
            {
                current_cmd = CMD_BACKFLIP;
            }
            else if (strcmp(buffer, "TURTLE") == 0)
            {
                current_cmd = CMD_TURTLE;
            }
            else if (strcmp(buffer, "KILL") == 0 || strcmp(buffer, "SHIT") == 0)
            {
                current_cmd = CMD_KILL; // Derruba o bicho no chão!
            }
            else if (strcmp(buffer, "LAND") == 0)
            {
                current_cmd = CMD_LAND;
            }
        }
        usleep(5000);
    }

    close(sockfd);
    return NULL;
}

int init_command_server(void)
{
    running = 1;
    return pthread_create(&server_thread, NULL, udp_listener_worker, NULL);
}

RobotCommand get_current_command(void)
{
    return current_cmd;
}

unsigned long get_packet_id(void)
{
    return packet_counter;
}

void stop_command_server(void)
{
    running = 0;
    pthread_join(server_thread, NULL);
}
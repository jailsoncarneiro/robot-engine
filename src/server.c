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

static void *udp_listener_worker(void *arg)
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[256];

    // 1. Cria socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("[IPC Server] Erro ao criar socket UDP");
        return NULL;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(UDP_PORT);

    // 2. Faz o bind na porta 9002
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("[IPC Server] Erro no bind UDP");
        close(sockfd);
        return NULL;
    }

    printf("[IPC Server] Escutando comandos UDP na porta %d...\n", UDP_PORT);

    // 3. Loop de escuta de comandos
    while (running)
    {
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, (char *)buffer, sizeof(buffer) - 1,
                         MSG_DONTWAIT, (struct sockaddr *)&cliaddr, &len);

        if (n > 0)
        {
            buffer[n] = '\0';

            // Remove quebra de linha se houver
            if (buffer[n - 1] == '\n')
                buffer[n - 1] = '\0';

            printf("[IPC Server] Comando recebido: %s\n", buffer);

            // Parser dos comandos
            if (strcmp(buffer, "THRUST") == 0)
            {
                current_cmd = CMD_THRUST;
            }
            else if (strcmp(buffer, "CROUCH") == 0)
            {
                current_cmd = CMD_CROUCH;
            }
            else if (strcmp(buffer, "RELAX") == 0)
            {
                current_cmd = CMD_RELAX;
            }
        }
        usleep(5000); // 5ms sleep para não fritar a CPU esperando pacote
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

void stop_command_server(void)
{
    running = 0;
    pthread_join(server_thread, NULL);
    printf("[IPC Server] Servidor finalizado.\n");
}
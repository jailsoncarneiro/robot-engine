#ifndef SERVER_H
#define SERVER_H

#define UDP_PORT 9002

typedef enum
{
    CMD_IDLE = 0,
    CMD_TAKEOFF,  // Decolar e pairar
    CMD_HOVER,    // Trava o drone no ar no lugar!
    CMD_BACKFLIP, // Fazer a manobra de giro acrobático 360
    CMD_TURTLE,   // Desvira o drone no chão
    CMD_KILL,     // Corta tudo no ar e deixa cair de cabeça para baixo
    CMD_LAND,     // Pousar / Desligar
    // Comandos legados para bípedes/robôs
    CMD_THRUST,
    CMD_CROUCH,
    CMD_RELAX
} RobotCommand;

int init_command_server(void);
RobotCommand get_current_command(void);
unsigned long get_packet_id(void);
void stop_command_server(void);

#endif
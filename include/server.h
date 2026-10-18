#ifndef SERVER_H
#define SERVER_H

#define UDP_PORT 9002

// Comandos possíveis suportados pelo motor
typedef enum
{
    CMD_IDLE = 0,
    CMD_THRUST, // Força total nos motores
    CMD_CROUCH, // Flexiona juntas
    CMD_RELAX   // Desliga torque (boneco de pano)
} RobotCommand;

// Inicializa a thread do servidor UDP
int init_command_server(void);

// Retorna o comando atual recebido da rede (thread-safe simples)
RobotCommand get_current_command(void);

// Para a thread do servidor no encerramento
void stop_command_server(void);

#endif
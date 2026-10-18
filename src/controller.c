#include "controller.h"
#include "server.h"
#include <stdio.h>

void init_controller(const mjModel *m)
{
    mjcb_control = my_combo_controller;
    printf("[Combo Engine] Controlador registrado com sucesso!\n");
}

void my_combo_controller(const mjModel *m, mjData *d)
{
    RobotCommand cmd = get_current_command();
    int max_actuators = (m->nu > 0 && m->nu <= 100) ? m->nu : 21;

    switch (cmd)
    {
    case CMD_THRUST:
        // Dispara todos os atuadores com torque positivo forte
        for (int i = 0; i < max_actuators; i++)
        {
            d->ctrl[i] = 120.0;
        }
        break;

    case CMD_CROUCH:
        // Flexiona atuadores (torque negativo suave)
        for (int i = 0; i < max_actuators; i++)
        {
            d->ctrl[i] = -40.0;
        }
        break;

    case CMD_RELAX:
    case CMD_IDLE:
    default:
        // Relaxa os motores (ragdoll)
        for (int i = 0; i < max_actuators; i++)
        {
            d->ctrl[i] = 0.0;
        }
        break;
    }
}
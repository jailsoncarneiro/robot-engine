#include "controller.h"
#include <stdio.h>

void init_controller(const mjModel *m)
{
    mjcb_control = my_combo_controller;
    printf("[Combo Engine] Controlador registrado com sucesso!\n");
}

void my_combo_controller(const mjModel *m, mjData *d)
{
    // Aplica o combo só entre 0.5s e 1.2s
    if (d->time >= 0.5 && d->time <= 1.2)
    {
        // O humanoid tem exatamente 17 atuadores.
        // Limitamos para nunca estourar o buffer:
        int max_actuators = (m->nu > 0 && m->nu <= 100) ? m->nu : 17;

        for (int i = 0; i < max_actuators; i++)
        {
            d->ctrl[i] = 100.0; // Força máxima
        }
    }
}
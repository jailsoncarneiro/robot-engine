#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <mujoco/mujoco.h>

// Registra nosso callback no loop do MuJoCo
void init_controller(const mjModel *m);

// Função que calcula a força dos motores a cada milissegundo
void my_combo_controller(const mjModel *m, mjData *d);

#endif
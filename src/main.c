#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mujoco/mujoco.h>
#include <GLFW/glfw3.h>
#include "controller.h"
#include "server.h"

// Modelos do MuJoCo
mjModel *m = NULL;
mjData *d = NULL;

// Estruturas de visualização
mjvCamera cam;
mjvOption opt;
mjvScene scn;
mjrContext con;

int main(int argc, const char **argv)
{
    char error[1000] = "Não foi possível carregar o modelo";

    // 1. Carrega modelo e dados
    m = mj_loadXML("models/humanoid.xml", NULL, error, 1000);
    if (!m)
    {
        printf("Erro ao carregar modelo: %s\n", error);
        return 1;
    }
    d = mj_makeData(m);

    // 2. Registra o seu controlador de combos em C!
    init_controller(m);

    // 3. Inicializa GLFW
    if (!glfwInit())
    {
        printf("Falha ao iniciar GLFW\n");
        return 1;
    }

    GLFWwindow *window = glfwCreateWindow(1200, 900, "Robot Engine - MVP", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        printf("Falha ao criar janela\n");
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // 4. Inicializa visualizador
    mjv_defaultCamera(&cam);
    mjv_defaultOption(&opt);
    mjv_defaultScene(&scn);
    mjr_defaultContext(&con);

    mjv_makeScene(m, &scn, 2000);
    mjr_makeContext(m, &con, mjFONTSCALE_150);

    // Posiciona a câmera para enquadrar o boneco
    cam.type = mjCAMERA_FREE;
    cam.distance = 4.0;
    cam.elevation = -20.0;
    cam.azimuth = 90.0;
    cam.lookat[2] = 1.0;

    printf("Motor iniciado com sucesso!\n");
    printf("Juntas: %ld | Atuadores: %ld\n", (long)m->nv, (long)m->nu);

    // Inicia o servidor de comandos IPC via UDP
    init_command_server();

    // 5. Loop principal estável
    while (!glfwWindowShouldClose(window))
    {
        mjtNum simstart = d->time;

        // Loop de física sincronizado a 60Hz
        while (d->time - simstart < 1.0 / 60.0)
        {
            mj_step(m, d); // Vai invocar seu controller automaticamente!
        }

        // Obtém dimensões do framebuffer
        mjrRect viewport = {0, 0, 0, 0};
        glfwGetFramebufferSize(window, &viewport.width, &viewport.height);

        // Renderiza
        mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
        mjr_render(viewport, &scn, &con);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Libera recursos
    mjv_freeScene(&scn);
    mjr_freeContext(&con);
    mj_deleteData(d);
    mj_deleteModel(m);
    glfwTerminate();

    stop_command_server();

    return 0;
}
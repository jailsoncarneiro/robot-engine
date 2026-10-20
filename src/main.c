#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mujoco/mujoco.h>
#include <GLFW/glfw3.h>
#include "controller.h"
#include "server.h"
#include "telemetry.h"

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
    const char *model_path = (argc > 1) ? argv[1] : "models/drone.xml";
    printf("Carregando modelo: %s\n", model_path);
    m = mj_loadXML(model_path, NULL, error, 1000);
    if (!m)
    {
        printf("Erro ao carregar modelo: %s\n", error);
        return 1;
    }
    d = mj_makeData(m);

    // Configura o modo de telemetria via argumento opcional
    TelemetryMode telem_mode = TELEM_EVENTS; // Padrão agora é EVENTS (leve!)
    if (argc > 2)
    {
        if (strcmp(argv[2], "--raw") == 0)
            telem_mode = TELEM_RAW;
        else if (strcmp(argv[2], "--off") == 0)
            telem_mode = TELEM_OFF;
    }

    init_telemetry("flight_log.csv", telem_mode);

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
            log_telemetry_step(m, d);
        }

        // Obtém dimensões do framebuffer
        mjrRect viewport = {0, 0, 0, 0};
        glfwGetFramebufferSize(window, &viewport.width, &viewport.height);

        // Câmera persegue a posição atual do robô no espaço
        cam.lookat[0] = d->qpos[0];
        cam.lookat[1] = d->qpos[1];
        cam.lookat[2] = d->qpos[2];

        // FEEDBACK VISUAL DOS MOTORES: Muda a cor conforme o empuxo!
        // Se m->nu == 4 (Drone), atualiza o brilho dos motores
        if (m->nu == 4)
        {
            for (int i = 0; i < 4; i++)
            {
                // geom 2, 3, 4, 5 são os 4 motores no XML
                int geom_id = 2 + i;
                float power = (float)(d->ctrl[i] / 10.0); // 0.0 a 1.0
                if (power > 1.0f)
                    power = 1.0f;
                if (power < 0.0f)
                    power = 0.0f;

                // Motor apagado = cinza escuro. Motor ligado = cor viva brilhante!
                m->geom_rgba[geom_id * 4 + 0] = 0.2f + (power * 0.8f); // Red
                m->geom_rgba[geom_id * 4 + 1] = 0.2f + (power * 0.8f); // Green
                m->geom_rgba[geom_id * 4 + 3] = 1.0f;                  // Alpha
            }
        }

        mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
        glViewport(0, 0, viewport.width, viewport.height);
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

    close_telemetry();

    return 0;
}
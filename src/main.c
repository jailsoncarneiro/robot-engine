#include <stdio.h>
#include <stdlib.h>
#include <mujoco/mujoco.h>
#include <GLFW/glfw3.h>

// Modelos do MuJoCo
mjModel *m = NULL; // Dados estáticos (geometria, massa)
mjData *d = NULL;  // Dados dinâmicos (posição, velocidade, forças)

// Estruturas do visualizador
mjvCamera cam;  // Câmera virtual
mjvOption opt;  // Opções de renderização
mjvScene scn;   // Cena gráfica
mjrContext con; // Contexto GPU OpenGL

// Controle de janela GLFW
GLFWwindow *window = NULL;

int main(int argc, const char **argv)
{
    char error[1000] = "Não foi possível carregar o modelo";

    // 1. Carrega o modelo XML compilado para binário em memória
    m = mj_loadXML("models/humanoid.xml", NULL, error, 1000);
    if (!m)
    {
        printf("Erro ao carregar o modelo: %s\n", error);
        return 1;
    }

    // 2. Aloca os dados dinâmicos da física
    d = mj_makeData(m);

    // 3. Inicializa GLFW e cria a janela gráfica
    if (!glfwInit())
    {
        printf("Erro ao inicializar GLFW\n");
        return 1;
    }

    window = glfwCreateWindow(1200, 900, "Robot Engine - MVP", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // 4. Inicializa o renderizador interno do MuJoCo
    mjv_defaultCamera(&cam);
    mjv_defaultOption(&opt);
    mjv_defaultScene(&scn);
    mjr_defaultContext(&con);

    mjv_makeScene(m, &scn, 2000);
    mjr_makeContext(m, &con, mjFONTSCALE_150);

    printf("Motor de física iniciado com sucesso!\n");
    printf("Número de juntas (graus de liberdade): %ld\n", (long)m->nv);
    printf("Número de atuadores/motores: %ld\n", (long)m->nu);

    // 5. O LOOP PRINCIPAL DA FÍSICA (Aqui nascem os combos)
    while (!glfwWindowShouldClose(window))
    {
        mjtNum simstart = d->time;

        // Avança a física a 60 FPS (fazendo micro-passos de 1ms do integrador)
        while (d->time - simstart < 1.0 / 60.0)
        {

            // --- AQUI A GENTE INJETA AS FORÇAS DOS COMBOS NO FUTURO ---
            // Exemplo: se quisesse aplicar torque no motor 0:
            // d->ctrl[0] = 50.0;

            mj_step(m, d); // O coração da física roda aqui!
        }

        // Renderização gráfica
        mjrRect viewport = {0, 0, 0, 0};
        glfwGetFramebufferSize(window, &viewport.width, &viewport.height);

        mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
        mjr_render(viewport, &scn, &con);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpeza de memória
    mjv_freeScene(&scn);
    mjr_freeContext(&con);
    mj_deleteData(d);
    mj_deleteModel(m);
    glfwTerminate();

    return 0;
}
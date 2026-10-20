#define _USE_MATH_DEFINES
#include "telemetry.h"
#include "server.h" // Para ler o Packet ID atual
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static FILE *log_file = NULL;
static TelemetryMode current_mode = TELEM_OFF;
static double last_log_time = 0.0;
static char active_event_name[64] = "IDLE";

void init_telemetry(const char *filename, TelemetryMode mode)
{
    current_mode = mode;
    if (current_mode == TELEM_OFF)
        return;

    log_file = fopen(filename, "w");
    if (!log_file)
        return;

    // Cabeçalho unificado com a coluna de evento/Packet ID
    fprintf(log_file, "time,pos_z,vel_z,pitch_deg,roll_deg,m0,m1,m2,m3,event_context\n");
    printf("[Telemetry] Modo RAW Unificado ativo em: %s\n", filename);
    fflush(log_file);
}

void log_telemetry_step(const mjModel *m, const mjData *d)
{
    if (!log_file || current_mode == TELEM_OFF)
        return;

    // Grava a cada 10ms (100 Hz)
    if (d->time - last_log_time >= 0.01)
    {
        last_log_time = d->time;

        const mjtNum *mat = &d->xmat[9]; // Corpo 1 = drone
        double pitch_deg = asin(-mat[2]) * (180.0 / M_PI);
        double roll_deg = atan2(mat[5], mat[8]) * (180.0 / M_PI);

        // Grava os dados de física + o evento que estava ativo no momento
        fprintf(log_file, "%.3f,%.3f,%.3f,%.1f,%.1f,%.2f,%.2f,%.2f,%.2f,%s\n",
                d->time, d->qpos[2], d->qvel[2], pitch_deg, roll_deg,
                d->ctrl[0], d->ctrl[1], d->ctrl[2], d->ctrl[3], active_event_name);

        static int flush_count = 0;
        if (++flush_count % 50 == 0)
            fflush(log_file);
    }
}

void log_telemetry_event(double sim_time, const char *event_name, const char *details)
{
    // Guarda o nome do evento atual para o RAW injetar continuamente
    snprintf(active_event_name, sizeof(active_event_name), "%s_%s", event_name, details);

    if (log_file && current_mode != TELEM_OFF)
    {
        fprintf(log_file, "%.3f,0,0,0,0,0,0,0,0,EVENT_TRIGGER:%s_%s\n", sim_time, event_name, details);
        fflush(log_file);
    }
}

void close_telemetry(void)
{
    if (log_file)
    {
        fflush(log_file);
        fclose(log_file);
        log_file = NULL;
        printf("[Telemetry] Caixa preta salva com sucesso!\n");
    }
}
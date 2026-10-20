#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <mujoco/mujoco.h>

typedef enum
{
    TELEM_OFF = 0,
    TELEM_EVENTS, // Só grava quando muda de estado/comando
    TELEM_RAW     // Grava a 100Hz contínuo
} TelemetryMode;

void init_telemetry(const char *filename, TelemetryMode mode);
void log_telemetry_step(const mjModel *m, const mjData *d);
void log_telemetry_event(double sim_time, const char *event_name, const char *details);
void close_telemetry(void);

#endif
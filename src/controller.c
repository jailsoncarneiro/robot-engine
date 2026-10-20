#include "controller.h"
#include "server.h"
#include "telemetry.h"
#include <stdio.h>
#include <math.h>

static double flip_timer = -1.0;
static double turtle_timer = -1.0;
static int disarmed = 0;
static unsigned long last_packet_id = 0;
static double target_z = 1.6;

void init_controller(const mjModel *m)
{
    mjcb_control = my_combo_controller;
    printf("[Combo Engine] Flight Controller Simplificado e Robusto Ativo!\n");
}

void my_combo_controller(const mjModel *m, mjData *d)
{
    RobotCommand cmd = get_current_command();
    unsigned long pkt_id = get_packet_id();

    // Sincroniza logs com o Packet ID exato
    if (pkt_id != last_packet_id)
    {
        char details[128];
        snprintf(details, sizeof(details), "Pkt#%lu, X=%.2f, Y=%.2f, Z=%.2fm", pkt_id, d->qpos[0], d->qpos[1], d->qpos[2]);

        if (cmd == CMD_TAKEOFF)
        {
            target_z = 1.6;
            log_telemetry_event(d->time, "TAKEOFF", details);
        }
        else if (cmd == CMD_HOVER)
        {
            target_z = d->qpos[2]; // Trava na altura atual
            if (target_z < 0.2)
                target_z = 1.0;
            log_telemetry_event(d->time, "HOVER", details);
        }
        else if (cmd == CMD_BACKFLIP)
        {
            flip_timer = -1.0; // Reseta o timer do flip
            log_telemetry_event(d->time, "BACKFLIP", details);
        }
        else if (cmd == CMD_TURTLE)
        {
            turtle_timer = -1.0; // Reseta o timer do turtle
            log_telemetry_event(d->time, "TURTLE", details);
        }
        else if (cmd == CMD_KILL)
        {
            log_telemetry_event(d->time, "KILL_SWITCH", details);
        }
        else if (cmd == CMD_LAND)
        {
            log_telemetry_event(d->time, "LAND", details);
        }
        last_packet_id = pkt_id;
    }

    if (m->nu == 4)
    {
        double current_z = d->qpos[2];
        double velocity_z = d->qvel[2];
        double rate_roll = d->qvel[3];
        double z_up = d->xmat[1 * 9 + 8];

        if (cmd == CMD_KILL || (current_z < 0.12 && z_up < 0.6 && cmd != CMD_TURTLE))
        {
            disarmed = 1;
        }

        if (disarmed && cmd != CMD_TURTLE && cmd != CMD_TAKEOFF)
        {
            for (int i = 0; i < 4; i++)
                d->ctrl[i] = 0.0;
            return;
        }

        switch (cmd)
        {
        case CMD_TAKEOFF:
        case CMD_HOVER:
        {
            turtle_timer = -1.0;
            disarmed = 0;

            double hover_thrust = 2.45;
            double error = target_z - current_z;
            double base_thrust = hover_thrust + (error * 4.0) - (velocity_z * 2.0);

            if (base_thrust < 0)
                base_thrust = 0;
            if (base_thrust > 8.0)
                base_thrust = 8.0;

            double gyro = rate_roll * 0.2;
            d->ctrl[0] = base_thrust - gyro;
            d->ctrl[1] = base_thrust - gyro;
            d->ctrl[2] = base_thrust + gyro;
            d->ctrl[3] = base_thrust + gyro;
            break;
        }

        case CMD_BACKFLIP:
        {
            if (current_z < 1.0)
            {
                d->ctrl[0] = 2.45;
                d->ctrl[1] = 2.45;
                d->ctrl[2] = 2.45;
                d->ctrl[3] = 2.45;
                break;
            }

            if (flip_timer < 0)
                flip_timer = d->time;
            double elapsed = d->time - flip_timer;

            if (elapsed < 0.08)
            {
                // 1. Soco forte para cima para ganhar altura instantânea e não afundar
                for (int i = 0; i < 4; i++)
                    d->ctrl[i] = 11.0;
            }
            else if (elapsed < 0.22)
            {
                // 2. Giro brutal e rápido (20.0N na frente para garantir que o bico suba rápido!)
                d->ctrl[0] = 20.0;
                d->ctrl[1] = 20.0;
                d->ctrl[2] = 0.0;
                d->ctrl[3] = 0.0;
            }
            else if (elapsed < 0.35)
            {
                // 3. Freio pesado para estancar o giro cravado
                d->ctrl[0] = 0.0;
                d->ctrl[1] = 0.0;
                d->ctrl[2] = 20.0;
                d->ctrl[3] = 20.0;
            }
            else
            {
                // 4. Hover de Recuperação agressivo
                target_z = 1.6;
                double hover_thrust = 2.45;
                double error = target_z - current_z;
                double thrust = hover_thrust + (error * 5.0) - (velocity_z * 3.0);
                if (thrust < 0.5)
                    thrust = 0.5;
                if (thrust > 9.5)
                    thrust = 9.5;

                double gyro = rate_roll * 0.2;
                d->ctrl[0] = thrust - gyro;
                d->ctrl[1] = thrust - gyro;
                d->ctrl[2] = thrust + gyro;
                d->ctrl[3] = thrust + gyro;
            }
            break;
        }

        case CMD_TURTLE:
        {
            disarmed = 0;
            if (turtle_timer < 0)
                turtle_timer = d->time;
            double elapsed = d->time - turtle_timer;

            // Dá a alavanca lateral por 300ms se estiver no chão invertido
            if (current_z < 0.20 && z_up < 0.5 && elapsed < 0.30)
            {
                d->ctrl[0] = -14.0;
                d->ctrl[2] = 18.0;
                d->ctrl[1] = 0.0;
                d->ctrl[3] = 0.0;
            }
            else
            {
                for (int i = 0; i < 4; i++)
                    d->ctrl[i] = 0.0;
            }
            break;
        }

        case CMD_KILL:
        {
            disarmed = 1;
            // Corta tudo imediatamente com um leve empurrão assimétrico para capotar
            static double kill_t = -1.0;
            if (kill_t < 0)
                kill_t = d->time;
            if (d->time - kill_t < 0.05)
            {
                d->ctrl[0] = 8.0;
                d->ctrl[1] = 0.0;
                d->ctrl[2] = 0.0;
                d->ctrl[3] = 0.0;
            }
            else
            {
                for (int i = 0; i < 4; i++)
                    d->ctrl[i] = 0.0;
            }
            break;
        }

        case CMD_LAND:
        default:
            flip_timer = -1.0;
            turtle_timer = -1.0;
            disarmed = 1;
            for (int i = 0; i < 4; i++)
                d->ctrl[i] = 0.0;
            break;
        }
    }
}
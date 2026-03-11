#include "wcx_pid.h"

void wcx_pid_init(wcx_pid_t *pid,
                  float kp,
                  float ki,
                  float kd,
                  float output_min,
                  float output_max)
{
    if (pid == NULL)
    {
        return;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->output_min = output_min;
    pid->output_max = output_max;
    pid->integrator = 0.0f;
    pid->previous_measurement = 0.0f;
    pid->initialized = false;
}

void wcx_pid_reset(wcx_pid_t *pid, float measurement)
{
    if (pid == NULL)
    {
        return;
    }

    pid->integrator = 0.0f;
    pid->previous_measurement = measurement;
    pid->initialized = true;
}

float wcx_pid_compute(wcx_pid_t *pid, float setpoint, float measurement, float dt_seconds)
{
    float error;
    float derivative = 0.0f;
    float proportional;
    float output;

    if (pid == NULL || dt_seconds <= 0.0f)
    {
        return 0.0f;
    }

    if (!pid->initialized)
    {
        wcx_pid_reset(pid, measurement);
    }

    error = setpoint - measurement;
    proportional = pid->kp * error;

    pid->integrator += pid->ki * error * dt_seconds;
    pid->integrator = WCX_CLAMP(pid->integrator, pid->output_min, pid->output_max);

    derivative = -(measurement - pid->previous_measurement) / dt_seconds;
    pid->previous_measurement = measurement;

    output = proportional + pid->integrator + (pid->kd * derivative);
    return WCX_CLAMP(output, pid->output_min, pid->output_max);
}

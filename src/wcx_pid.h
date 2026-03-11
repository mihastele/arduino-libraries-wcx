#ifndef WCX_PID_H
#define WCX_PID_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        float kp;
        float ki;
        float kd;
        float output_min;
        float output_max;
        float integrator;
        float previous_measurement;
        bool initialized;
    } wcx_pid_t;

    void wcx_pid_init(wcx_pid_t *pid,
                      float kp,
                      float ki,
                      float kd,
                      float output_min,
                      float output_max);
    void wcx_pid_reset(wcx_pid_t *pid, float measurement);
    float wcx_pid_compute(wcx_pid_t *pid, float setpoint, float measurement, float dt_seconds);

#ifdef __cplusplus
}
#endif

#endif

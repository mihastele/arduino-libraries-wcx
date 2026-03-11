#ifndef WCX_RATE_LIMITER_H
#define WCX_RATE_LIMITER_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Slew-rate limiter.
     * Constrains how fast an output value can change per update step.
     * Useful for smoothing setpoint changes, motor ramps, and LED fading.
     */
    typedef struct
    {
        float value;
        float max_rise; /* maximum positive change per step */
        float max_fall; /* maximum negative change per step (positive number) */
        bool primed;
    } wcx_rate_limiter_t;

    void wcx_rate_limiter_init(wcx_rate_limiter_t *rl,
                               float max_rise, float max_fall);
    void wcx_rate_limiter_reset(wcx_rate_limiter_t *rl);
    float wcx_rate_limiter_update(wcx_rate_limiter_t *rl, float target);
    float wcx_rate_limiter_value(const wcx_rate_limiter_t *rl);

    /**
     * Integer rate limiter — no floating point.
     */
    typedef struct
    {
        int32_t value;
        int32_t max_rise;
        int32_t max_fall;
        bool primed;
    } wcx_irate_limiter_t;

    void wcx_irate_limiter_init(wcx_irate_limiter_t *rl,
                                int32_t max_rise, int32_t max_fall);
    void wcx_irate_limiter_reset(wcx_irate_limiter_t *rl);
    int32_t wcx_irate_limiter_update(wcx_irate_limiter_t *rl, int32_t target);
    int32_t wcx_irate_limiter_value(const wcx_irate_limiter_t *rl);

#ifdef __cplusplus
}
#endif

#endif /* WCX_RATE_LIMITER_H */

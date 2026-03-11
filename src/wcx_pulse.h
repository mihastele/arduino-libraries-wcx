#ifndef WCX_PULSE_H
#define WCX_PULSE_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Pulse counter and frequency meter.
     * Feed it the current digital state each loop iteration; it counts
     * rising edges and computes period / frequency from the timestamps.
     */
    typedef struct
    {
        uint32_t count;
        uint32_t last_edge_ms;
        uint32_t period_ms;
        bool last_state;
        bool primed;
    } wcx_pulse_t;

    void wcx_pulse_init(wcx_pulse_t *p);
    void wcx_pulse_reset(wcx_pulse_t *p);
    void wcx_pulse_update(wcx_pulse_t *p, bool state, uint32_t now_ms);
    uint32_t wcx_pulse_count(const wcx_pulse_t *p);
    uint32_t wcx_pulse_period_ms(const wcx_pulse_t *p);
    float wcx_pulse_frequency_hz(const wcx_pulse_t *p);

#ifdef __cplusplus
}
#endif

#endif /* WCX_PULSE_H */

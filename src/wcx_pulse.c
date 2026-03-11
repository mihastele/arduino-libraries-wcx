#include "wcx_pulse.h"

void wcx_pulse_init(wcx_pulse_t *p)
{
    if (p == NULL)
    {
        return;
    }
    p->count = 0U;
    p->last_edge_ms = 0U;
    p->period_ms = 0U;
    p->last_state = false;
    p->primed = false;
}

void wcx_pulse_reset(wcx_pulse_t *p)
{
    wcx_pulse_init(p);
}

void wcx_pulse_update(wcx_pulse_t *p, bool state, uint32_t now_ms)
{
    if (p == NULL)
    {
        return;
    }
    /* Detect rising edge */
    if (state && !p->last_state)
    {
        p->count++;
        if (p->primed)
        {
            p->period_ms = now_ms - p->last_edge_ms;
        }
        p->last_edge_ms = now_ms;
        p->primed = true;
    }
    p->last_state = state;
}

uint32_t wcx_pulse_count(const wcx_pulse_t *p)
{
    if (p == NULL)
    {
        return 0U;
    }
    return p->count;
}

uint32_t wcx_pulse_period_ms(const wcx_pulse_t *p)
{
    if (p == NULL)
    {
        return 0U;
    }
    return p->period_ms;
}

float wcx_pulse_frequency_hz(const wcx_pulse_t *p)
{
    if (p == NULL || p->period_ms == 0U)
    {
        return 0.0f;
    }
    return 1000.0f / (float)p->period_ms;
}

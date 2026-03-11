#include "wcx_rate_limiter.h"

/* ---- Float rate limiter ---- */

void wcx_rate_limiter_init(wcx_rate_limiter_t *rl,
                           float max_rise, float max_fall)
{
    if (rl == NULL)
    {
        return;
    }
    rl->value = 0.0f;
    rl->max_rise = max_rise;
    rl->max_fall = max_fall;
    rl->primed = false;
}

void wcx_rate_limiter_reset(wcx_rate_limiter_t *rl)
{
    if (rl == NULL)
    {
        return;
    }
    rl->value = 0.0f;
    rl->primed = false;
}

float wcx_rate_limiter_update(wcx_rate_limiter_t *rl, float target)
{
    if (rl == NULL)
    {
        return 0.0f;
    }
    if (!rl->primed)
    {
        rl->value = target;
        rl->primed = true;
        return rl->value;
    }
    float delta = target - rl->value;
    if (delta > rl->max_rise)
    {
        delta = rl->max_rise;
    }
    else if (delta < -rl->max_fall)
    {
        delta = -rl->max_fall;
    }
    rl->value += delta;
    return rl->value;
}

float wcx_rate_limiter_value(const wcx_rate_limiter_t *rl)
{
    if (rl == NULL)
    {
        return 0.0f;
    }
    return rl->value;
}

/* ---- Integer rate limiter ---- */

void wcx_irate_limiter_init(wcx_irate_limiter_t *rl,
                            int32_t max_rise, int32_t max_fall)
{
    if (rl == NULL)
    {
        return;
    }
    rl->value = 0;
    rl->max_rise = max_rise;
    rl->max_fall = max_fall;
    rl->primed = false;
}

void wcx_irate_limiter_reset(wcx_irate_limiter_t *rl)
{
    if (rl == NULL)
    {
        return;
    }
    rl->value = 0;
    rl->primed = false;
}

int32_t wcx_irate_limiter_update(wcx_irate_limiter_t *rl, int32_t target)
{
    if (rl == NULL)
    {
        return 0;
    }
    if (!rl->primed)
    {
        rl->value = target;
        rl->primed = true;
        return rl->value;
    }
    int32_t delta = target - rl->value;
    if (delta > rl->max_rise)
    {
        delta = rl->max_rise;
    }
    else if (delta < -rl->max_fall)
    {
        delta = -rl->max_fall;
    }
    rl->value += delta;
    return rl->value;
}

int32_t wcx_irate_limiter_value(const wcx_irate_limiter_t *rl)
{
    if (rl == NULL)
    {
        return 0;
    }
    return rl->value;
}

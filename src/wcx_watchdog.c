#include "wcx_watchdog.h"

void wcx_watchdog_init(wcx_watchdog_t *wd, uint32_t timeout_ms,
                       wcx_watchdog_callback_t on_expire, void *context)
{
    if (wd == NULL)
    {
        return;
    }
    wd->timeout_ms = timeout_ms;
    wd->last_kick = 0U;
    wd->expired = false;
    wd->started = false;
    wd->on_expire = on_expire;
    wd->context = context;
}

void wcx_watchdog_start(wcx_watchdog_t *wd, uint32_t now_ms)
{
    if (wd == NULL)
    {
        return;
    }
    wd->last_kick = now_ms;
    wd->expired = false;
    wd->started = true;
}

void wcx_watchdog_kick(wcx_watchdog_t *wd, uint32_t now_ms)
{
    if (wd == NULL)
    {
        return;
    }
    wd->last_kick = now_ms;
    wd->expired = false;
}

bool wcx_watchdog_check(wcx_watchdog_t *wd, uint32_t now_ms)
{
    if (wd == NULL)
    {
        return false;
    }
    if (!wd->started)
    {
        return false;
    }
    if (wd->expired)
    {
        return true;
    }
    if (wcx_time_reached(wd->last_kick, wd->timeout_ms, now_ms))
    {
        wd->expired = true;
        if (wd->on_expire != NULL)
        {
            wd->on_expire(wd->context);
        }
        return true;
    }
    return false;
}

bool wcx_watchdog_is_expired(const wcx_watchdog_t *wd)
{
    if (wd == NULL)
    {
        return false;
    }
    return wd->expired;
}

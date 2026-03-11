#include "wcx_timer.h"

void wcx_timer_init(wcx_timer_t *timer, uint32_t interval_ms, bool periodic)
{
    if (timer == NULL)
    {
        return;
    }

    timer->interval_ms = interval_ms;
    timer->deadline_ms = 0U;
    timer->running = false;
    timer->periodic = periodic;
}

void wcx_timer_set_interval(wcx_timer_t *timer, uint32_t interval_ms)
{
    if (timer == NULL)
    {
        return;
    }

    timer->interval_ms = interval_ms;
}

void wcx_timer_start(wcx_timer_t *timer, uint32_t now_ms)
{
    if (timer == NULL)
    {
        return;
    }

    timer->deadline_ms = now_ms + timer->interval_ms;
    timer->running = true;
}

void wcx_timer_start_at(wcx_timer_t *timer, uint32_t deadline_ms)
{
    if (timer == NULL)
    {
        return;
    }

    timer->deadline_ms = deadline_ms;
    timer->running = true;
}

void wcx_timer_stop(wcx_timer_t *timer)
{
    if (timer == NULL)
    {
        return;
    }

    timer->running = false;
}

bool wcx_timer_running(const wcx_timer_t *timer)
{
    return timer != NULL && timer->running;
}

uint32_t wcx_timer_remaining(const wcx_timer_t *timer, uint32_t now_ms)
{
    if (timer == NULL || !timer->running || wcx_time_reached(now_ms, timer->deadline_ms))
    {
        return 0U;
    }

    return timer->deadline_ms - now_ms;
}

bool wcx_timer_expired(wcx_timer_t *timer, uint32_t now_ms)
{
    if (timer == NULL || !timer->running || !wcx_time_reached(now_ms, timer->deadline_ms))
    {
        return false;
    }

    if (timer->periodic)
    {
        timer->deadline_ms += timer->interval_ms;
    }
    else
    {
        timer->running = false;
    }

    return true;
}
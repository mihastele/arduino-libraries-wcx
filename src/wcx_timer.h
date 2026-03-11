#ifndef WCX_TIMER_H
#define WCX_TIMER_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint32_t interval_ms;
        uint32_t deadline_ms;
        bool running;
        bool periodic;
    } wcx_timer_t;

    void wcx_timer_init(wcx_timer_t *timer, uint32_t interval_ms, bool periodic);
    void wcx_timer_set_interval(wcx_timer_t *timer, uint32_t interval_ms);
    void wcx_timer_start(wcx_timer_t *timer, uint32_t now_ms);
    void wcx_timer_start_at(wcx_timer_t *timer, uint32_t deadline_ms);
    void wcx_timer_stop(wcx_timer_t *timer);
    bool wcx_timer_running(const wcx_timer_t *timer);
    uint32_t wcx_timer_remaining(const wcx_timer_t *timer, uint32_t now_ms);
    bool wcx_timer_expired(wcx_timer_t *timer, uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif
#ifndef WCX_WATCHDOG_H
#define WCX_WATCHDOG_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Software watchdog / heartbeat monitor.
     * Tracks whether a subsystem has checked in within its deadline.
     * Call wcx_watchdog_kick() periodically to signal liveness;
     * call wcx_watchdog_check() to see if the deadline has expired.
     */
    typedef void (*wcx_watchdog_callback_t)(void *context);

    typedef struct
    {
        uint32_t timeout_ms;
        uint32_t last_kick;
        bool expired;
        bool started;
        wcx_watchdog_callback_t on_expire;
        void *context;
    } wcx_watchdog_t;

    void wcx_watchdog_init(wcx_watchdog_t *wd, uint32_t timeout_ms,
                           wcx_watchdog_callback_t on_expire, void *context);
    void wcx_watchdog_start(wcx_watchdog_t *wd, uint32_t now_ms);
    void wcx_watchdog_kick(wcx_watchdog_t *wd, uint32_t now_ms);
    bool wcx_watchdog_check(wcx_watchdog_t *wd, uint32_t now_ms);
    bool wcx_watchdog_is_expired(const wcx_watchdog_t *wd);

#ifdef __cplusplus
}
#endif

#endif /* WCX_WATCHDOG_H */

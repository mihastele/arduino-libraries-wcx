#ifndef WCX_SCHEDULER_H
#define WCX_SCHEDULER_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*wcx_task_callback_t)(void *context);

    typedef struct
    {
        uint32_t period_ms;
        uint32_t next_run_ms;
        bool enabled;
        bool repeat;
        wcx_task_callback_t callback;
        void *context;
    } wcx_task_t;

    typedef struct
    {
        wcx_task_t *tasks;
        size_t task_count;
    } wcx_scheduler_t;

    void wcx_task_init(wcx_task_t *task,
                       uint32_t start_at_ms,
                       uint32_t period_ms,
                       bool repeat,
                       wcx_task_callback_t callback,
                       void *context);
    void wcx_task_enable(wcx_task_t *task, bool enabled);
    void wcx_task_schedule_from_now(wcx_task_t *task, uint32_t now_ms);
    bool wcx_task_run(wcx_task_t *task, uint32_t now_ms);

    void wcx_scheduler_init(wcx_scheduler_t *scheduler, wcx_task_t *tasks, size_t task_count);
    size_t wcx_scheduler_tick(wcx_scheduler_t *scheduler, uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif

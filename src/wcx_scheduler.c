#include "wcx_scheduler.h"

void wcx_task_init(wcx_task_t *task,
                   uint32_t start_at_ms,
                   uint32_t period_ms,
                   bool repeat,
                   wcx_task_callback_t callback,
                   void *context)
{
    if (task == NULL)
    {
        return;
    }

    task->period_ms = period_ms;
    task->next_run_ms = start_at_ms;
    task->enabled = true;
    task->repeat = repeat;
    task->callback = callback;
    task->context = context;
}

void wcx_task_enable(wcx_task_t *task, bool enabled)
{
    if (task == NULL)
    {
        return;
    }

    task->enabled = enabled;
}

void wcx_task_schedule_from_now(wcx_task_t *task, uint32_t now_ms)
{
    if (task == NULL)
    {
        return;
    }

    task->next_run_ms = now_ms + task->period_ms;
}

bool wcx_task_run(wcx_task_t *task, uint32_t now_ms)
{
    if (task == NULL || !task->enabled || task->callback == NULL)
    {
        return false;
    }

    if (!wcx_time_reached(now_ms, task->next_run_ms))
    {
        return false;
    }

    task->callback(task->context);

    if (task->repeat)
    {
        task->next_run_ms += task->period_ms;
    }
    else
    {
        task->enabled = false;
    }

    return true;
}

void wcx_scheduler_init(wcx_scheduler_t *scheduler, wcx_task_t *tasks, size_t task_count)
{
    if (scheduler == NULL)
    {
        return;
    }

    scheduler->tasks = tasks;
    scheduler->task_count = task_count;
}

size_t wcx_scheduler_tick(wcx_scheduler_t *scheduler, uint32_t now_ms)
{
    size_t ran = 0U;
    size_t index;

    if (scheduler == NULL || scheduler->tasks == NULL)
    {
        return 0U;
    }

    for (index = 0U; index < scheduler->task_count; ++index)
    {
        if (wcx_task_run(&scheduler->tasks[index], now_ms))
        {
            ran++;
        }
    }

    return ran;
}

#include "wcx_filter.h"

bool wcx_moving_average_init(wcx_moving_average_t *filter, float *storage, size_t capacity)
{
    size_t index;

    if (filter == NULL || storage == NULL || capacity == 0U)
    {
        return false;
    }

    filter->samples = storage;
    filter->capacity = capacity;
    filter->count = 0U;
    filter->index = 0U;
    filter->sum = 0.0f;

    for (index = 0U; index < capacity; ++index)
    {
        filter->samples[index] = 0.0f;
    }

    return true;
}

void wcx_moving_average_reset(wcx_moving_average_t *filter, float seed_value)
{
    size_t index;

    if (filter == NULL || filter->samples == NULL)
    {
        return;
    }

    for (index = 0U; index < filter->capacity; ++index)
    {
        filter->samples[index] = seed_value;
    }

    filter->count = filter->capacity;
    filter->index = 0U;
    filter->sum = seed_value * (float)filter->capacity;
}

float wcx_moving_average_update(wcx_moving_average_t *filter, float sample)
{
    if (filter == NULL || filter->samples == NULL || filter->capacity == 0U)
    {
        return 0.0f;
    }

    if (filter->count < filter->capacity)
    {
        filter->count++;
    }
    else
    {
        filter->sum -= filter->samples[filter->index];
    }

    filter->samples[filter->index] = sample;
    filter->sum += sample;
    filter->index = (filter->index + 1U) % filter->capacity;
    return wcx_moving_average_value(filter);
}

float wcx_moving_average_value(const wcx_moving_average_t *filter)
{
    if (filter == NULL || filter->count == 0U)
    {
        return 0.0f;
    }

    return filter->sum / (float)filter->count;
}

void wcx_ema_init(wcx_ema_filter_t *filter, float alpha, float initial_value, bool initialized)
{
    if (filter == NULL)
    {
        return;
    }

    filter->alpha = WCX_CLAMP(alpha, 0.0f, 1.0f);
    filter->state = initial_value;
    filter->initialized = initialized;
}

float wcx_ema_update(wcx_ema_filter_t *filter, float sample)
{
    if (filter == NULL)
    {
        return sample;
    }

    if (!filter->initialized)
    {
        filter->state = sample;
        filter->initialized = true;
        return filter->state;
    }

    filter->state += filter->alpha * (sample - filter->state);
    return filter->state;
}

float wcx_ema_value(const wcx_ema_filter_t *filter)
{
    return filter == NULL ? 0.0f : filter->state;
}

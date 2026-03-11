#include "wcx_stats.h"

void wcx_stats_init(wcx_stats_t *stats)
{
    wcx_stats_reset(stats);
}

void wcx_stats_reset(wcx_stats_t *stats)
{
    if (stats == NULL)
    {
        return;
    }

    stats->count = 0U;
    stats->minimum = 0.0f;
    stats->maximum = 0.0f;
    stats->sum = 0.0f;
    stats->sum_of_squares = 0.0f;
}

void wcx_stats_update(wcx_stats_t *stats, float sample)
{
    if (stats == NULL)
    {
        return;
    }

    if (stats->count == 0U)
    {
        stats->minimum = sample;
        stats->maximum = sample;
    }
    else
    {
        if (sample < stats->minimum)
        {
            stats->minimum = sample;
        }

        if (sample > stats->maximum)
        {
            stats->maximum = sample;
        }
    }

    stats->count++;
    stats->sum += sample;
    stats->sum_of_squares += sample * sample;
}

uint32_t wcx_stats_count(const wcx_stats_t *stats)
{
    return stats == NULL ? 0U : stats->count;
}

float wcx_stats_min(const wcx_stats_t *stats)
{
    return stats == NULL ? 0.0f : stats->minimum;
}

float wcx_stats_max(const wcx_stats_t *stats)
{
    return stats == NULL ? 0.0f : stats->maximum;
}

float wcx_stats_mean(const wcx_stats_t *stats)
{
    if (stats == NULL || stats->count == 0U)
    {
        return 0.0f;
    }

    return stats->sum / (float)stats->count;
}

float wcx_stats_variance(const wcx_stats_t *stats)
{
    float mean;

    if (stats == NULL || stats->count < 2U)
    {
        return 0.0f;
    }

    mean = wcx_stats_mean(stats);
    return (stats->sum_of_squares / (float)stats->count) - (mean * mean);
}

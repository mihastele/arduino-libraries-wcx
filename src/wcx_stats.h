#ifndef WCX_STATS_H
#define WCX_STATS_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint32_t count;
        float minimum;
        float maximum;
        float sum;
        float sum_of_squares;
    } wcx_stats_t;

    void wcx_stats_init(wcx_stats_t *stats);
    void wcx_stats_reset(wcx_stats_t *stats);
    void wcx_stats_update(wcx_stats_t *stats, float sample);
    uint32_t wcx_stats_count(const wcx_stats_t *stats);
    float wcx_stats_min(const wcx_stats_t *stats);
    float wcx_stats_max(const wcx_stats_t *stats);
    float wcx_stats_mean(const wcx_stats_t *stats);
    float wcx_stats_variance(const wcx_stats_t *stats);

#ifdef __cplusplus
}
#endif

#endif

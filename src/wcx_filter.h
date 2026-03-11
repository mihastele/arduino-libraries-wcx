#ifndef WCX_FILTER_H
#define WCX_FILTER_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        float *samples;
        size_t capacity;
        size_t count;
        size_t index;
        float sum;
    } wcx_moving_average_t;

    typedef struct
    {
        float alpha;
        float state;
        bool initialized;
    } wcx_ema_filter_t;

    bool wcx_moving_average_init(wcx_moving_average_t *filter, float *storage, size_t capacity);
    void wcx_moving_average_reset(wcx_moving_average_t *filter, float seed_value);
    float wcx_moving_average_update(wcx_moving_average_t *filter, float sample);
    float wcx_moving_average_value(const wcx_moving_average_t *filter);

    void wcx_ema_init(wcx_ema_filter_t *filter, float alpha, float initial_value, bool initialized);
    float wcx_ema_update(wcx_ema_filter_t *filter, float sample);
    float wcx_ema_value(const wcx_ema_filter_t *filter);

#ifdef __cplusplus
}
#endif

#endif

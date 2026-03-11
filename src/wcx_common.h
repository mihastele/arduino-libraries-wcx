#ifndef WCX_COMMON_H
#define WCX_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define WCX_ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define WCX_MIN(a, b) ((a) < (b) ? (a) : (b))
#define WCX_MAX(a, b) ((a) > (b) ? (a) : (b))
#define WCX_CLAMP(value, low, high) \
    ((value) < (low) ? (low) : ((value) > (high) ? (high) : (value)))

    bool wcx_time_reached(uint32_t now_ms, uint32_t deadline_ms);
    float wcx_mapf(float value,
                   float in_min,
                   float in_max,
                   float out_min,
                   float out_max);
    float wcx_apply_deadband(float value, float threshold);

#ifdef __cplusplus
}
#endif

#endif

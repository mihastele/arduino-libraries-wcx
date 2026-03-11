#include "wcx_common.h"

bool wcx_time_reached(uint32_t now_ms, uint32_t deadline_ms)
{
    return (int32_t)(now_ms - deadline_ms) >= 0;
}

float wcx_mapf(float value,
               float in_min,
               float in_max,
               float out_min,
               float out_max)
{
    if (in_max == in_min)
    {
        return out_min;
    }

    return ((value - in_min) * (out_max - out_min) / (in_max - in_min)) + out_min;
}

float wcx_apply_deadband(float value, float threshold)
{
    if (threshold < 0.0f)
    {
        threshold = -threshold;
    }

    if (value > -threshold && value < threshold)
    {
        return 0.0f;
    }

    return value;
}

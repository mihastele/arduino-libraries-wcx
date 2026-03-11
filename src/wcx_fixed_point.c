#include "wcx_fixed_point.h"

#include <limits.h>
#include <math.h>

static wcx_q16_16_t wcx_q16_16_saturate(int64_t value)
{
    if (value > (int64_t)INT32_MAX)
    {
        return INT32_MAX;
    }

    if (value < (int64_t)INT32_MIN)
    {
        return INT32_MIN;
    }

    return (wcx_q16_16_t)value;
}

wcx_q16_16_t wcx_q16_16_from_int(int32_t value)
{
    return wcx_q16_16_saturate((int64_t)value << 16);
}

wcx_q16_16_t wcx_q16_16_from_float(float value)
{
    double scaled = round((double)value * 65536.0);
    return wcx_q16_16_saturate((int64_t)scaled);
}

int32_t wcx_q16_16_to_int(wcx_q16_16_t value)
{
    return (int32_t)(value / WCX_Q16_16_ONE);
}

float wcx_q16_16_to_float(wcx_q16_16_t value)
{
    return (float)value / 65536.0f;
}

wcx_q16_16_t wcx_q16_16_mul(wcx_q16_16_t left, wcx_q16_16_t right)
{
    return wcx_q16_16_saturate(((int64_t)left * (int64_t)right) >> 16);
}

wcx_q16_16_t wcx_q16_16_div(wcx_q16_16_t numerator, wcx_q16_16_t denominator)
{
    if (denominator == 0)
    {
        if (numerator > 0)
        {
            return INT32_MAX;
        }

        if (numerator < 0)
        {
            return INT32_MIN;
        }

        return 0;
    }

    return wcx_q16_16_saturate(((int64_t)numerator << 16) / denominator);
}

wcx_q16_16_t wcx_q16_16_clamp(wcx_q16_16_t value,
                              wcx_q16_16_t minimum,
                              wcx_q16_16_t maximum)
{
    return WCX_CLAMP(value, minimum, maximum);
}
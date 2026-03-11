#ifndef WCX_FIXED_POINT_H
#define WCX_FIXED_POINT_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int32_t wcx_q16_16_t;

#define WCX_Q16_16_ONE ((wcx_q16_16_t)0x00010000L)

    wcx_q16_16_t wcx_q16_16_from_int(int32_t value);
    wcx_q16_16_t wcx_q16_16_from_float(float value);
    int32_t wcx_q16_16_to_int(wcx_q16_16_t value);
    float wcx_q16_16_to_float(wcx_q16_16_t value);
    wcx_q16_16_t wcx_q16_16_mul(wcx_q16_16_t left, wcx_q16_16_t right);
    wcx_q16_16_t wcx_q16_16_div(wcx_q16_16_t numerator, wcx_q16_16_t denominator);
    wcx_q16_16_t wcx_q16_16_clamp(wcx_q16_16_t value,
                                  wcx_q16_16_t minimum,
                                  wcx_q16_16_t maximum);

#ifdef __cplusplus
}
#endif

#endif
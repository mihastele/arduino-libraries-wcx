#ifndef WCX_IFILTER_H
#define WCX_IFILTER_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Integer moving average filter.  Stores int32_t samples in a
     * caller-owned ring buffer.  No floating-point operations.
     */
    typedef struct
    {
        int32_t *storage;
        size_t capacity;
        size_t count;
        size_t index;
        int32_t sum;
    } wcx_imoving_average_t;

    /**
     * Integer exponential moving average (EMA).
     * Uses a shift-based weighting:  out = out + (sample - out) >> shift
     * Equivalent to alpha = 1 / (1 << shift).
     */
    typedef struct
    {
        int32_t value;
        uint8_t shift;
        bool primed;
    } wcx_iema_t;

    /* Integer moving average API */
    bool wcx_imoving_average_init(wcx_imoving_average_t *f,
                                  int32_t *storage, size_t capacity);
    void wcx_imoving_average_reset(wcx_imoving_average_t *f);
    int32_t wcx_imoving_average_update(wcx_imoving_average_t *f, int32_t sample);
    int32_t wcx_imoving_average_value(const wcx_imoving_average_t *f);
    size_t wcx_imoving_average_count(const wcx_imoving_average_t *f);

    /* Integer EMA API */
    void wcx_iema_init(wcx_iema_t *f, uint8_t shift);
    void wcx_iema_reset(wcx_iema_t *f);
    int32_t wcx_iema_update(wcx_iema_t *f, int32_t sample);
    int32_t wcx_iema_value(const wcx_iema_t *f);

#ifdef __cplusplus
}
#endif

#endif /* WCX_IFILTER_H */

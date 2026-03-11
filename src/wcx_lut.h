#ifndef WCX_LUT_H
#define WCX_LUT_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * A single (x, y) pair in a lookup table.
     */
    typedef struct
    {
        float x;
        float y;
    } wcx_lut_point_t;

    /**
     * Lookup table backed by a caller-owned array of points sorted by x.
     * Supports linear interpolation between entries and flat extrapolation
     * outside the table bounds.
     */
    typedef struct
    {
        const wcx_lut_point_t *points;
        size_t count;
    } wcx_lut_t;

    /**
     * Integer lookup table entry.
     */
    typedef struct
    {
        int32_t x;
        int32_t y;
    } wcx_lut_ipoint_t;

    /**
     * Integer lookup table (all computations in int32_t).
     */
    typedef struct
    {
        const wcx_lut_ipoint_t *points;
        size_t count;
    } wcx_ilut_t;

    /* Float lookup table API */
    bool wcx_lut_init(wcx_lut_t *lut, const wcx_lut_point_t *points,
                      size_t count);
    float wcx_lut_lookup(const wcx_lut_t *lut, float x);

    /* Integer lookup table API */
    bool wcx_ilut_init(wcx_ilut_t *lut, const wcx_lut_ipoint_t *points,
                       size_t count);
    int32_t wcx_ilut_lookup(const wcx_ilut_t *lut, int32_t x);

#ifdef __cplusplus
}
#endif

#endif /* WCX_LUT_H */

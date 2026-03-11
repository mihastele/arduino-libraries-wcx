#include "wcx_lut.h"

bool wcx_lut_init(wcx_lut_t *lut, const wcx_lut_point_t *points,
                  size_t count)
{
    if (lut == NULL || points == NULL || count == 0U)
    {
        return false;
    }
    lut->points = points;
    lut->count = count;
    return true;
}

float wcx_lut_lookup(const wcx_lut_t *lut, float x)
{
    if (lut == NULL || lut->points == NULL || lut->count == 0U)
    {
        return 0.0f;
    }
    /* Clamp to table bounds */
    if (x <= lut->points[0].x)
    {
        return lut->points[0].y;
    }
    if (x >= lut->points[lut->count - 1U].x)
    {
        return lut->points[lut->count - 1U].y;
    }
    /* Binary search for the surrounding interval */
    size_t lo = 0U;
    size_t hi = lut->count - 1U;
    while (hi - lo > 1U)
    {
        size_t mid = lo + (hi - lo) / 2U;
        if (x < lut->points[mid].x)
        {
            hi = mid;
        }
        else
        {
            lo = mid;
        }
    }
    /* Linear interpolation */
    float x0 = lut->points[lo].x;
    float y0 = lut->points[lo].y;
    float x1 = lut->points[hi].x;
    float y1 = lut->points[hi].y;
    float dx = x1 - x0;
    if (dx == 0.0f)
    {
        return y0;
    }
    return y0 + (y1 - y0) * ((x - x0) / dx);
}

/* ---- Integer lookup table ---- */

bool wcx_ilut_init(wcx_ilut_t *lut, const wcx_lut_ipoint_t *points,
                   size_t count)
{
    if (lut == NULL || points == NULL || count == 0U)
    {
        return false;
    }
    lut->points = points;
    lut->count = count;
    return true;
}

int32_t wcx_ilut_lookup(const wcx_ilut_t *lut, int32_t x)
{
    if (lut == NULL || lut->points == NULL || lut->count == 0U)
    {
        return 0;
    }
    if (x <= lut->points[0].x)
    {
        return lut->points[0].y;
    }
    if (x >= lut->points[lut->count - 1U].x)
    {
        return lut->points[lut->count - 1U].y;
    }
    size_t lo = 0U;
    size_t hi = lut->count - 1U;
    while (hi - lo > 1U)
    {
        size_t mid = lo + (hi - lo) / 2U;
        if (x < lut->points[mid].x)
        {
            hi = mid;
        }
        else
        {
            lo = mid;
        }
    }
    int32_t x0 = lut->points[lo].x;
    int32_t y0 = lut->points[lo].y;
    int32_t x1 = lut->points[hi].x;
    int32_t y1 = lut->points[hi].y;
    int32_t dx = x1 - x0;
    if (dx == 0)
    {
        return y0;
    }
    /* Integer interpolation: y0 + (y1-y0)*(x-x0)/dx with rounding */
    int32_t dy = y1 - y0;
    int32_t num = dy * (x - x0);
    int32_t half = (dx > 0) ? (dx / 2) : -(dx / 2);
    return y0 + (num + ((num >= 0) ? half : -half)) / dx;
}

#include "wcx_hysteresis.h"

void wcx_hysteresis_init(wcx_hysteresis_t *h, float threshold_low,
                         float threshold_high, bool initial_state)
{
    if (h == NULL)
    {
        return;
    }
    h->threshold_low = threshold_low;
    h->threshold_high = threshold_high;
    h->state = initial_state;
}

bool wcx_hysteresis_update(wcx_hysteresis_t *h, float value)
{
    if (h == NULL)
    {
        return false;
    }
    if (h->state)
    {
        if (value < h->threshold_low)
        {
            h->state = false;
        }
    }
    else
    {
        if (value > h->threshold_high)
        {
            h->state = true;
        }
    }
    return h->state;
}

bool wcx_hysteresis_state(const wcx_hysteresis_t *h)
{
    if (h == NULL)
    {
        return false;
    }
    return h->state;
}

/* ---- Threshold detector ---- */

void wcx_threshold_detector_init(wcx_threshold_detector_t *det,
                                 float low_threshold,
                                 float high_threshold,
                                 float critical_threshold,
                                 float hysteresis)
{
    if (det == NULL)
    {
        return;
    }
    det->low_threshold = low_threshold;
    det->high_threshold = high_threshold;
    det->critical_threshold = critical_threshold;
    det->hysteresis = hysteresis;
    det->zone = WCX_ZONE_NORMAL;
}

wcx_zone_t wcx_threshold_detector_update(wcx_threshold_detector_t *det,
                                         float value)
{
    if (det == NULL)
    {
        return WCX_ZONE_NORMAL;
    }

    float hyst = det->hysteresis;

    switch (det->zone)
    {
    case WCX_ZONE_LOW:
        if (value > det->low_threshold + hyst)
        {
            det->zone = WCX_ZONE_NORMAL;
        }
        break;
    case WCX_ZONE_NORMAL:
        if (value < det->low_threshold)
        {
            det->zone = WCX_ZONE_LOW;
        }
        else if (value > det->high_threshold)
        {
            det->zone = WCX_ZONE_HIGH;
        }
        break;
    case WCX_ZONE_HIGH:
        if (value > det->critical_threshold)
        {
            det->zone = WCX_ZONE_CRITICAL;
        }
        else if (value < det->high_threshold - hyst)
        {
            det->zone = WCX_ZONE_NORMAL;
        }
        break;
    case WCX_ZONE_CRITICAL:
        if (value < det->critical_threshold - hyst)
        {
            det->zone = WCX_ZONE_HIGH;
        }
        break;
    }
    return det->zone;
}

wcx_zone_t wcx_threshold_detector_zone(const wcx_threshold_detector_t *det)
{
    if (det == NULL)
    {
        return WCX_ZONE_NORMAL;
    }
    return det->zone;
}

#ifndef WCX_HYSTERESIS_H
#define WCX_HYSTERESIS_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Hysteresis comparator.  The output goes high when the input rises
     * above the upper threshold and stays high until the input falls
     * below the lower threshold (and vice-versa).
     */
    typedef struct
    {
        float threshold_high;
        float threshold_low;
        bool state;
    } wcx_hysteresis_t;

    /**
     * Threshold detector with configurable high, low and critical bands.
     * Reports which zone the current value occupies.
     */
    typedef enum
    {
        WCX_ZONE_LOW = 0,
        WCX_ZONE_NORMAL = 1,
        WCX_ZONE_HIGH = 2,
        WCX_ZONE_CRITICAL = 3
    } wcx_zone_t;

    typedef struct
    {
        float low_threshold;
        float high_threshold;
        float critical_threshold;
        float hysteresis;
        wcx_zone_t zone;
    } wcx_threshold_detector_t;

    /* Hysteresis comparator API */
    void wcx_hysteresis_init(wcx_hysteresis_t *h, float threshold_low,
                             float threshold_high, bool initial_state);
    bool wcx_hysteresis_update(wcx_hysteresis_t *h, float value);
    bool wcx_hysteresis_state(const wcx_hysteresis_t *h);

    /* Threshold detector API */
    void wcx_threshold_detector_init(wcx_threshold_detector_t *det,
                                     float low_threshold,
                                     float high_threshold,
                                     float critical_threshold,
                                     float hysteresis);
    wcx_zone_t wcx_threshold_detector_update(wcx_threshold_detector_t *det,
                                             float value);
    wcx_zone_t wcx_threshold_detector_zone(const wcx_threshold_detector_t *det);

#ifdef __cplusplus
}
#endif

#endif /* WCX_HYSTERESIS_H */

#ifndef WCX_CALIBRATION_H
#define WCX_CALIBRATION_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        float input_min;
        float input_max;
        float output_min;
        float output_max;
        bool clamp_output;
    } wcx_linear_calibration_t;

    typedef struct
    {
        float scale;
        float offset;
    } wcx_affine_calibration_t;

    void wcx_linear_calibration_init(wcx_linear_calibration_t *calibration,
                                     float input_min,
                                     float input_max,
                                     float output_min,
                                     float output_max,
                                     bool clamp_output);
    float wcx_linear_calibration_apply(const wcx_linear_calibration_t *calibration, float input_value);
    float wcx_linear_calibration_inverse(const wcx_linear_calibration_t *calibration, float output_value);

    void wcx_affine_calibration_init(wcx_affine_calibration_t *calibration,
                                     float scale,
                                     float offset);
    float wcx_affine_calibration_apply(const wcx_affine_calibration_t *calibration, float value);
    float wcx_affine_calibration_inverse(const wcx_affine_calibration_t *calibration, float value);

#ifdef __cplusplus
}
#endif

#endif
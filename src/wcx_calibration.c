#include "wcx_calibration.h"

void wcx_linear_calibration_init(wcx_linear_calibration_t *calibration,
                                 float input_min,
                                 float input_max,
                                 float output_min,
                                 float output_max,
                                 bool clamp_output)
{
    if (calibration == NULL)
    {
        return;
    }

    calibration->input_min = input_min;
    calibration->input_max = input_max;
    calibration->output_min = output_min;
    calibration->output_max = output_max;
    calibration->clamp_output = clamp_output;
}

float wcx_linear_calibration_apply(const wcx_linear_calibration_t *calibration, float input_value)
{
    float output_value;
    float lower_bound;
    float upper_bound;

    if (calibration == NULL)
    {
        return 0.0f;
    }

    output_value = wcx_mapf(input_value,
                            calibration->input_min,
                            calibration->input_max,
                            calibration->output_min,
                            calibration->output_max);

    if (!calibration->clamp_output)
    {
        return output_value;
    }

    lower_bound = WCX_MIN(calibration->output_min, calibration->output_max);
    upper_bound = WCX_MAX(calibration->output_min, calibration->output_max);
    return WCX_CLAMP(output_value, lower_bound, upper_bound);
}

float wcx_linear_calibration_inverse(const wcx_linear_calibration_t *calibration, float output_value)
{
    if (calibration == NULL)
    {
        return 0.0f;
    }

    return wcx_mapf(output_value,
                    calibration->output_min,
                    calibration->output_max,
                    calibration->input_min,
                    calibration->input_max);
}

void wcx_affine_calibration_init(wcx_affine_calibration_t *calibration,
                                 float scale,
                                 float offset)
{
    if (calibration == NULL)
    {
        return;
    }

    calibration->scale = scale;
    calibration->offset = offset;
}

float wcx_affine_calibration_apply(const wcx_affine_calibration_t *calibration, float value)
{
    if (calibration == NULL)
    {
        return 0.0f;
    }

    return (value * calibration->scale) + calibration->offset;
}

float wcx_affine_calibration_inverse(const wcx_affine_calibration_t *calibration, float value)
{
    if (calibration == NULL || calibration->scale == 0.0f)
    {
        return 0.0f;
    }

    return (value - calibration->offset) / calibration->scale;
}
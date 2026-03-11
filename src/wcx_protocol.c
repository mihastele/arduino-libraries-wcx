#include "wcx_protocol.h"

size_t wcx_frame_encoded_capacity(size_t payload_length)
{
    return (payload_length * 2U) + 2U;
}

size_t wcx_frame_encode(const uint8_t *payload,
                        size_t payload_length,
                        uint8_t delimiter,
                        uint8_t escape,
                        uint8_t *output,
                        size_t output_capacity)
{
    size_t input_index;
    size_t output_index = 0U;
    uint8_t byte;

    if (output == NULL || output_capacity < 2U ||
        (payload == NULL && payload_length != 0U))
    {
        return 0U;
    }

    output[output_index++] = delimiter;

    for (input_index = 0U; input_index < payload_length; ++input_index)
    {
        byte = payload[input_index];

        if (byte == delimiter || byte == escape)
        {
            if ((output_index + 2U) > output_capacity)
            {
                return 0U;
            }

            output[output_index++] = escape;
            output[output_index++] = (uint8_t)(byte ^ WCX_FRAME_ESCAPE_XOR);
        }
        else
        {
            if ((output_index + 1U) > output_capacity)
            {
                return 0U;
            }

            output[output_index++] = byte;
        }
    }

    if ((output_index + 1U) > output_capacity)
    {
        return 0U;
    }

    output[output_index++] = delimiter;
    return output_index;
}

bool wcx_frame_decoder_init(wcx_frame_decoder_t *decoder,
                            uint8_t *storage,
                            size_t capacity,
                            uint8_t delimiter,
                            uint8_t escape)
{
    if (decoder == NULL || storage == NULL || capacity == 0U)
    {
        return false;
    }

    decoder->buffer = storage;
    decoder->capacity = capacity;
    decoder->length = 0U;
    decoder->delimiter = delimiter;
    decoder->escape = escape;
    decoder->receiving = false;
    decoder->escape_next = false;
    decoder->overflowed = false;
    return true;
}

void wcx_frame_decoder_reset(wcx_frame_decoder_t *decoder)
{
    if (decoder == NULL)
    {
        return;
    }

    decoder->length = 0U;
    decoder->receiving = false;
    decoder->escape_next = false;
    decoder->overflowed = false;
}

wcx_frame_decoder_result_t wcx_frame_decoder_push(wcx_frame_decoder_t *decoder, uint8_t byte)
{
    if (decoder == NULL)
    {
        return WCX_FRAME_DECODER_ERROR;
    }

    if (!decoder->receiving)
    {
        if (byte == decoder->delimiter)
        {
            decoder->receiving = true;
            decoder->length = 0U;
            decoder->escape_next = false;
            decoder->overflowed = false;
            return WCX_FRAME_DECODER_IN_PROGRESS;
        }

        return WCX_FRAME_DECODER_IDLE;
    }

    if (decoder->overflowed)
    {
        if (byte == decoder->delimiter)
        {
            decoder->length = 0U;
            decoder->escape_next = false;
            decoder->overflowed = false;
            return WCX_FRAME_DECODER_OVERFLOW;
        }

        return WCX_FRAME_DECODER_IN_PROGRESS;
    }

    if (decoder->escape_next)
    {
        decoder->escape_next = false;
        byte = (uint8_t)(byte ^ WCX_FRAME_ESCAPE_XOR);
    }
    else if (byte == decoder->escape)
    {
        decoder->escape_next = true;
        return WCX_FRAME_DECODER_IN_PROGRESS;
    }
    else if (byte == decoder->delimiter)
    {
        decoder->receiving = false;
        decoder->escape_next = false;
        return WCX_FRAME_DECODER_COMPLETE;
    }

    if (decoder->length >= decoder->capacity)
    {
        decoder->length = 0U;
        decoder->escape_next = false;
        decoder->overflowed = true;
        return WCX_FRAME_DECODER_IN_PROGRESS;
    }

    decoder->buffer[decoder->length++] = byte;
    return WCX_FRAME_DECODER_IN_PROGRESS;
}

const uint8_t *wcx_frame_decoder_data(const wcx_frame_decoder_t *decoder)
{
    if (decoder == NULL)
    {
        return NULL;
    }

    return decoder->buffer;
}

size_t wcx_frame_decoder_length(const wcx_frame_decoder_t *decoder)
{
    if (decoder == NULL)
    {
        return 0U;
    }

    return decoder->length;
}
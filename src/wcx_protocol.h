#ifndef WCX_PROTOCOL_H
#define WCX_PROTOCOL_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define WCX_FRAME_ESCAPE_XOR 0x20U

    typedef enum
    {
        WCX_FRAME_DECODER_IDLE = 0,
        WCX_FRAME_DECODER_IN_PROGRESS,
        WCX_FRAME_DECODER_COMPLETE,
        WCX_FRAME_DECODER_OVERFLOW,
        WCX_FRAME_DECODER_ERROR
    } wcx_frame_decoder_result_t;

    typedef struct
    {
        uint8_t *buffer;
        size_t capacity;
        size_t length;
        uint8_t delimiter;
        uint8_t escape;
        bool receiving;
        bool escape_next;
        bool overflowed;
    } wcx_frame_decoder_t;

    size_t wcx_frame_encoded_capacity(size_t payload_length);
    size_t wcx_frame_encode(const uint8_t *payload,
                            size_t payload_length,
                            uint8_t delimiter,
                            uint8_t escape,
                            uint8_t *output,
                            size_t output_capacity);

    bool wcx_frame_decoder_init(wcx_frame_decoder_t *decoder,
                                uint8_t *storage,
                                size_t capacity,
                                uint8_t delimiter,
                                uint8_t escape);
    void wcx_frame_decoder_reset(wcx_frame_decoder_t *decoder);
    wcx_frame_decoder_result_t wcx_frame_decoder_push(wcx_frame_decoder_t *decoder, uint8_t byte);
    const uint8_t *wcx_frame_decoder_data(const wcx_frame_decoder_t *decoder);
    size_t wcx_frame_decoder_length(const wcx_frame_decoder_t *decoder);

#ifdef __cplusplus
}
#endif

#endif
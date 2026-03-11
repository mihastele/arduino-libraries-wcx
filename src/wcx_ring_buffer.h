#ifndef WCX_RING_BUFFER_H
#define WCX_RING_BUFFER_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint8_t *buffer;
        size_t capacity;
        size_t head;
        size_t tail;
        size_t size;
    } wcx_ring_buffer_t;

    bool wcx_ring_buffer_init(wcx_ring_buffer_t *ring, uint8_t *storage, size_t capacity);
    void wcx_ring_buffer_clear(wcx_ring_buffer_t *ring);
    bool wcx_ring_buffer_is_empty(const wcx_ring_buffer_t *ring);
    bool wcx_ring_buffer_is_full(const wcx_ring_buffer_t *ring);
    size_t wcx_ring_buffer_size(const wcx_ring_buffer_t *ring);
    size_t wcx_ring_buffer_capacity(const wcx_ring_buffer_t *ring);
    bool wcx_ring_buffer_push(wcx_ring_buffer_t *ring, uint8_t value);
    bool wcx_ring_buffer_pop(wcx_ring_buffer_t *ring, uint8_t *value);
    bool wcx_ring_buffer_peek(const wcx_ring_buffer_t *ring, uint8_t *value);
    size_t wcx_ring_buffer_write(wcx_ring_buffer_t *ring, const uint8_t *data, size_t length);
    size_t wcx_ring_buffer_read(wcx_ring_buffer_t *ring, uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif

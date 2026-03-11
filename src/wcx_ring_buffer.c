#include "wcx_ring_buffer.h"

static size_t wcx_ring_buffer_advance(const wcx_ring_buffer_t *ring, size_t index)
{
    return (index + 1U) % ring->capacity;
}

bool wcx_ring_buffer_init(wcx_ring_buffer_t *ring, uint8_t *storage, size_t capacity)
{
    if (ring == NULL || storage == NULL || capacity == 0U)
    {
        return false;
    }

    ring->buffer = storage;
    ring->capacity = capacity;
    ring->head = 0U;
    ring->tail = 0U;
    ring->size = 0U;
    return true;
}

void wcx_ring_buffer_clear(wcx_ring_buffer_t *ring)
{
    if (ring == NULL)
    {
        return;
    }

    ring->head = 0U;
    ring->tail = 0U;
    ring->size = 0U;
}

bool wcx_ring_buffer_is_empty(const wcx_ring_buffer_t *ring)
{
    return ring == NULL || ring->size == 0U;
}

bool wcx_ring_buffer_is_full(const wcx_ring_buffer_t *ring)
{
    return ring != NULL && ring->size == ring->capacity;
}

size_t wcx_ring_buffer_size(const wcx_ring_buffer_t *ring)
{
    return ring == NULL ? 0U : ring->size;
}

size_t wcx_ring_buffer_capacity(const wcx_ring_buffer_t *ring)
{
    return ring == NULL ? 0U : ring->capacity;
}

bool wcx_ring_buffer_push(wcx_ring_buffer_t *ring, uint8_t value)
{
    if (ring == NULL || wcx_ring_buffer_is_full(ring))
    {
        return false;
    }

    ring->buffer[ring->head] = value;
    ring->head = wcx_ring_buffer_advance(ring, ring->head);
    ring->size++;
    return true;
}

bool wcx_ring_buffer_pop(wcx_ring_buffer_t *ring, uint8_t *value)
{
    if (ring == NULL || value == NULL || wcx_ring_buffer_is_empty(ring))
    {
        return false;
    }

    *value = ring->buffer[ring->tail];
    ring->tail = wcx_ring_buffer_advance(ring, ring->tail);
    ring->size--;
    return true;
}

bool wcx_ring_buffer_peek(const wcx_ring_buffer_t *ring, uint8_t *value)
{
    if (ring == NULL || value == NULL || wcx_ring_buffer_is_empty(ring))
    {
        return false;
    }

    *value = ring->buffer[ring->tail];
    return true;
}

size_t wcx_ring_buffer_write(wcx_ring_buffer_t *ring, const uint8_t *data, size_t length)
{
    size_t written = 0U;

    if (ring == NULL || data == NULL)
    {
        return 0U;
    }

    while (written < length && wcx_ring_buffer_push(ring, data[written]))
    {
        written++;
    }

    return written;
}

size_t wcx_ring_buffer_read(wcx_ring_buffer_t *ring, uint8_t *data, size_t length)
{
    size_t read = 0U;

    if (ring == NULL || data == NULL)
    {
        return 0U;
    }

    while (read < length && wcx_ring_buffer_pop(ring, &data[read]))
    {
        read++;
    }

    return read;
}

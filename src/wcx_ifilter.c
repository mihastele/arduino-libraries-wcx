#include "wcx_ifilter.h"

/* ---- Integer moving average ---- */

bool wcx_imoving_average_init(wcx_imoving_average_t *f,
                              int32_t *storage, size_t capacity)
{
    if (f == NULL || storage == NULL || capacity == 0U)
    {
        return false;
    }
    f->storage = storage;
    f->capacity = capacity;
    f->count = 0U;
    f->index = 0U;
    f->sum = 0;
    for (size_t i = 0U; i < capacity; i++)
    {
        storage[i] = 0;
    }
    return true;
}

void wcx_imoving_average_reset(wcx_imoving_average_t *f)
{
    if (f == NULL)
    {
        return;
    }
    f->count = 0U;
    f->index = 0U;
    f->sum = 0;
    for (size_t i = 0U; i < f->capacity; i++)
    {
        f->storage[i] = 0;
    }
}

int32_t wcx_imoving_average_update(wcx_imoving_average_t *f, int32_t sample)
{
    if (f == NULL)
    {
        return 0;
    }
    if (f->count >= f->capacity)
    {
        f->sum -= f->storage[f->index];
    }
    else
    {
        f->count++;
    }
    f->storage[f->index] = sample;
    f->sum += sample;
    f->index = (f->index + 1U) % f->capacity;
    return f->sum / (int32_t)f->count;
}

int32_t wcx_imoving_average_value(const wcx_imoving_average_t *f)
{
    if (f == NULL || f->count == 0U)
    {
        return 0;
    }
    return f->sum / (int32_t)f->count;
}

size_t wcx_imoving_average_count(const wcx_imoving_average_t *f)
{
    if (f == NULL)
    {
        return 0U;
    }
    return f->count;
}

/* ---- Integer EMA ---- */

void wcx_iema_init(wcx_iema_t *f, uint8_t shift)
{
    if (f == NULL)
    {
        return;
    }
    f->value = 0;
    f->shift = (shift > 0U) ? shift : 1U;
    f->primed = false;
}

void wcx_iema_reset(wcx_iema_t *f)
{
    if (f == NULL)
    {
        return;
    }
    f->value = 0;
    f->primed = false;
}

int32_t wcx_iema_update(wcx_iema_t *f, int32_t sample)
{
    if (f == NULL)
    {
        return 0;
    }
    if (!f->primed)
    {
        f->value = sample;
        f->primed = true;
    }
    else
    {
        f->value += (sample - f->value) >> f->shift;
    }
    return f->value;
}

int32_t wcx_iema_value(const wcx_iema_t *f)
{
    if (f == NULL)
    {
        return 0;
    }
    return f->value;
}

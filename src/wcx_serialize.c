#include "wcx_serialize.h"

/* ---- Standalone big-endian pack/unpack ---- */

size_t wcx_pack_u8(uint8_t *buf, size_t buf_size, uint8_t value)
{
    if (buf == NULL || buf_size < 1U)
    {
        return 0U;
    }
    buf[0] = value;
    return 1U;
}

size_t wcx_pack_u16_be(uint8_t *buf, size_t buf_size, uint16_t value)
{
    if (buf == NULL || buf_size < 2U)
    {
        return 0U;
    }
    buf[0] = (uint8_t)(value >> 8);
    buf[1] = (uint8_t)(value);
    return 2U;
}

size_t wcx_pack_u32_be(uint8_t *buf, size_t buf_size, uint32_t value)
{
    if (buf == NULL || buf_size < 4U)
    {
        return 0U;
    }
    buf[0] = (uint8_t)(value >> 24);
    buf[1] = (uint8_t)(value >> 16);
    buf[2] = (uint8_t)(value >> 8);
    buf[3] = (uint8_t)(value);
    return 4U;
}

size_t wcx_pack_i16_be(uint8_t *buf, size_t buf_size, int16_t value)
{
    return wcx_pack_u16_be(buf, buf_size, (uint16_t)value);
}

size_t wcx_pack_i32_be(uint8_t *buf, size_t buf_size, int32_t value)
{
    return wcx_pack_u32_be(buf, buf_size, (uint32_t)value);
}

uint8_t wcx_unpack_u8(const uint8_t *buf, size_t buf_size)
{
    if (buf == NULL || buf_size < 1U)
    {
        return 0U;
    }
    return buf[0];
}

uint16_t wcx_unpack_u16_be(const uint8_t *buf, size_t buf_size)
{
    if (buf == NULL || buf_size < 2U)
    {
        return 0U;
    }
    return (uint16_t)((uint16_t)buf[0] << 8) | (uint16_t)buf[1];
}

uint32_t wcx_unpack_u32_be(const uint8_t *buf, size_t buf_size)
{
    if (buf == NULL || buf_size < 4U)
    {
        return 0U;
    }
    return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
           ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
}

int16_t wcx_unpack_i16_be(const uint8_t *buf, size_t buf_size)
{
    return (int16_t)wcx_unpack_u16_be(buf, buf_size);
}

int32_t wcx_unpack_i32_be(const uint8_t *buf, size_t buf_size)
{
    return (int32_t)wcx_unpack_u32_be(buf, buf_size);
}

/* ---- Standalone little-endian pack/unpack ---- */

size_t wcx_pack_u16_le(uint8_t *buf, size_t buf_size, uint16_t value)
{
    if (buf == NULL || buf_size < 2U)
    {
        return 0U;
    }
    buf[0] = (uint8_t)(value);
    buf[1] = (uint8_t)(value >> 8);
    return 2U;
}

size_t wcx_pack_u32_le(uint8_t *buf, size_t buf_size, uint32_t value)
{
    if (buf == NULL || buf_size < 4U)
    {
        return 0U;
    }
    buf[0] = (uint8_t)(value);
    buf[1] = (uint8_t)(value >> 8);
    buf[2] = (uint8_t)(value >> 16);
    buf[3] = (uint8_t)(value >> 24);
    return 4U;
}

size_t wcx_pack_i16_le(uint8_t *buf, size_t buf_size, int16_t value)
{
    return wcx_pack_u16_le(buf, buf_size, (uint16_t)value);
}

size_t wcx_pack_i32_le(uint8_t *buf, size_t buf_size, int32_t value)
{
    return wcx_pack_u32_le(buf, buf_size, (uint32_t)value);
}

uint16_t wcx_unpack_u16_le(const uint8_t *buf, size_t buf_size)
{
    if (buf == NULL || buf_size < 2U)
    {
        return 0U;
    }
    return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
}

uint32_t wcx_unpack_u32_le(const uint8_t *buf, size_t buf_size)
{
    if (buf == NULL || buf_size < 4U)
    {
        return 0U;
    }
    return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) |
           ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}

int16_t wcx_unpack_i16_le(const uint8_t *buf, size_t buf_size)
{
    return (int16_t)wcx_unpack_u16_le(buf, buf_size);
}

int32_t wcx_unpack_i32_le(const uint8_t *buf, size_t buf_size)
{
    return (int32_t)wcx_unpack_u32_le(buf, buf_size);
}

/* ---- Cursor-based packer ---- */

void wcx_packer_init(wcx_packer_t *p, uint8_t *buf, size_t capacity)
{
    if (p == NULL)
    {
        return;
    }
    p->buf = buf;
    p->capacity = (buf != NULL) ? capacity : 0U;
    p->pos = 0U;
    p->overflow = false;
}

static bool packer_reserve(wcx_packer_t *p, size_t n)
{
    if (p->overflow || p->buf == NULL)
    {
        return false;
    }
    if (p->pos + n > p->capacity)
    {
        p->overflow = true;
        return false;
    }
    return true;
}

bool wcx_packer_u8(wcx_packer_t *p, uint8_t value)
{
    if (p == NULL)
    {
        return false;
    }
    if (!packer_reserve(p, 1U))
    {
        return false;
    }
    p->buf[p->pos++] = value;
    return true;
}

bool wcx_packer_u16_be(wcx_packer_t *p, uint16_t value)
{
    if (p == NULL)
    {
        return false;
    }
    if (!packer_reserve(p, 2U))
    {
        return false;
    }
    p->buf[p->pos++] = (uint8_t)(value >> 8);
    p->buf[p->pos++] = (uint8_t)(value);
    return true;
}

bool wcx_packer_u32_be(wcx_packer_t *p, uint32_t value)
{
    if (p == NULL)
    {
        return false;
    }
    if (!packer_reserve(p, 4U))
    {
        return false;
    }
    p->buf[p->pos++] = (uint8_t)(value >> 24);
    p->buf[p->pos++] = (uint8_t)(value >> 16);
    p->buf[p->pos++] = (uint8_t)(value >> 8);
    p->buf[p->pos++] = (uint8_t)(value);
    return true;
}

bool wcx_packer_i16_be(wcx_packer_t *p, int16_t value)
{
    return wcx_packer_u16_be(p, (uint16_t)value);
}

bool wcx_packer_i32_be(wcx_packer_t *p, int32_t value)
{
    return wcx_packer_u32_be(p, (uint32_t)value);
}

size_t wcx_packer_length(const wcx_packer_t *p)
{
    if (p == NULL)
    {
        return 0U;
    }
    return p->pos;
}

bool wcx_packer_ok(const wcx_packer_t *p)
{
    if (p == NULL)
    {
        return false;
    }
    return !p->overflow;
}

/* ---- Cursor-based unpacker ---- */

void wcx_unpacker_init(wcx_unpacker_t *u, const uint8_t *buf,
                       size_t length)
{
    if (u == NULL)
    {
        return;
    }
    u->buf = buf;
    u->length = (buf != NULL) ? length : 0U;
    u->pos = 0U;
    u->overflow = false;
}

static bool unpacker_reserve(wcx_unpacker_t *u, size_t n)
{
    if (u->overflow || u->buf == NULL)
    {
        return false;
    }
    if (u->pos + n > u->length)
    {
        u->overflow = true;
        return false;
    }
    return true;
}

uint8_t wcx_unpacker_u8(wcx_unpacker_t *u)
{
    if (u == NULL)
    {
        return 0U;
    }
    if (!unpacker_reserve(u, 1U))
    {
        return 0U;
    }
    return u->buf[u->pos++];
}

uint16_t wcx_unpacker_u16_be(wcx_unpacker_t *u)
{
    if (u == NULL)
    {
        return 0U;
    }
    if (!unpacker_reserve(u, 2U))
    {
        return 0U;
    }
    uint16_t v = ((uint16_t)u->buf[u->pos] << 8) |
                 (uint16_t)u->buf[u->pos + 1U];
    u->pos += 2U;
    return v;
}

uint32_t wcx_unpacker_u32_be(wcx_unpacker_t *u)
{
    if (u == NULL)
    {
        return 0U;
    }
    if (!unpacker_reserve(u, 4U))
    {
        return 0U;
    }
    uint32_t v = ((uint32_t)u->buf[u->pos] << 24) |
                 ((uint32_t)u->buf[u->pos + 1U] << 16) |
                 ((uint32_t)u->buf[u->pos + 2U] << 8) |
                 (uint32_t)u->buf[u->pos + 3U];
    u->pos += 4U;
    return v;
}

int16_t wcx_unpacker_i16_be(wcx_unpacker_t *u)
{
    return (int16_t)wcx_unpacker_u16_be(u);
}

int32_t wcx_unpacker_i32_be(wcx_unpacker_t *u)
{
    return (int32_t)wcx_unpacker_u32_be(u);
}

size_t wcx_unpacker_remaining(const wcx_unpacker_t *u)
{
    if (u == NULL)
    {
        return 0U;
    }
    if (u->pos >= u->length)
    {
        return 0U;
    }
    return u->length - u->pos;
}

bool wcx_unpacker_ok(const wcx_unpacker_t *u)
{
    if (u == NULL)
    {
        return false;
    }
    return !u->overflow;
}

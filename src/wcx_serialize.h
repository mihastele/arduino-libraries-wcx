#ifndef WCX_SERIALIZE_H
#define WCX_SERIALIZE_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Byte-order-safe serialization helpers.
     * All pack functions write into a caller-supplied buffer and return
     * the number of bytes written (0 on NULL).
     * All unpack functions read from a buffer and return the decoded value.
     */

    /* Big-endian (network byte order) */
    size_t wcx_pack_u8(uint8_t *buf, size_t buf_size, uint8_t value);
    size_t wcx_pack_u16_be(uint8_t *buf, size_t buf_size, uint16_t value);
    size_t wcx_pack_u32_be(uint8_t *buf, size_t buf_size, uint32_t value);
    size_t wcx_pack_i16_be(uint8_t *buf, size_t buf_size, int16_t value);
    size_t wcx_pack_i32_be(uint8_t *buf, size_t buf_size, int32_t value);

    uint8_t wcx_unpack_u8(const uint8_t *buf, size_t buf_size);
    uint16_t wcx_unpack_u16_be(const uint8_t *buf, size_t buf_size);
    uint32_t wcx_unpack_u32_be(const uint8_t *buf, size_t buf_size);
    int16_t wcx_unpack_i16_be(const uint8_t *buf, size_t buf_size);
    int32_t wcx_unpack_i32_be(const uint8_t *buf, size_t buf_size);

    /* Little-endian */
    size_t wcx_pack_u16_le(uint8_t *buf, size_t buf_size, uint16_t value);
    size_t wcx_pack_u32_le(uint8_t *buf, size_t buf_size, uint32_t value);
    size_t wcx_pack_i16_le(uint8_t *buf, size_t buf_size, int16_t value);
    size_t wcx_pack_i32_le(uint8_t *buf, size_t buf_size, int32_t value);

    uint16_t wcx_unpack_u16_le(const uint8_t *buf, size_t buf_size);
    uint32_t wcx_unpack_u32_le(const uint8_t *buf, size_t buf_size);
    int16_t wcx_unpack_i16_le(const uint8_t *buf, size_t buf_size);
    int32_t wcx_unpack_i32_le(const uint8_t *buf, size_t buf_size);

    /**
     * Cursor-based packing into a buffer.  Tracks the current write
     * position so the caller does not have to manage offsets manually.
     */
    typedef struct
    {
        uint8_t *buf;
        size_t capacity;
        size_t pos;
        bool overflow;
    } wcx_packer_t;

    void wcx_packer_init(wcx_packer_t *p, uint8_t *buf, size_t capacity);
    bool wcx_packer_u8(wcx_packer_t *p, uint8_t value);
    bool wcx_packer_u16_be(wcx_packer_t *p, uint16_t value);
    bool wcx_packer_u32_be(wcx_packer_t *p, uint32_t value);
    bool wcx_packer_i16_be(wcx_packer_t *p, int16_t value);
    bool wcx_packer_i32_be(wcx_packer_t *p, int32_t value);
    size_t wcx_packer_length(const wcx_packer_t *p);
    bool wcx_packer_ok(const wcx_packer_t *p);

    /**
     * Cursor-based unpacking from a buffer.
     */
    typedef struct
    {
        const uint8_t *buf;
        size_t length;
        size_t pos;
        bool overflow;
    } wcx_unpacker_t;

    void wcx_unpacker_init(wcx_unpacker_t *u, const uint8_t *buf,
                           size_t length);
    uint8_t wcx_unpacker_u8(wcx_unpacker_t *u);
    uint16_t wcx_unpacker_u16_be(wcx_unpacker_t *u);
    uint32_t wcx_unpacker_u32_be(wcx_unpacker_t *u);
    int16_t wcx_unpacker_i16_be(wcx_unpacker_t *u);
    int32_t wcx_unpacker_i32_be(wcx_unpacker_t *u);
    size_t wcx_unpacker_remaining(const wcx_unpacker_t *u);
    bool wcx_unpacker_ok(const wcx_unpacker_t *u);

#ifdef __cplusplus
}
#endif

#endif /* WCX_SERIALIZE_H */

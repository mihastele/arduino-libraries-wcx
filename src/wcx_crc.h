#ifndef WCX_CRC_H
#define WCX_CRC_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    uint8_t wcx_crc8(const uint8_t *data, size_t length, uint8_t initial_value);
    uint16_t wcx_crc16_ccitt(const uint8_t *data, size_t length, uint16_t initial_value);
    uint8_t wcx_xor_checksum(const uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif

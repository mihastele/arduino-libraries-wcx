#include "wcx_crc.h"

uint8_t wcx_crc8(const uint8_t *data, size_t length, uint8_t initial_value)
{
    size_t index;
    uint8_t bit;
    uint8_t crc = initial_value;

    if (data == NULL)
    {
        return crc;
    }

    for (index = 0U; index < length; ++index)
    {
        crc ^= data[index];
        for (bit = 0U; bit < 8U; ++bit)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (uint8_t)((crc << 1U) ^ 0x07U);
            }
            else
            {
                crc <<= 1U;
            }
        }
    }

    return crc;
}

uint16_t wcx_crc16_ccitt(const uint8_t *data, size_t length, uint16_t initial_value)
{
    size_t index;
    uint8_t bit;
    uint16_t crc = initial_value;

    if (data == NULL)
    {
        return crc;
    }

    for (index = 0U; index < length; ++index)
    {
        crc ^= (uint16_t)data[index] << 8U;
        for (bit = 0U; bit < 8U; ++bit)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (uint16_t)((crc << 1U) ^ 0x1021U);
            }
            else
            {
                crc <<= 1U;
            }
        }
    }

    return crc;
}

uint8_t wcx_xor_checksum(const uint8_t *data, size_t length)
{
    size_t index;
    uint8_t checksum = 0U;

    if (data == NULL)
    {
        return checksum;
    }

    for (index = 0U; index < length; ++index)
    {
        checksum ^= data[index];
    }

    return checksum;
}

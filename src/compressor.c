/**
 * Copyright (C) 2023 Your Name <jlnbotello@gmail.com>.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "compressor.h"

#define CONTINUE_BIT_MASK 0x80 /**< Continuation bit mask */
#define PAYLOAD_MASK 0x7F      /**< Payload mask */

typedef union
{
    uint32_t integer;
    uint8_t bytes[4];
} sample_t;

RetCode delta_encoding(const int32_t in[], int32_t out[], size_t len)
{
    // Error checking
    if (!in || !out)
    {
        return ERROR_NULL_POINTER;
    }

    if (len < 2)
    {
        return ERROR_INVALID_LENGTH;
    }

    // Encoding logic
    out[0] = in[0];

    for (size_t i = 1; i < len; i++)
    {
        out[i] = in[i] - in[i - 1];
    }

    return SUCCESS;
}

RetCode delta_decoding(const int32_t in[], int32_t out[], size_t len)
{
    // Error checking
    if (!in || !out)
    {
        return ERROR_NULL_POINTER;
    }

    if (len < 2)
    {
        return ERROR_INVALID_LENGTH;
    }

    // Decoding logic
    out[0] = in[0];

    for (size_t i = 1; i < len; i++)
    {
        out[i] = in[i] + out[i - 1];
    }

    return SUCCESS;
}

RetCode zigzag_encoding(const int32_t in[], uint32_t out[], size_t len)
{
    // Error checking
    if (!in || !out)
    {
        return ERROR_NULL_POINTER;
    }

    if (len < 1)
    {
        return ERROR_INVALID_LENGTH;
    }

    // Encoding logic
    for (size_t i = 0; i < len; i++)
    {
        if (in[i] == 0)
        {
            out[i] = in[i];
        }
        else
        {
            if (in[i] > 0)
            {
                out[i] = in[i] << 1;
            }
            else
            {
                out[i] = ((~in[i] + 1) << 1) - 1;
            }
        }
    }

    return SUCCESS;
}

RetCode zigzag_decoding(const uint32_t in[], int32_t out[], size_t len)
{
    // Error checking
    if (!out)
    {
        return ERROR_NULL_POINTER;
    }

    if (len < 1)
    {
        return ERROR_INVALID_LENGTH;
    }

    // Decoding logic
    for (size_t i = 0; i < len; i++)
    {
        if (in[i] == 0)
        {
            out[i] = 0;
        }
        else
        {
           if (in[i] % 2 == 0)
            {
                out[i] = in[i] >> 1; // Positive value
            }
            else
            {
                out[i] = -((in[i] + 1) >> 1); // Negative value
            }
        }
    }

    return SUCCESS;
}

RetCode varuint_encoding(const uint32_t in[], size_t in_len, uint8_t out[], size_t *out_len)
{
    sample_t aux;
    size_t out_index = 0;

    for (size_t i = 0; i < in_len; i++)
    {
        aux.integer = in[i];

        out[out_index] = aux.bytes[0] & PAYLOAD_MASK;
        aux.integer <<= 1;
        out_index++;

        for (uint8_t j = 1; j < 4; j++)
        {
            if (aux.bytes[j])
            {
                out[out_index - 1] |= CONTINUE_BIT_MASK;
                out[out_index] = aux.bytes[j] & PAYLOAD_MASK;
                aux.integer <<= 1;
                out_index++;
            }
            else
            {
                break;
            }
        }
    }

    *out_len = out_index;

    return SUCCESS;
}

RetCode varuint_decoding(const uint8_t in[], size_t in_len, uint32_t out[], size_t *out_len)
{
    size_t out_index = 0;
    uint8_t bit_shifting = 0;
    uint32_t decoded_value = 0;

    for (size_t i = 0; i < in_len; i++)
    {
        decoded_value |= (in[i] & PAYLOAD_MASK) << bit_shifting;
        bit_shifting += 7;

        if ((in[i] & CONTINUE_BIT_MASK) == 0)
        {
            out[out_index] = decoded_value;
            out_index++;
            bit_shifting = 0;
            decoded_value = 0;
        }
    }

    *out_len = out_index;

    return SUCCESS;
}
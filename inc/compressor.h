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

#include "stdint.h"
#include "stddef.h"

typedef enum
{
    ERROR_NULL_POINTER = -3,
    ERROR_INVALID_LENGTH,
    ERROR_GENERIC,

    SUCCESS    
} RetCode;

/**
 * @brief Encodes a buffer of integers in delta encoding
 *
 * @param in  Input buffer
 * @param out Output buffer
 * @param len Data length (both buffers have the same length)
 *
 * @return Status code indicating the success or failure of the operation.
 */
RetCode delta_encoding(const int32_t in[], int32_t out[], size_t len);

/**
 * @brief Decodes a buffer of delta-encoded integers
 *
 * @param in  Input buffer
 * @param out Output buffer
 * @param len Data length (both buffers have the same length)
 *
 * @return Status code indicating the success or failure of the operation.
 */
RetCode delta_decoding(const int32_t in[], int32_t out[], size_t len);

/**
 * @brief Perform zigzag encoding on an array of integers.
 *
 * This function performs zigzag encoding on the input array of integers.
 * Zigzag encoding maps signed integers to positive integers for efficient
 * varint encoding. Positive integers are left-shifted by 1, while negative
 * integers are mapped to positive values and then left-shifted by 1 and subtracted by 1.
 *
 * Example: (0 = 0, -1 = 1, 1 = 2, -2 = 3, 2 = 4, -3 = 5, 3 = 6 ...)
 * 
 * @param in Input array of integers to be zigzag-encoded.
 * @param out Output array to store the zigzag-encoded results.
 * @param len Number of integers in the input and output arrays.
 * @return Status code indicating the success or failure of the operation.
 *         - SUCCESS: Zigzag encoding was successful.
 *         - ERROR_NULL_POINTER: Input or output arrays are NULL.
 *         - ERROR_INVALID_LENGTH: Invalid array length.
 */
RetCode zigzag_encoding(const int32_t in[], uint32_t out[], size_t len);

/**
 * @brief Perform zigzag decoding on an array of integers.
 *
 * This function performs zigzag decoding on the input array of zigzag-encoded integers,
 * converting them back to their original signed integer values.
 *
 * @param in Input array of zigzag-encoded integers to be decoded.
 * @param out Output array to store the decoded results.
 * @param len Number of integers in the input and output arrays.
 * @return Status code indicating the success or failure of the operation.
 *         - SUCCESS: Zigzag decoding was successful.
 *         - ERROR_NULL_POINTER: Output array is NULL.
 *         - ERROR_INVALID_LENGTH: Invalid array length.
 */
RetCode zigzag_decoding(const uint32_t in[], int32_t out[], size_t len);

/**
 * @brief Encode an array of 32-bit integers into variable-length format.
 *
 * This function encodes an array of 32-bit integers into a variable-length
 * byte sequence using a continuation bit and payload mask. The encoded bytes
 * are stored in the output array, and the length of the encoded data is updated.
 *
 * @param in Input array of 32-bit integers to be encoded.
 * @param in_len Number of integers in the input array.
 * @param out Output array to store the encoded bytes.
 * @param out_len Pointer to the length of the encoded data.
 * @return Status code indicating the success or failure of the operation.
 */
RetCode varuint_encoding(const uint32_t in[], size_t in_len, uint8_t out[], size_t *out_len);

/**
 * @brief Decode a variable-length encoded byte sequence into an array of 32-bit integers.
 *
 * This function decodes a variable-length encoded byte sequence back into an
 * array of 32-bit integers using the continuation bit and payload mask.
 * The decoded integers are stored in the output array, and the length of the
 * decoded data is updated.
 *
 * @param in Input array of bytes in variable-length encoded format.
 * @param in_len Number of bytes in the input array.
 * @param out Output array to store the decoded 32-bit integers.
 * @param out_len Pointer to the length of the decoded data.
 * @return Status code indicating the success or failure of the operation.
 */
RetCode varuint_decoding(const uint8_t in[], size_t in_len, uint32_t out[], size_t *out_len);
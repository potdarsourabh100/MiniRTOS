/**
 * \file           StdUtils.h
 * \brief          Standard utility header file
 */

/*
 * Copyright (c) 2024 Sourabh Potdar
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sub-license, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author:          Sourabh Potdar
 * Version:         V1.0.0
 */

#ifndef STDUTIL_H_
#define STDUTIL_H_

/*****************************************************************************/
/* Include Files                                                             */
/*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>

/*****************************************************************************/
/* Boolean and State Macros                                                  */
/*****************************************************************************/
#define FALSE                   0
#define TRUE                    1

#define YES                     1
#define NO                      0

#define READ                    0
#define WRITE                   1

#define INACTIVE                0
#define ACTIVE                  1

#define INVALID                 0
#define VALID                   1

#define ON                      1
#define OFF                     0

#define SET                     1
#define CLEAR                   0

#define HIGH                    1
#define LOW                     0

#define OK                      1
#define NOK                     0

/*****************************************************************************/
/* Status Macros                                                             */
/*****************************************************************************/
#define ZERO                    0x00u
#define NULL_CHAR               '\0'

#define FAILED                  0x00u
#define SUCCESSFUL              0x01u
#define BUSY                    0x02u

/*****************************************************************************/
/* Utility Function Macros                                                  */
/*****************************************************************************/
#define STDUTIL_GET_BIT_MASK(bit)            (1U << (bit))
#define STDUTIL_BIT_SET(x, bit)              ((x) |= STDUTIL_GET_BIT_MASK(bit))
#define STDUTIL_BIT_CLEAR(x, bit)            ((x) &= ~STDUTIL_GET_BIT_MASK(bit))
#define STDUTIL_BIT_TOGGLE(x, bit)           ((x) ^= STDUTIL_GET_BIT_MASK(bit))
#define STDUTIL_UPDATE_BIT(x, bit, val)      ((val) ? STDUTIL_BIT_SET(x, bit) : STDUTIL_BIT_CLEAR(x, bit))

#define STDUTIL_GET_BIT_STATUS(x, bit)       (((x) & STDUTIL_GET_BIT_MASK(bit)) != 0U)
#define STDUTIL_IS_BIT_SET(x, bit)           STDUTIL_GET_BIT_STATUS(x, bit)
#define STDUTIL_IS_BIT_CLEARED(x, bit)       (((x) & STDUTIL_GET_BIT_MASK(bit)) == 0U)

#define STDUTIL_ARE_ALL_BITS_SET(x, mask)    (((x) & (mask)) == (mask))
#define STDUTIL_ARE_ANY_BITS_SET(x, mask)    (((x) & (mask)) != 0U)

/*****************************************************************************/
/* Nibble and Byte Extraction Macros                                         */
/*****************************************************************************/
#define STDUTIL_NIBBLE_0_4(x)                ((uint8_t)((x)       & 0x0FU))
#define STDUTIL_NIBBLE_4_8(x)                ((uint8_t)(((x) >> 4)  & 0x0FU))
#define STDUTIL_NIBBLE_8_12(x)               ((uint8_t)(((x) >> 8)  & 0x0FU))
#define STDUTIL_NIBBLE_12_16(x)              ((uint8_t)(((x) >> 12) & 0x0FU))

#define STDUTIL_BYTE_0_8(x)                  ((uint8_t)((x)       & 0xFFU))
#define STDUTIL_BYTE_8_16(x)                 ((uint8_t)(((x) >> 8)  & 0xFFU))
#define STDUTIL_BYTE_16_24(x)                ((uint8_t)(((x) >> 16) & 0xFFU))
#define STDUTIL_BYTE_24_32(x)                ((uint8_t)(((x) >> 24) & 0xFFU))

/*****************************************************************************/
/* ASCII and HEX Conversion Macros                                           */
/*****************************************************************************/
#define STDUTIL_ASCII_TO_HEX(c)              (((c) > 0x39U) ? (((c) + 10U) - 0x41U) : ((c) - 0x30U))
#define STDUTIL_HEX_TO_ASCII(h)              (((h) > 0x09U) ? (((h) - 10U) + 0x41U) : ((h) + 0x30U))

/*****************************************************************************/
/* Mathematical Utility Macros                                               */
/*****************************************************************************/
#define STDUTIL_MIN(a, b)                    (((a) < (b)) ? (a) : (b))
#define STDUTIL_MAX(a, b)                    (((a) > (b)) ? (a) : (b))
#define STDUTIL_ABS(x)                       (((x) < 0) ? -(x) : (x))
#define STDUTIL_CLAMP(val, min, max)         (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val))

/*****************************************************************************/
/* Mathematical Functions                                                    */
/*****************************************************************************/
/**
 * @brief Maps a given integer value from one range to another.
 *
 * @details Linearly converts a value from the input range [in_min, in_max]
 *          to an output range [out_min, out_max].
 *
 * @param[in] x        Input value to be mapped.
 * @param[in] in_min   Lower bound of the input range.
 * @param[in] in_max   Upper bound of the input range.
 * @param[in] out_min  Lower bound of the output range.
 * @param[in] out_max  Upper bound of the output range.
 *
 * @return Mapped output value in the range [out_min, out_max].
 *
 * @note Does not constrain the result within the output range.
 */

static inline int32_t stdUtil_map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
/*****************************************************************************/
/* Debug Print Utility                                                       */
/*****************************************************************************/
#define DEBUG_PRINTF_BUFFER_SIZE             128U

/**
 * @brief Weak implementation of putchar for debug printing.
 *        Override this in your application with actual serial/UART code.
 *
 * @param[in] c Character to output
 */
__attribute__((weak)) void stdUtil_putChar(char c)
{
    (void)c; /* Suppress unused warning */
    /* Example:
     * USART1->DR = c;
     * or
     * HAL_UART_Transmit(&huart, (uint8_t *)&c, 1, 10);
     */
}

/**
 * @brief Lightweight printf using internal buffer and StdUtil_PutChar().
 *
 * @param[in] format Format string
 * @param[in] ...    Variable arguments
 */
inline void debugPrintf(const char *format, ...)
{
    char buffer[DEBUG_PRINTF_BUFFER_SIZE];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    for (char *p = buffer; *p != '\0'; ++p)
    {
    	stdUtil_putChar(*p);
    }
}

#endif /* STDUTIL_H_ */

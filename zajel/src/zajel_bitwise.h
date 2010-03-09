/***************************************************************************************************
 *
 * zajel - an embedded communication framework for multi-threaded/multi-core environment.
 *
 * Copyright © 2009  Mohamed Galal El-Din, Karim Emad Morsy.
 *
 ***************************************************************************************************
 *
 * This file is part of zajel library.
 *
 * zajel is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 * zajel is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with zajel. If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************************************
 *
 * For more information, questions, or inquiries please contact:
 *
 * Mohamed Galal El-Din:    mohamed.g.ebrahim@gmail.com
 * Karim Emad Morsy:        karim.e.morsy@gmail.com
 *
 **************************************************************************************************/
#ifndef ZAJEL_BITWISE_H_
#define ZAJEL_BITWISE_H_


/***************************************************************************************************
 *
 *  M A C R O S
 *
 **************************************************************************************************/

/***************************************************************************************************
 *  Macro Name  : BITWISE_BITS_TO_BYTES_FLOOR
 *
 *  Arguments   : bits
 *
 *  Description : This macro converts bits to bytes (simply divide by 8).
 *
 *  Returns     : Floored number of bytes in given bits (9 bits will return 1 byte).
 **************************************************************************************************/
#define BITWISE_BITS_TO_BYTES_FLOOR(bits) ((bits) >> 3)

/***************************************************************************************************
 *  Macro Name  : BITWISE_BITS_TO_BYTES
 *
 *  Arguments   : bits
 *
 *  Description : This macro converts bytes to bits (simply multiply by 8).
 *
 *  Returns     : Number of bits.
 **************************************************************************************************/
#define BITWISE_BYTES_TO_BITS(bytes) ((bytes) << 3)

/***************************************************************************************************
 *  Macro Name  : BITWISE_CLEAR_ALL
 *
 *  Arguments   : byteShortWordOrDword
 *
 *  Description : This macro takes a bitmap of any primitive type, and reset it.
 *
 *  Returns     : None.
 **************************************************************************************************/
#define BITWISE_CLEAR_ALL(byteShortWordOrDword) ((byteShortWordOrDword) = 0)

/***************************************************************************************************
 *  Macro Name  : BITWISE_BYTE_SET_BIT
 *
 *  Arguments   : byteShortWordOrDword, bitIndex
 *
 *  Description : Sets bit located at [bitIndex] in the given [byteShortWordOrDword]. bitIndex zero
 *                refers to LSB and bitIndex 7, 15, 31, and 63  refers to the MSB in byte, short,
 *                word, and double word respectively.
 *
 *  Returns     : None.
 **************************************************************************************************/
#define BITWISE_BYTE_SET_BIT(byteShortWordOrDword, bitIndex)                                       \
{                                                                                                  \
    ASSERT((BITWISE_BITS2BYTES(bitIndex) < sizeof(byteShortWordOrDword)),                          \
           "bitwise: bit index is out of range!",                                                  \
           __FILE__,                                                                               \
           __LINE__);                                                                              \
    (byteShortWordOrDword) |= ((bitIndex) << 1);                                                   \
}

/***************************************************************************************************
 *  Macro Name  : BITWISE_BYTE_CLEAR_BIT
 *
 *  Arguments   : byteShortWordOrDword, bitIndex
 *
 *  Description : Clears bit located at [bitIndex] in the given [byteShortWordOrDword]. bitIndex
 *                zero refers to LSB and bitIndex 7, 15, 31, and 63  refers to the MSB in byte,
 *                short, word, and double word respectively.
 *
 *  Returns     : None.
 **************************************************************************************************/
#define BITWISE_BYTE_CLEAR_BIT(byteShortWordOrDword, bitIndex)                                     \
{                                                                                                  \
    ASSERT((BITWISE_BITS2BYTES(bitIndex) < sizeof(byteShortWordOrDword)),                          \
           "bitwise: bit index is out of range!",                                                  \
           __FILE__,                                                                               \
           __LINE__);                                                                              \
    (byteShortWordOrDword) &= ~((bitIndex) << 1);                                                  \
}

/***************************************************************************************************
 *  Macro Name  : BITWISE_BYTE_TOGGLE_BIT
 *
 *  Arguments   : byteShortWordOrDword, bitIndex
 *
 *  Description : Toggles bit located at [bitIndex] in the given [byteShortWordOrDword]. bitIndex
 *                zero refers to LSB and bitIndex 7, 15, 31, and 63  refers to the MSB in byte,
 *                short, word, and double word respectively.
 *
 *  Returns     : None.
 **************************************************************************************************/
#define BITWISE_BYTE_TOGGLE_BIT(byteShortWordOrDword, bitIndex)                                    \
{                                                                                                  \
    ASSERT((BITWISE_BITS2BYTES(bitIndex) < sizeof(byteShortWordOrDword)),                          \
           "bitwise: bit index is out of range!",                                                  \
           __FILE__,                                                                               \
           __LINE__);                                                                              \
    (byteShortWordOrDword) ^= ((bitIndex) << 1);                                                   \
}

/***************************************************************************************************
 *
 *  T Y P E S
 *
 **************************************************************************************************/


/***************************************************************************************************
 *
 *  I N T E R F A C E   F U N C T I O N   D E C L A R A T I O N S
 *
 **************************************************************************************************/


#endif /* ZAJEL_BITWISE_H_ */

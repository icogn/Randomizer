/**	@file clr0.cpp
 *  @brief Handles reading color data from the CLR0 chunk of the seed.
 *
 *	@author Isaac
 *	@bug No known bugs.
 */

#include "rando/clr0.h"

namespace mod::rando
{
    // Maps nibble value to how many bits it has set.
    uint8_t NIBBLE_LOOKUP[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

    // Maps a bit index to a bit mask which can be used to clear all bits but
    // the ones to the right of the given bit index.
    uint8_t BITS_TO_RIGHT_MASK[8] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f };

    // bitIndex is one of 0,1,2,3,4,5,6,7. 0 means the least significant bit.
    // Given a bit index, returns the number of bits which have a lower index and are set.
    inline uint8_t countSetBitsToRight( uint8_t byte, uint8_t bitIndex )
    {
        if ( bitIndex == 0 )
        {
            return 0;
        }

        uint8_t maskedByte = byte & BITS_TO_RIGHT_MASK[bitIndex];

        uint8_t result = NIBBLE_LOOKUP[maskedByte & 0x0f];

        if ( bitIndex >= 5 )
        {
            result += NIBBLE_LOOKUP[maskedByte >> 4];
        }

        return result;
    }

    uint8_t* CLR0::getRecolorRgb( RecolorId recolorId )
    {
        if ( recolorId > this->maxRecolorId || this->dataFormat != DataFormat::BASIC )
        {
            // RecolorId is guaranteed to not have any data in the CLR0 chunk or
            // the dataFormat is not supported.
            return nullptr;
        }

        uint8_t* thisAddr = reinterpret_cast<uint8_t*>( this );
        uint8_t* bitTable = thisAddr + this->bitTableOffset;

        uint8_t tableIndex = recolorId >> 3;
        uint8_t bitIndex = recolorId & 0x7;
        uint8_t bitTableEntry = bitTable[tableIndex];

        if ( ( bitTableEntry & ( 1 << bitIndex ) ) == 0 )
        {
            // This RecolorId is not enabled, so return nullptr.
            return nullptr;
        }

        // Find index in RGB table.
        uint16_t* cummulativeSums = reinterpret_cast<uint16_t*>( thisAddr + this->cummulativeSumsOffset );
        uint16_t rgbTableIndex = cummulativeSums[tableIndex] + countSetBitsToRight( bitTableEntry, bitIndex );

        uint8_t* colorPtr = thisAddr + this->rbgDataOffset + ( rgbTableIndex * 3 );

        // Check that color is within bounds of the CLR0 chunk. Each color is 3
        // bytes long, so 3 bytes after the colorPtr should be at most the start
        // of the next seed chunk.
        if ( colorPtr + 3 <= thisAddr + this->size )
        {
            return colorPtr;
        }

        return nullptr;
    }
}     // namespace mod::rando
/**	@file clr0.h
 *  @brief Handles reading recoloring data from the seed data.
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#ifndef RANDO_CLR0_H
#define RANDO_CLR0_H

#include <cstdint>

namespace mod::rando
{
    // Each RecolorId represents a thing or group of things which are considered
    // a single unit and which should all be recolored using a specific color
    // when the user says "I want this thing to be this color". Basically, a
    // single RecolorId does NOT equal a single texture.
    // - Intentionally uint16_t (going over 0xFF is not impossible, and the CLR0
    //   chunk was designed to support up to 0xFFFF).
    enum RecolorId : uint16_t
    {
        HerosClothes = 0x00,     // Cap and Body
        ZoraArmorPrimary = 0x01,
        ZoraArmorSecondary = 0x02,
        ZoraArmorHelmet = 0x03,
    };

    class CLR0
    {
       private:
        enum DataFormat : uint8_t
        {
            NO_DATA = 0x00,
            BASIC = 0x01,
            // Should be able to add to this if need to restructure the
            // post-header data in the future in a backwards-compatible way.
            // Basically just a "recoloring enabled" boolean at the moment
            // though. If the user didn't enable any recoloring, the CLR0 chunk
            // should be the following 0x10 bytes:
            // 43 4C 52 30 00 00 00 10 00 00 00 00 00 00 00 00
        };

        // Should always be "CLR0".
        char magic[4];
        // Total byte size of the entire CLR0 chunk including the header and any
        // padding.
        uint32_t size;
        // Any RecolorId larger than this is guaranteed to not have any RGB
        // values in the data. Generated seed should set this value as low as
        // possible based on selected user settings.
        uint16_t maxRecolorId;
        // Indicates what format of the data after the header. 0x00 means there
        // are no rgb values in the chunk.
        DataFormat dataFormat;
        // Offset to bitTable section relative to start of header. (This is also
        // the header size: 0x10)
        uint8_t bitTableOffset;
        // Offset to cummulativeSums section relative to start of header.
        uint16_t cummulativeSumsOffset;
        // Offset to rgbData section relative to start of header.
        uint16_t rbgDataOffset;

       public:
        /**
         * @brief If the RecolorId has data in the CLR0 chunk, returns a pointer
         * to a 3 byte array (R,G,B). Else returns nullptr (meaning recolor
         * should not occur).
         *
         * @return uint8_t* Pointer to 3 bytes (R,G,B), or nullptr.
         */
        uint8_t* getRecolorRgb( RecolorId );
    };

}     // namespace mod::rando
#endif
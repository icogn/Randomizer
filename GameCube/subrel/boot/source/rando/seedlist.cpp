/**	@file seedselector.cpp
 *  @brief Selection tool to select the desired data-GCI
 *
 *	@author AECX
 *	@bug No known bugs.
 */
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cinttypes>

#ifdef DVD
#include "gc_wii/dvd.h"
#else
#include "gc_wii/card.h"
#endif

#include "rando/seedlist.h"
#include "main.h"
#include "rando/data.h"
#include "rando/seed.h"
#include "tools.h"
#include "memory.h"
#include "cxx.h"

namespace mod::rando
{
    SeedList::SeedList()
    {
        using namespace libtp;

        // Loop through possible seed-data-GCIs until we don't read anything
        char fileName[32];
        libtp::memory::clearMemory( fileName, sizeof( fileName ) );

        // Use snprintf to get the index of the end of the string
#ifdef DVD
        const int32_t fileNumIndex = snprintf( fileName, sizeof( fileName ), "/mod/rando-data" );
#else
        const int32_t fileNumIndex = snprintf( fileName, sizeof( fileName ), "rando-data" );
#endif
        // Bitwise representation of the seeds available
        uint16_t seedIDX = 0;

        // Store header data so we don't have to open the files again later
        // Allocate the memory to the back of the heap to avoid fragmentation
        // Align to uint64_t, as it is the largest variable type used in the Header struct
        Header* headerBuffer = new ( -sizeof( uint64_t ) ) Header[SEED_MAX_ENTRIES];

        m_numSeeds = 0;
#ifndef DVD
        // The memory card should already be mounted
        constexpr int32_t memCardChan = CARD_SLOT_A;
#endif
        for ( uint8_t i = 0; i < SEED_MAX_ENTRIES; i++ )
        {
            // Check next filename depending on i
            // rando-data0, rando-data1, ...
            fileName[fileNumIndex] = static_cast<char>( '0' + i );

            Header header;
#ifdef DVD
            if ( DVD_STATE_END == libtp::tools::ReadFile( fileName, sizeof( header ), 0, &header ) )
#else
            if ( CARD_RESULT_READY ==
                 libtp::tools::ReadGCIMounted( memCardChan, fileName, sizeof( header ), 0, &header, true ) )
#endif
            {
                const uint16_t versionMajor = header.versionMajor;

                if ( CHECK_MIN_SUPPORTED_SEED_DATA_VER_MAJOR( versionMajor ) &&
                     CHECK_MIN_SUPPORTED_SEED_DATA_VER_MINOR( header.versionMinor ) &&
                     CHECK_MAX_FULLY_SUPPORTED_SEED_DATA_VER_MAJOR( versionMajor ) )
                {
                    seedIDX = seedIDX | ( 1 << i );
                    memcpy( &headerBuffer[i], &header, sizeof( Header ) );
                    m_numSeeds++;
                }
            }
        }

        if ( m_numSeeds > 0 )
        {
            // Align to uint64_t, as it is the largest variable type used in the Header struct
            m_seedInfo = new ( sizeof( uint64_t ) ) SeedInfo[m_numSeeds];
            uint8_t j = 0;     // seedInfo index

            // Loop through all possible seeds and load them into our seedInfo
            for ( uint8_t i = 0; i < SEED_MAX_ENTRIES; i++ )
            {
                if ( ( ( seedIDX >> i ) & 1 ) == 1 )
                {
                    memcpy( &m_seedInfo[j].header, &headerBuffer[i], sizeof( Header ) );
                    m_seedInfo[j].fileIndex = i;
                    j++;
                }
            }
        }

        // getConsole() << static_cast<int32_t>( m_numSeeds ) << " seed(s) available.\n";
        delete[] headerBuffer;
    }

    SeedList::~SeedList()
    {
        delete[] m_seedInfo;
    }

}     // namespace mod::rando
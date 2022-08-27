/**	@file seedlist2.h
 *  @brief Stores list of seeds
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#ifndef RANDO_SEEDLIST2_H
#define RANDO_SEEDLIST2_H

#include <cstdint>

#include "util/cardUtils.h"

namespace mod::rando
{
    enum SeedListEntryStatus : uint8_t
    {
        FULLY_SUPPORTED,
        PARTIALLY_SUPPORTED,
        NOT_SUPPORTED,
        VERSION_UNKNOWN
    };

    class SeedListEntry
    {
       public:
        void updateFromDirectoryEntry( libtp::util::card::DirectoryEntry& dirEntry );

        uint16_t verMajor() { return m_verMajor; }
        uint16_t verMinor() { return m_verMinor; }
        const char* filename() { return m_filename; }
        const char* playthroughName() { return &m_filename[8]; }
        SeedListEntryStatus status() { return m_status; }

       private:
        uint16_t m_verMajor;
        uint16_t m_verMinor;
        char m_filename[33];
        SeedListEntryStatus m_status;
    };

    class SeedList2
    {
       public:
        static SeedList2* fromDirectoryEntries( libtp::util::card::DirectoryEntry* dirEntries, int count );

        ~SeedList2( void );

       private:
        int count;
        SeedListEntry* entries;

        SeedList2( SeedListEntry* entries, int count );

        // SeedInfo FindSeed( uint64_t seed );

        // uint8_t m_numSeeds;
        // uint8_t m_selectedSeed;

        // SeedInfo* m_seedInfo = nullptr;
    };

    // int abc();

}     // namespace mod::rando

#endif
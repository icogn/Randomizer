#include <cstdint>

#include "game_patch/game_patch.h"
#include "user_patch/user_patch.h"
#include "tp/d_item.h"
#include "tp/d_msg_flow.h"
#include "tp/d_a_alink.h"
#include "data/items.h"
#include "tp/d_menu_collect.h"
#include "patch.h"
#include "events.h"
#include "tp/d_kankyo_rain.h"
#include "asm.h"
#include "tp/d_event.h"

namespace mod::game_patch
{
    void _06_writeASMPatches()
    {
        // Get the addresses to overwrite
        uint32_t woodenSwordFunctionAddress = reinterpret_cast<uint32_t>( libtp::tp::d_item::item_func_WOOD_STICK );
        uint32_t heartContainerFunctionAddress = reinterpret_cast<uint32_t>( libtp::tp::d_item::item_func_UTUWA_HEART );
        uint32_t event035MemoAddress = reinterpret_cast<uint32_t>( libtp::tp::d_msg_flow::event035 );
        uint32_t procCoGetItemAddress = reinterpret_cast<uint32_t>( libtp::tp::d_a_alink::procCoGetItem );
        uint32_t screenSetAddress = reinterpret_cast<uint32_t>( libtp::tp::d_menu_collect::dMenuCollect_screenSet );
        uint32_t odourDrawAddress = reinterpret_cast<uint32_t>( libtp::tp::d_kankyo_rain::dKyr_odour_draw );
        uint32_t skipperFunctionAddress = reinterpret_cast<uint32_t>( libtp::tp::d_event::skipper );

#ifdef TP_US
        uint32_t* enableCrashScreen = reinterpret_cast<uint32_t*>( 0x8000B8A4 );
        uint32_t* patchMessageCalculation = reinterpret_cast<uint32_t*>( 0x80238F58 );
        uint32_t* mDoDvdThd_mountArchive_c__execute_setOffset0xcTo1 = reinterpret_cast<uint32_t*>( 0x800160DC );
#elif defined TP_EU
        uint32_t* enableCrashScreen = reinterpret_cast<uint32_t*>( 0x8000B878 );
        uint32_t* patchMessageCalculation = reinterpret_cast<uint32_t*>( 0x802395D8 );
        uint32_t* mDoDvdThd_mountArchive_c__execute_setOffset0xcTo1 = reinterpret_cast<uint32_t*>( 0x80016090 );
#elif defined TP_JP
        uint32_t* enableCrashScreen = reinterpret_cast<uint32_t*>( 0x8000B8A4 );
        uint32_t* patchMessageCalculation = reinterpret_cast<uint32_t*>( 0x802398E0 );
        uint32_t* mDoDvdThd_mountArchive_c__execute_setOffset0xcTo1 = reinterpret_cast<uint32_t*>( 0x800160DC );
#endif

        // Perform the overwrites

        /* If the address is loaded into the cache before the overwrite is made,
        then the cache will need to be cleared after the overwrite */

        // Enable the crash screen
        *enableCrashScreen = ASM_BRANCH( 0x14 );

        // Nop out the instruction that causes a miscalculation in message resources.
        *patchMessageCalculation = ASM_NOP;

        // Modify the skipper function to check whether or not a cutscene is skippable instead of whether the player skips the
        // CS. This effectively auto-skips all skippable cutscenes.
        *reinterpret_cast<uint32_t*>( skipperFunctionAddress + 0x54 ) = 0x281e0000;     // Previous 0x4540004e7

        // Modify the Wooden Sword function to not set a region flag by default by nopping out the function call to isSwitch
        *reinterpret_cast<uint32_t*>( woodenSwordFunctionAddress + 0x40 ) = ASM_NOP;     // Previous 0x4bf9cafd

        // Modify dKyr_odour_draw to draw the Reekfish path so long as we have smelled the fish once.
        libtp::patch::writeBranchBL( reinterpret_cast<void*>( odourDrawAddress + 0xBC ),
                                     reinterpret_cast<void*>( assembly::asmShowReekfishPath ) );

        // Modify the Heart Container function to not set the dungeon flag for the heart container upon collection
        *reinterpret_cast<uint32_t*>( heartContainerFunctionAddress + 0x7C ) = ASM_NOP;     // Previous 0x4bf9c5e9

        // Modify event035 to not remove Auru's Memo from inventory after talking to Fyer.
        *reinterpret_cast<uint32_t*>( event035MemoAddress + 0x40 ) =
            ASM_COMPARE_WORD_IMMEDIATE( 4, libtp::data::items::Asheis_Sketch );     // Previous 0x2c040090

        // Modify procCoGetItem to display the 20 and 60 poe messages when the player currently has 19 and 59 respectively, as
        // this project changes the poe count to increment after the message is displayed instead of before
        *reinterpret_cast<uint32_t*>( procCoGetItemAddress + 0x56C ) = ASM_COMPARE_LOGICAL_WORD_IMMEDIATE( 0, 19 );
        *reinterpret_cast<uint32_t*>( procCoGetItemAddress + 0x580 ) = ASM_COMPARE_LOGICAL_WORD_IMMEDIATE( 0, 59 );

        // Modify getRupeeMax calls in screenSet to display the proper wallet in the pause menu
        libtp::patch::writeBranchBL( reinterpret_cast<void*>( screenSetAddress + 0xDCC ),
                                     reinterpret_cast<void*>( events::getPauseRupeeMax ) );

        libtp::patch::writeBranchBL( reinterpret_cast<void*>( screenSetAddress + 0xDF0 ),
                                     reinterpret_cast<void*>( events::getPauseRupeeMax ) );

        // Prep for hooking into RARC loading. Prevent setting offset 0xC to 1
        // which would indicate to another thread which is polling this byte
        // that the loading has finished and it can do its thing. Instead, put
        // r26 (this) into r4. We will hook into the instruction 8 bytes later
        // so that we can do any necessary transforms to the loaded RARC data
        // before setting offset 0xC to 1 to indicate that loading has finished.
        *mDoDvdThd_mountArchive_c__execute_setOffset0xcTo1 = 0x7f44d378;     // or r4, r26, r26
    }
}     // namespace mod::game_patch
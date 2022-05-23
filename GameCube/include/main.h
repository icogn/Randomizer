
/**	@file main.h
 *  @brief Main structure of the Randomizer
 *
 *	@author AECX
 *	@bug No known bugs.
 */

#ifndef MAIN_H
#define MAIN_H
#include <cstdint>

#include "display/console.h"
#include "rando/randomizer.h"
#include "rando/seedlist.h"
#include "tp/d_save.h"
#include "tp/dynamic_link.h"
#include "tp/dzx.h"
#include "tp/J2DPicture.h"

// Game States
#define GAME_BOOT 0       // Default; At startup (should only ever be active once)
#define GAME_TITLE 1      // When user is on title screen; let them choose their seed etc.
#define GAME_ACTIVE 2     // When user is ingame and rando should be active

namespace mod
{
    // General public objects
    extern libtp::display::Console console;
    extern rando::Randomizer* randomizer;
    extern rando::SeedList* seedList;

    // Variables
    extern libtp::tp::J2DPicture::J2DPicture* bgWindow;
    extern uint32_t lastButtonInput;
    extern int32_t lastLoadingState;
    extern uint32_t nextVal;
    extern bool consoleState;
    extern uint8_t gameState;
    extern bool ringDrawnThisFrame;

    void main();
    void hookFunctions();
    void setScreen( bool state );     // Sets visibility of console
    uint32_t rand( uint32_t* seed );
    uint32_t ulRand( uint32_t range );

    void handleInput( uint32_t inputs );
    
    // Inline randoIsEnabled, as it's short enough to use less memory when inlined
    inline bool randoIsEnabled(rando::Randomizer* rando)
    {
        if (!rando)
        {
            return false;
        }
        
        return rando->m_Enabled;
    }

    // Function hook handlers & trampolines
    void handle_fapGm_Execute( void );
    extern void ( *return_fapGm_Execute )( void );

    // DMC (REL) Hook
    extern bool ( *return_do_Link )( libtp::tp::dynamic_link::DynamicModuleControl* dmc );

    // DZX Functions; Handler is lambda -> randomizer::onDZX();
    extern bool ( *return_actorInit )( void* mStatus_roomControl,
                                       libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                       int32_t unk3,
                                       void* unk4 );

    extern bool ( *return_actorInit_always )( void* mStatus_roomControl,
                                              libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                              int32_t unk3,
                                              void* unk4 );

    extern bool ( *return_actorCommonLayerInit )( void* mStatus_roomControl,
                                                  libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                                  int32_t unk3,
                                                  void* unk4 );

    extern int ( *return_getLayerNo_common_common )( const char* stageName, int32_t roomId, int32_t layerOverride );

    extern int32_t ( *return_createItemForBoss )( const float pos[3],
                                                  int32_t item,
                                                  int32_t roomNo,
                                                  const int16_t rot[3],
                                                  const float scale[3],
                                                  float unk6,
                                                  float unk7,
                                                  int32_t parameters );

    extern bool ( *return_query022 )( void* unk1, void* unk2, int32_t unk3 );
    extern bool ( *return_query023 )( void* unk1, void* unk2, int32_t unk3 );

    extern bool ( *return_query042 )( void* unk1, void* unk2, int32_t unk3 );

    extern bool ( *return_isDungeonItem )( libtp::tp::d_save::dSv_memBit_c* memBitPtr, const int memBit );
    extern void* Z2ScenePtr;
    extern bool isFoolishTrapQueued;
}     // namespace mod
#endif
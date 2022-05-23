#include "main.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cinttypes>

#include "Z2AudioLib/Z2AudioMgr.h"
#include "data/items.h"
#include "data/stages.h"
#include "events.h"
#include "game_patch/game_patch.h"
#include "gc_wii/OSTime.h"
#include "patch.h"
#include "rando/data.h"
#include "rando/randomizer.h"
#include "rando/seedlist.h"
#include "tools.h"
#include "tp/J2DPicture.h"
#include "tp/JKRArchive.h"
#include "tp/JKRDvdRipper.h"
#include "tp/control.h"
#include "tp/d_a_alink.h"
#include "tp/d_a_player.h"
#include "tp/d_com_inf_game.h"
#include "tp/d_item.h"
#include "tp/d_kankyo.h"
#include "tp/d_menu_ring.h"
#include "tp/d_menu_window.h"
#include "tp/d_meter2_info.h"
#include "tp/d_msg_class.h"
#include "tp/d_msg_flow.h"
#include "tp/d_msg_object.h"
#include "tp/d_save.h"
#include "tp/d_stage.h"
#include "tp/dzx.h"
#include "tp/f_ap_game.h"
#include "tp/f_op_actor_mng.h"
#include "tp/f_op_scene_req.h"
#include "tp/f_pc_node_req.h"
#include "tp/m_do_controller_pad.h"
#include "tp/resource.h"
#include "tp/d_s_logo.h"

namespace mod
{
    // Bind extern global variables
    libtp::display::Console console( 9 );
    rando::Randomizer* randomizer = nullptr;
    rando::SeedList* seedList = nullptr;

    // Variables
    libtp::tp::J2DPicture::J2DPicture* bgWindow = nullptr;
    uint32_t lastButtonInput = 0;
    int32_t lastLoadingState = 0;
    bool consoleState = true;
    uint8_t gameState = GAME_BOOT;
    bool ringDrawnThisFrame = false;
    void* Z2ScenePtr = nullptr;
    bool isFoolishTrapQueued = false;
    uint32_t nextVal = libtp::gc_wii::os_time::OSGetTick();

    // Function hook return trampolines
    void ( *return_fapGm_Execute )( void ) = nullptr;
    void ( *return_setLineUpItem )( libtp::tp::d_save::dSv_player_item_c* ) = nullptr;
    bool ( *return_do_Link )( libtp::tp::dynamic_link::DynamicModuleControl* dmc ) = nullptr;
    bool ( *return_do_unlink )( libtp::tp::dynamic_link::DynamicModuleControl* dmc ) = nullptr;

    // DZX trampolines
    bool ( *return_actorInit )( void* mStatus_roomControl,
                                libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                int32_t unk3,
                                void* unk4 ) = nullptr;

    bool ( *return_actorInit_always )( void* mStatus_roomControl,
                                       libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                       int32_t unk3,
                                       void* unk4 ) = nullptr;

    bool ( *return_actorCommonLayerInit )( void* mStatus_roomControl,
                                           libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                           int32_t unk3,
                                           void* unk4 ) = nullptr;

    // GetLayerNo trampoline
    int32_t ( *return_getLayerNo_common_common )( const char* stageName, int32_t roomId, int32_t layerOverride ) = nullptr;

    // Used in replacing Heart Containers
    int32_t ( *return_createItemForBoss )( const float pos[3],
                                           int32_t item,
                                           int32_t roomNo,
                                           const int16_t rot[3],
                                           const float scale[3],
                                           float unk6,
                                           float unk7,
                                           int32_t parameters ) = nullptr;

    int32_t ( *return_createItemForPresentDemo )( const float pos[3],
                                                  int32_t item,
                                                  uint8_t unk3,
                                                  int32_t unk4,
                                                  int32_t unk5,
                                                  const float unk6[3],
                                                  const float unk7[3] ) = nullptr;

    int32_t ( *return_createItemForTrBoxDemo )( const float pos[3],
                                                int32_t item,
                                                int32_t itemPickupFlag,
                                                int32_t roomNo,
                                                const int16_t rot[3],
                                                const float scale[3] ) = nullptr;
    int32_t ( *return_createItemForMidBoss )( const float pos[3],
                                              int32_t item,
                                              int32_t roomNo,
                                              const int16_t rot[3],
                                              const float scale[3],
                                              int32_t unk6,
                                              int32_t itemPickupFlag ) = nullptr;

    int32_t ( *return_execItemGet )( uint8_t item ) = nullptr;

    int32_t ( *return_checkItemGet )( uint8_t item, int32_t defaultValue ) = nullptr;

    bool ( *return_setMessageCode_inSequence )( libtp::tp::control::TControl* control,
                                                const void* TProcessor,
                                                uint16_t unk3,
                                                uint16_t msgId ) = nullptr;

    uint32_t ( *return_getFontCCColorTable )( uint8_t colorId, uint8_t unk ) = nullptr;
    uint32_t ( *return_getFontGCColorTable )( uint8_t colorId, uint8_t unk ) = nullptr;

    char ( *return_parseCharacter_1Byte )( const char** text ) = nullptr;

    bool ( *return_query022 )( void* unk1, void* unk2, int32_t unk3 ) = nullptr;
    bool ( *return_query023 )( void* unk1, void* unk2, int32_t unk3 ) = nullptr;
    bool ( *return_query025 )( void* unk1, void* unk2, int32_t unk3 ) = nullptr;
    bool ( *return_query042 )( void* unk1, void* unk2, int32_t unk3 ) = nullptr;
    int ( *return_query004 )( void* unk1, void* unk2, int32_t unk3 ) = nullptr;
    int ( *return_query037 )( void* unk1, void* unk2, int32_t unk3 ) = nullptr;

    bool ( *return_checkTreasureRupeeReturn )( void* unk1, int32_t item ) = nullptr;

    void ( *return_collect_save_open_init )( uint8_t param_1 ) = nullptr;

    bool ( *return_isDungeonItem )( libtp::tp::d_save::dSv_memBit_c* memBitPtr, const int memBit ) = nullptr;

    bool ( *return_chkEvtBit )( uint32_t flag ) = nullptr;

    bool ( *return_isEventBit )( libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag ) = nullptr;
    void ( *return_onEventBit )( libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag ) = nullptr;

    bool ( *return_isSwitch_dSv_memBit )( libtp::tp::d_save::dSv_memBit_c* memoryBit, int flag ) = nullptr;
    void ( *return_onSwitch_dSv_memBit )( libtp::tp::d_save::dSv_memBit_c* memoryBit, int flag ) = nullptr;
    uint32_t ( *return_event000 )( void* messageFlow, void* nodeEvent, void* actrPtr ) = nullptr;
    int ( *return_event003 )( void* messageFlow, void* nodeEvent, void* actrPtr ) = nullptr;
    int ( *return_event041 )( void* messageFlow, void* nodeEvent, void* actrPtr ) = nullptr;

    int32_t ( *return_tgscInfoInit )( void* stageDt, void* i_data, int32_t entryNum, void* param_3 ) = nullptr;

    bool ( *return_checkBootsMoveAnime )( libtp::tp::d_a_alink::daAlink* d_a_alink, int param_1 ) = nullptr;

    void ( *return_roomLoader )( void* data, void* stageDt, int roomNo ) = nullptr;

    bool ( *return_checkDamageAction )( libtp::tp::d_a_alink::daAlink* linkMapPtr ) = nullptr;

    void ( *return_loadSeWave )( void* Z2SceneMgr, uint32_t waveID ) = nullptr;

    void ( *return_setGetItemFace )( libtp::tp::d_a_alink::daAlink* daALink, uint16_t itemID ) = nullptr;
    void ( *return_dMenuRing__draw )( void* dMenuRing ) = nullptr;
    void* ( *return_dScnLogo_c_dt )( void* dScnLogo_c, int16_t bFreeThis ) = nullptr;

    void main()
    {
        // Run game patches
        game_patch::_00_poe();
        game_patch::_02_modifyItemData();
        game_patch::_03_increaseClimbSpeed();
        game_patch::_06_writeASMPatches();

        // Display some info
        console << "Welcome to TPR!\n"
                << "(C) AECX, Lunar Soap, Zephiles\n\n"
                << "Note:\n"
                << "Please avoid [re]starting rando unnecessarily\n"
                << "on ORIGINAL HARDWARE as it wears down your\n"
                << "Memory Card!\n"
                << "Press R + Z to close the console.\n"
                << "Press X/Y to select a seed.\n\n";

        // Generate our seedList
        seedList = new rando::SeedList();

        // Just hook functions for now
        hookFunctions();
    }

    void hookFunctions()
    {
        using namespace libtp;
        using namespace libtp::tp::d_stage;
        using namespace libtp::tp::dzx;
        using namespace libtp::tp::d_com_inf_game;
        // Hook functions
        return_fapGm_Execute = patch::hookFunction( libtp::tp::f_ap_game::fapGm_Execute, mod::handle_fapGm_Execute );

        // DMC
        return_do_Link = patch::hookFunction( libtp::tp::dynamic_link::do_link,
                                              []( libtp::tp::dynamic_link::DynamicModuleControl* dmc )
                                              {
                                                  // Call the original function immediately, as the REL file needs to be linked
                                                  // before applying patches
                                                  const bool result = return_do_Link( dmc );

                                                  if ( result && dmc->moduleInfo )
                                                  {
                                                      events::onRELLink( randomizer, dmc );
                                                  }

                                                  return result;
                                              } );

        return_do_unlink = patch::hookFunction( libtp::tp::dynamic_link::do_unlink,
                                                []( libtp::tp::dynamic_link::DynamicModuleControl* dmc )
                                                {
                                                    events::onRELUnlink( randomizer, dmc );

                                                    return return_do_unlink( dmc );
                                                } );

        // DZX
        return_actorInit =
            patch::hookFunction( actorInit,
                                 []( void* mStatus_roomControl, ChunkTypeInfo* chunkTypeInfo, int32_t unk3, void* unk4 )
                                 {
                                     events::onDZX( mod::randomizer, chunkTypeInfo );
                                     return return_actorInit( mStatus_roomControl, chunkTypeInfo, unk3, unk4 );
                                 } );

        return_actorInit_always =
            patch::hookFunction( actorInit_always,
                                 []( void* mStatus_roomControl, ChunkTypeInfo* chunkTypeInfo, int32_t unk3, void* unk4 )
                                 {
                                     events::onDZX( mod::randomizer, chunkTypeInfo );
                                     return return_actorInit_always( mStatus_roomControl, chunkTypeInfo, unk3, unk4 );
                                 } );

        return_actorCommonLayerInit =
            patch::hookFunction( actorCommonLayerInit,
                                 []( void* mStatus_roomControl, ChunkTypeInfo* chunkTypeInfo, int32_t unk3, void* unk4 )
                                 {
                                     events::onDZX( mod::randomizer, chunkTypeInfo );
                                     events::loadCustomRoomActors();
                                     return return_actorCommonLayerInit( mStatus_roomControl, chunkTypeInfo, unk3, unk4 );
                                 } );

        return_tgscInfoInit = patch::hookFunction( tgscInfoInit,
                                                   []( void* stageDt, void* i_data, int32_t entryNum, void* param_3 )
                                                   {
                                                       events::loadCustomRoomSCOBs();
                                                       return return_tgscInfoInit( stageDt, i_data, entryNum, param_3 );
                                                   } );

        // Custom States
        return_getLayerNo_common_common =
            patch::hookFunction( getLayerNo_common_common,
                                 []( const char* stageName, int32_t roomId, int32_t layerOverride )
                                 { return game_patch::_01_getLayerNo( stageName, roomId, layerOverride ); } );

        // Custom Item Messages
        return_setMessageCode_inSequence = patch::hookFunction(
            libtp::tp::control::setMessageCode_inSequence,
            []( libtp::tp::control::TControl* control, const void* TProcessor, uint16_t unk3, uint16_t msgId )
            {
                // Call the original function immediately, as a lot of stuff needs to be set before our code runs
                const bool ret = return_setMessageCode_inSequence( control, TProcessor, unk3, msgId );

                // Make sure the function ran successfully
                if ( !ret )
                {
                    return ret;
                }
                game_patch::_05_setCustomItemMessage( control, TProcessor, unk3, msgId, randomizer );
                return ret;
            } );

        // Set custom font color
        return_getFontCCColorTable = patch::hookFunction( tp::d_msg_class::getFontCCColorTable,
                                                          []( uint8_t colorId, uint8_t unk )
                                                          {
                                                              if ( colorId >= 0x9 )
                                                              {
                                                                  return game_patch::_05_getCustomMsgColor( colorId );
                                                              }
                                                              else
                                                              {
                                                                  return return_getFontCCColorTable( colorId, unk );
                                                              }
                                                          } );
        // Set custom font color
        return_getFontGCColorTable = patch::hookFunction( tp::d_msg_class::getFontGCColorTable,
                                                          []( uint8_t colorId, uint8_t unk )
                                                          {
                                                              if ( colorId >= 0x9 )
                                                              {
                                                                  return game_patch::_05_getCustomMsgColor( colorId );
                                                              }
                                                              else
                                                              {
                                                                  return return_getFontCCColorTable( colorId, unk );
                                                              }
                                                          } );

        return_collect_save_open_init = patch::hookFunction( tp::d_menu_window::collect_save_open_init,
                                                             []( uint8_t param_1 )
                                                             {
                                                                 game_patch::_07_checkPlayerStageReturn();
                                                                 return return_collect_save_open_init( param_1 );
                                                             } );

        return_parseCharacter_1Byte = patch::hookFunction( tp::resource::parseCharacter_1Byte,
                                                           []( const char** text )
                                                           {
                                                               game_patch::_05_replaceMessageString( text );
                                                               return return_parseCharacter_1Byte( text );
                                                           } );

        // Replace the Item that spawns when a boss is defeated
        return_createItemForBoss =
            patch::hookFunction( libtp::tp::f_op_actor_mng::createItemForBoss,
                                 []( const float pos[3],
                                     int32_t item,
                                     int32_t roomNo,
                                     const int16_t rot[3],
                                     const float scale[3],
                                     float unk6,
                                     float unk7,
                                     int32_t parameters )
                                 {
                                     // Spawn the appropriate item with model
                                     uint8_t itemID = randomizer->getBossItem();
                                     itemID = game_patch::_04_verifyProgressiveItem( mod::randomizer, itemID );
                                     uint32_t params = 0xFF0000 | ( parameters & 0xFF ) << 0x8 | ( itemID & 0xFF );
                                     return tp::f_op_actor_mng::fopAcM_create( 539, params, pos, roomNo, rot, scale, -1 );
                                 } );

        return_createItemForMidBoss =
            patch::hookFunction( libtp::tp::f_op_actor_mng::createItemForMidBoss,
                                 []( const float pos[3],
                                     int32_t item,
                                     int32_t roomNo,
                                     const int16_t rot[3],
                                     const float scale[3],
                                     int32_t unk6,
                                     int32_t itemPickupFlag )
                                 {
                                     if ( item == 0x40 )
                                     {
                                         // Spawn the appropriate item
                                         uint8_t itemID = randomizer->getBossItem();
                                         itemID = game_patch::_04_verifyProgressiveItem( mod::randomizer, itemID );
                                         uint32_t params = itemID | 0xFFFF00;
                                         return tp::f_op_actor_mng::fopAcM_create( 539, params, pos, roomNo, rot, scale, -1 );
                                     }
                                     return return_createItemForMidBoss( pos, item, roomNo, rot, scale, unk6, itemPickupFlag );
                                 } );

        return_createItemForPresentDemo =
            patch::hookFunction( libtp::tp::f_op_actor_mng::createItemForPresentDemo,
                                 []( const float pos[3],
                                     int32_t item,
                                     uint8_t unk3,
                                     int32_t unk4,
                                     int32_t unk5,
                                     const float rot[3],
                                     const float scale[3] )
                                 {
                                     item = game_patch::_04_verifyProgressiveItem( mod::randomizer, item );
                                     return return_createItemForPresentDemo( pos, item, unk3, unk4, unk5, rot, scale );
                                 } );

        return_checkTreasureRupeeReturn =
            patch::hookFunction( tp::d_a_alink::checkTreasureRupeeReturn, []( void* unk1, int32_t item ) { return false; } );

        return_execItemGet = patch::hookFunction( libtp::tp::d_item::execItemGet,
                                                  []( uint8_t item )
                                                  {
                                                      item = game_patch::_04_verifyProgressiveItem( mod::randomizer, item );
                                                      return return_execItemGet( item );
                                                  } );

        return_checkItemGet = patch::hookFunction(
            libtp::tp::d_item::checkItemGet,
            []( uint8_t item, int32_t defaultValue )
            {
                switch ( item )
                {
                    using namespace libtp::tp;
                    using namespace libtp::data;
                    case items::Hylian_Shield:
                    {
                        // Check if we are at Kakariko Malo mart and verify that we have not bought the shield.
                        if ( tp::d_a_alink::checkStageName( stage::allStages[stage::stageIDs::Kakariko_Village_Interiors] ) &&
                             tp::d_kankyo::env_light.currentRoom == 3 && !tp::d_a_alink::dComIfGs_isEventBit( 0x6102 ) )
                        {
                            // Return false so we can buy the shield.
                            return 0;
                        }
                        break;
                    }
                    case items::Hawkeye:
                    {
                        // Check if we are at Kakariko Village and that the hawkeye is currently not for sale.
                        if ( ( tp::d_a_alink::checkStageName( stage::allStages[stage::stageIDs::Kakariko_Village] ) &&
                               !libtp::tp::d_save::isSwitch_dSv_memBit(
                                   &d_com_inf_game::dComIfG_gameInfo.save.memory.temp_flags,
                                   0x3E ) ) )
                        {
                            // Return false so we can buy the hawkeye.
                            return 0;
                        }
                        break;
                    }

                    case items::Ordon_Pumpkin:
                    case items::Ordon_Goat_Cheese:
                    {
                        // Check to see if currently in Snowpeak Ruins
                        if ( libtp::tp::d_a_alink::checkStageName(
                                 libtp::data::stage::allStages[libtp::data::stage::stageIDs::Snowpeak_Ruins] ) )
                        {
                            // Return false so that yeta will give the map item no matter what.
                            return 0;
                        }
                        break;
                    }
                    default:
                    {
                        // Call original function if the conditions are not met.
                        return return_checkItemGet( item, defaultValue );
                    }
                }
                return return_checkItemGet( item, defaultValue );
            } );

        return_query022 = patch::hookFunction( libtp::tp::d_msg_flow::query022,
                                               []( void* unk1, void* unk2, int32_t unk3 )
                                               { return events::proc_query022( unk1, unk2, unk3 ); } );

        return_query023 = patch::hookFunction( libtp::tp::d_msg_flow::query023,
                                               []( void* unk1, void* unk2, int32_t unk3 )
                                               { return events::proc_query023( unk1, unk2, unk3 ); } );

        return_checkDamageAction = patch::hookFunction(
            libtp::tp::d_a_alink::checkDamageAction,
            []( libtp::tp::d_a_alink::daAlink* linkMapPtr )
            {
                if ( isFoolishTrapQueued )
                {
                    if ( events::checkFoolItemFreeze() )
                    {
                        /* Store the currently loaded sound wave to local variables as we will need to load them back later.
                         * We use this method because if we just loaded the sound waves every time the item was gotten, we'd
                         * eventually run out of memory so it is safer to unload everything and load it back in.*/
                        uint8_t seWave1 = libtp::z2audiolib::z2audiomgr::g_mDoAud_zelAudio.mSceneMgr.SeWaveToErase_1;
                        uint8_t seWave2 = libtp::z2audiolib::z2audiomgr::g_mDoAud_zelAudio.mSceneMgr.SeWaveToErase_2;
                        isFoolishTrapQueued = false;
                        libtp::z2audiolib::z2scenemgr::eraseSeWave(
                            Z2ScenePtr,
                            libtp::z2audiolib::z2audiomgr::g_mDoAud_zelAudio.mSceneMgr.SeWaveToErase_1 );
                        libtp::z2audiolib::z2scenemgr::eraseSeWave(
                            Z2ScenePtr,
                            libtp::z2audiolib::z2audiomgr::g_mDoAud_zelAudio.mSceneMgr.SeWaveToErase_2 );
                        libtp::z2audiolib::z2scenemgr::loadSeWave( Z2ScenePtr, 0x46 );
                        libtp::z2audiolib::z2semgr::seStartLevel( 0x10040, nullptr, 0, 0 );
                        libtp::z2audiolib::z2scenemgr::loadSeWave( Z2ScenePtr, seWave1 );
                        libtp::z2audiolib::z2scenemgr::loadSeWave( Z2ScenePtr, seWave2 );
                        if ( libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_a.currentForm ==
                             1 )
                        {
                            return libtp::tp::d_a_alink::procWolfDamageInit( linkMapPtr, nullptr );
                        }
                        else
                        {
                            return libtp::tp::d_a_alink::procDamageInit( linkMapPtr, nullptr, 0 );
                        }
                    }
                }
                return return_checkDamageAction( linkMapPtr );
            } );

        return_query025 =
            patch::hookFunction( libtp::tp::d_msg_flow::query025,
                                 []( void* unk1, void* unk2, int32_t unk3 )
                                 {
                                     if ( libtp::tp::d_a_alink::checkStageName(
                                              libtp::data::stage::allStages[libtp::data::stage::stageIDs::Cave_of_Ordeals] ) )
                                     {
                                         // Return False to allow us to collect the item from the floor 50 reward.
                                         return false;
                                     }
                                     return return_query025( unk1, unk2, unk3 );
                                 } );

        return_query004 = patch::hookFunction(
            libtp::tp::d_msg_flow::query004,
            []( void* unk1, void* unk2, int32_t unk3 )
            {
                if ( libtp::tp::d_a_alink::checkStageName(
                         libtp::data::stage::allStages[libtp::data::stage::stageIDs::Castle_Town] ) &&
                     tp::d_kankyo::env_light.currentRoom == 2 )
                {
                    uint16_t donationCheck = *reinterpret_cast<uint16_t*>( reinterpret_cast<uint32_t>( unk2 ) + 4 );
                    if ( donationCheck == 0x1E )
                    {
                        *reinterpret_cast<uint16_t*>( reinterpret_cast<uint32_t>( unk2 ) + 4 ) = 100;
                    }
                }
                return return_query004( unk1, unk2, unk3 );
            } );

        return_loadSeWave = patch::hookFunction( libtp::z2audiolib::z2scenemgr::loadSeWave,
                                                 []( void* Z2SceneMgr, uint32_t waveID )
                                                 {
                                                     Z2ScenePtr = Z2SceneMgr;
                                                     return return_loadSeWave( Z2SceneMgr, waveID );
                                                 } );

        return_query037 = patch::hookFunction(
            libtp::tp::d_msg_flow::query037,
            []( void* unk1, void* unk2, int32_t unk3 )
            {
                int menuType = return_query037( unk1, unk2, unk3 );
                if ( ( menuType == 0x2 ) && ( reinterpret_cast<int32_t>( libtp::tp::d_a_player::m_midnaActor ) ==
                                              libtp::tp::f_op_actor_mng::fopAcM_getTalkEventPartner( nullptr ) ) )
                {
                    events::handleTimeOfDayChange();
                }
                return menuType;
            } );

        return_query042 = patch::hookFunction( libtp::tp::d_msg_flow::query042,
                                               []( void* unk1, void* unk2, int32_t unk3 )
                                               { return events::proc_query042( unk1, unk2, unk3 ); } );

        return_isSwitch_dSv_memBit = patch::hookFunction(
            libtp::tp::d_save::isSwitch_dSv_memBit,
            []( libtp::tp::d_save::dSv_memBit_c* memoryBit, int flag )
            {
                if ( libtp::tp::d_a_alink::checkStageName(
                         libtp::data::stage::allStages[libtp::data::stage::stageIDs::Kakariko_Graveyard] ) )
                {
                    if ( flag == 0x66 )     // Check for escort completed flag
                    {
                        if ( !libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                 0x804 ) )     // return false if we haven't gotten the item from Rutella.
                        {
                            return false;
                        }
                    }
                }

                if ( libtp::tp::d_a_alink::checkStageName(
                         libtp::data::stage::allStages[libtp::data::stage::stageIDs::Forest_Temple] ) )
                {
                    if ( flag == 0x3E && libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mEvtManager.mRoomNo == 0 )
                    {
                        return false;
                    }
                }
                return return_isSwitch_dSv_memBit( memoryBit, flag );
            } );

        return_onSwitch_dSv_memBit =
            patch::hookFunction( libtp::tp::d_save::onSwitch_dSv_memBit,
                                 []( libtp::tp::d_save::dSv_memBit_c* memoryBit, int flag )
                                 {
                                     if ( libtp::tp::d_a_alink::checkStageName(
                                              libtp::data::stage::allStages[libtp::data::stage::stageIDs::Forest_Temple] ) )
                                     {
                                         if ( memoryBit == &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.memory.temp_flags )
                                         {
                                             if ( flag == 0x52 )
                                             {
                                                 return;
                                             }
                                         }
                                     }
                                     return return_onSwitch_dSv_memBit( memoryBit, flag );
                                 } );

        return_roomLoader = patch::hookFunction(
            libtp::tp::d_stage::roomLoader,
            []( void* data, void* stageDt, int roomNo )
            {
                if ( randoIsEnabled( randomizer ) )
                {
                    events::onARC( randomizer, data, roomNo, rando::FileDirectory::Room );     // Replace room based checks.
                    void* filePtr = libtp::tp::d_com_inf_game::dComIfG_getStageRes(
                        "stage.dzs" );     // Could hook stageLoader instead since it takes the first param as a pointer to the
                                           // stage.dzs
                    events::onARC( randomizer,
                                   filePtr,
                                   roomNo,
                                   rando::FileDirectory::Stage );     // Replace stage based checks.
                }
                return return_roomLoader( data, stageDt, roomNo );
            } );

        return_chkEvtBit = patch::hookFunction(
            libtp::tp::d_msg_flow::chkEvtBit,
            []( uint32_t flag )
            {
                switch ( flag )
                {
                    case 0x153:     // Checking if the player has Ending Blow
                    {
                        if ( libtp::tp::d_a_alink::checkStageName(
                                 libtp::data::stage::allStages[libtp::data::stage::stageIDs::Hidden_Skill] ) )
                        {
                            return true;
                        }
                        break;
                    }

                    case 0x40:     // Checking if the player has completed Goron Mines
                    {
                        if ( libtp::tp::d_a_alink::checkStageName(
                                 libtp::data::stage::allStages[libtp::data::stage::stageIDs::Kakariko_Village_Interiors] ) )
                        {
                            return true;
                        }
                        break;
                    }
                    default:
                        break;
                }
                return return_chkEvtBit( flag );
            } );

        return_setLineUpItem = patch::hookFunction(
            tp::d_save::setLineUpItem,
            []( libtp::tp::d_save::dSv_player_item_c* unk1 )
            {
                using namespace libtp::tp::d_com_inf_game;
                static uint8_t i_item_lst[24] = { 0x0A, 0x08, 0x06, 0x02, 0x09, 0x04, 0x03, 0x00, 0x01, 0x17, 0x14, 0x05,
                                                  0x0F, 0x10, 0x11, 0x0B, 0x0C, 0x0D, 0x0E, 0x13, 0x12, 0x16, 0x15, 0x7 };
                int i1 = 0;
                int i2 = 0;

                for ( ; i1 < 24; i1++ )     // Clear all of the item slots.
                {
                    dComIfG_gameInfo.save.save_file.player.player_item.item_slots[i1] = 0xFF;
                }

                for ( i1 = 0; i1 < 24; i1++ )     // Fill all of the item wheel slots with their respective items if gotten.
                {
                    if ( dComIfG_gameInfo.save.save_file.player.player_item.item[i_item_lst[i1]] != 0xFF )
                    {
                        dComIfG_gameInfo.save.save_file.player.player_item.item_slots[i2] = i_item_lst[i1];
                        i2++;
                    }
                }
            } );
        return_isEventBit = patch::hookFunction(
            libtp::tp::d_save::isEventBit,
            []( libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag )
            {
                using namespace libtp::tp::d_a_alink;
                using namespace libtp::data::stage;
                switch ( flag )
                {
                    case 0x2904:     // Checking for ending blow.
                    {
                        if ( checkStageName( allStages[stageIDs::Hidden_Skill] ) )
                        {
                            return true;     // If we don't have the flag, the game sends us to Faron by default. Which we don't
                                             // want.
                        }
                        break;
                    }

                    case 0x2A20:
                    {
                        if ( checkStageName( allStages[stageIDs::Hidden_Skill] ) )
                        {
                            return false;     // Tell the game we don't have great spin to
                                              // not softlock in hidden skill training.
                        }
                        break;
                    }

                    case 0x2320:     // Gave Ilia the charm
                    case 0x3E02:     // CiTS Intro CS watched
                    {
                        if ( checkStageName( allStages[stageIDs::Hidden_Village] ) )
                        {
                            if ( !dComIfGs_isEventBit( 0x2280 ) )
                            {
                                return false;     // If we haven't gotten the item from Impaz then we need to return false or it
                                                  // will break her dialogue.
                            }
                        }
                        break;
                    }

                    case 0x701:     // Goron Mines Story Flag
                    {
                        if ( checkStageName( allStages[stageIDs::Goron_Mines] ) )
                        {
                            return false;     // The elders will not spawn if the flag is set.
                        }
                        break;
                    }

                    case 0x810:     // Escort Completed
                    {
                        if ( checkStageName( allStages[stageIDs::Castle_Town] ) )
                        {
                            return true;     // If flag isn't set, the player will be thrown into escort when they open the
                                             // door.
                        }
                        else if ( checkStageName( allStages[stageIDs::Kakariko_Village_Interiors] ) &&
                                  ( libtp::tp::d_kankyo::env_light.currentRoom ==
                                    0 ) )     // Return true to prevent Renado/Illia crash after ToT
                        {
                            return true;
                        }
                        break;
                    }

                    case 0x2010:     // AG story flag.
                    {
                        if ( checkStageName( allStages[stageIDs::Stallord] ) )
                        {
                            return false;     // If the flag is set, the post boss music plays during the boss fight.
                        }
                        break;
                    }

                    case 0x2008:     // Snowpeak Ruins Story flag
                    {
                        if ( checkStageName( allStages[stageIDs::Kakariko_Graveyard] ) )
                        {
                            return false;     // If the flag is set, Ralis will no longer spawn in the graveyard.
                        }
                        break;
                    }

                    case 0x602:     // Forest Temple Story Flag
                    {
                        if ( checkStageName( allStages[stageIDs::Diababa] ) )
                        {
                            return false;     // If the flag is set, the post boss music plays during the boss fight.
                        }
                        break;
                    }

                    case 0x5410:     // Zant Defeated (PoT Story Flag)
                    {
                        if ( checkStageName( allStages[stageIDs::Castle_Town] ) )
                        {
                            if ( !tp::d_a_alink::dComIfGs_isEventBit( 0x4208 ) )
                            {
                                using namespace libtp::data;
                                if ( randoIsEnabled( randomizer ) )
                                {
                                    switch ( randomizer->m_Seed->m_Header->castleRequirements )
                                    {
                                        case 3:     // All Dungeons
                                        {
                                            uint8_t numDungeons = 0x0;
                                            for ( int i = 0x10; i < 0x18; i++ )
                                            {
                                                if ( libtp::tp::d_save::isDungeonItem(
                                                         &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file
                                                              .area_flags[i]
                                                              .temp_flags,
                                                         3 ) )
                                                {
                                                    numDungeons++;
                                                }
                                            }
                                            if ( numDungeons == 0x8 )
                                            {
                                                libtp::tp::d_save::onSwitch_dSv_memBit(
                                                    &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.memory.temp_flags,
                                                    0x0F );
                                                events::setSaveFileEventFlag( 0x4208 );
                                                break;
                                            }
                                            else
                                            {
                                                return false;
                                            }
                                        }
                                        case 4:
                                        {
                                            return return_isEventBit( eventPtr, flag );
                                            break;
                                        }
                                        default:
                                        {
                                            return false;
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }

                    case 0x2002:     // City in the Sky Story flag
                    {
                        if ( checkStageName( allStages[stageIDs::Mirror_Chamber] ) )
                        {
                            if ( !tp::d_a_alink::dComIfGs_isEventBit( 0x2B08 ) )
                            {
                                using namespace libtp::data;
                                if ( randoIsEnabled( randomizer ) )
                                {
                                    if ( randomizer->m_Seed->m_Header->palaceRequirements != 3 )
                                    {
                                        return false;
                                    }
                                }
                            }
                        }
                        break;
                    }

                    default:
                    {
                        return return_isEventBit( eventPtr, flag );
                        break;
                    }
                }
                return return_isEventBit( eventPtr, flag );
            } );

        return_onEventBit = patch::hookFunction(
            libtp::tp::d_save::onEventBit,
            []( libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag )
            {
                using namespace libtp::tp::d_a_alink;
                using namespace libtp::data::stage;
                switch ( flag )
                {
                    case 0x1E08:
                    {
                        libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b
                            .dark_clear_level_flag |= 0x8;
                        break;
                    }

                    case 0x610:
                    {
                        if ( libtp::tp::d_a_alink::dComIfGs_isEventBit( 0x1e08 ) )
                        {
                            if ( libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b
                                     .dark_clear_level_flag == 0x6 )
                            {
                                libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b
                                    .transform_level_flag = 0x8;     // Set the flag for the last transformed twilight.
                                                                     // Also puts Midna on the player's back
                                libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b
                                    .dark_clear_level_flag |= 0x8;
                            }
                        }
                        break;
                    }

                    case 0x708:
                    {
                        if ( libtp::tp::d_a_alink::dComIfGs_isEventBit( 0x1e08 ) )
                        {
                            if ( libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b
                                     .dark_clear_level_flag == 0x5 )
                            {
                                libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b
                                    .transform_level_flag |= 0x8;     // Set the flag for the last transformed twilight.
                                // Also puts Midna on the player's back
                                libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b
                                    .dark_clear_level_flag |= 0x8;
                            }
                        }

                        break;
                    }

                    case 0xC02:
                    {
                        if ( libtp::tp::d_a_alink::dComIfGs_isEventBit( 0x1e08 ) )
                        {
                            if ( libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b
                                     .dark_clear_level_flag == 0x3 )
                            {
                                libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b
                                    .transform_level_flag |= 0x8;     // Set the flag for the last transformed twilight.
                                // Also puts Midna on the player's back
                                libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b
                                    .dark_clear_level_flag |= 0x8;
                            }
                        }

                        break;
                    }

                    default:
                    {
                        return return_onEventBit( eventPtr, flag );
                        break;
                    }
                }
                return return_onEventBit( eventPtr, flag );
            } );

        return_event000 =
            patch::hookFunction( libtp::tp::d_msg_flow::event000,
                                 []( void* messageFlow, void* nodeEvent, void* actrPtr )
                                 {
                                     // Prevent the hidden skill CS from setting the proper flags
                                     if ( libtp::tp::d_a_alink::checkStageName(
                                              libtp::data::stage::allStages[libtp::data::stage::stageIDs::Hidden_Skill] ) )
                                     {
                                         *reinterpret_cast<uint16_t*>( reinterpret_cast<uint32_t>( nodeEvent ) + 4 ) = 0x0000;
                                     }
                                     return return_event000( messageFlow, nodeEvent, actrPtr );
                                 } );

        return_event003 = patch::hookFunction(
            libtp::tp::d_msg_flow::event003,
            []( void* messageFlow, void* nodeEvent, void* actrPtr )
            {
                if ( libtp::tp::d_a_alink::checkStageName(
                         libtp::data::stage::allStages[libtp::data::stage::stageIDs::Castle_Town] ) &&
                     tp::d_kankyo::env_light.currentRoom == 2 )
                {
                    uint32_t donationAmount = *reinterpret_cast<uint32_t*>( reinterpret_cast<uint32_t>( nodeEvent ) + 4 );
                    if ( donationAmount == 0x1E )
                    {
                        libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_a.currentRupees =
                            libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_a.currentRupees -
                            100;
                        return 1;
                    }
                }
                return return_event003( messageFlow, nodeEvent, actrPtr );
            } );

        return_event041 = patch::hookFunction(
            libtp::tp::d_msg_flow::event041,
            []( void* messageFlow, void* nodeEvent, void* actrPtr )
            {
                if ( libtp::tp::d_a_alink::checkStageName(
                         libtp::data::stage::allStages[libtp::data::stage::stageIDs::Castle_Town] ) &&
                     tp::d_kankyo::env_light.currentRoom == 2 )
                {
                    uint32_t donationAmount = *reinterpret_cast<uint32_t*>( reinterpret_cast<uint32_t>( nodeEvent ) + 4 );
                    if ( donationAmount == 0x1E )
                    {
                        *reinterpret_cast<uint16_t*>(
                            &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.event_flags.event_flags[0xF7] ) += 100;
                        return 1;
                    }
                }
                return return_event041( messageFlow, nodeEvent, actrPtr );
            } );

        return_checkBootsMoveAnime = patch::hookFunction(
            libtp::tp::d_a_alink::checkBootsMoveAnime,
            []( libtp::tp::d_a_alink::daAlink* d_a_alink, int param_1 )
            {
                uint32_t ironBootsVars = reinterpret_cast<uint32_t>( &libtp::tp::d_a_alink::ironBootsVars );
                if ( *reinterpret_cast<float*>( ironBootsVars + 0x14 ) == 1.f )
                {
                    return false;
                }
                return return_checkBootsMoveAnime( d_a_alink, param_1 );
            } );

        return_createItemForTrBoxDemo =
            patch::hookFunction( libtp::tp::f_op_actor_mng::createItemForTrBoxDemo,
                                 []( const float pos[3],
                                     int32_t item,
                                     int32_t itemPickupFlag,
                                     int32_t roomNo,
                                     const int16_t rot[3],
                                     const float scale[3] )
                                 {
                                     events::handleDungeonHeartContainer();     // Set the flag for the dungeon heart container
                                                                                // if this item replaces it.
                                     item = game_patch::_04_verifyProgressiveItem( mod::randomizer, item );
                                     return return_createItemForTrBoxDemo( pos, item, itemPickupFlag, roomNo, rot, scale );
                                 } );

        return_isDungeonItem = patch::hookFunction( tp::d_save::isDungeonItem,
                                                    []( tp::d_save::dSv_memBit_c* membitPtr, const int memBit )
                                                    { return events::proc_isDungeonItem( membitPtr, memBit ); } );

        return_setGetItemFace = patch::hookFunction( libtp::tp::d_a_alink::setGetItemFace,
                                                     []( libtp::tp::d_a_alink::daAlink* linkMapPtr, uint16_t itemID )
                                                     {
                                                         switch ( itemID )
                                                         {
                                                             case libtp::data::items::Foolish_Item:
                                                             {
                                                                 itemID = libtp::data::items::Ordon_Pumpkin;
                                                                 break;
                                                             }
                                                             default:
                                                             {
                                                                 break;
                                                             }
                                                         }
                                                         return return_setGetItemFace( linkMapPtr, itemID );
                                                     } );

        return_dScnLogo_c_dt =
            patch::hookFunction( libtp::tp::d_s_logo::dScnLogo_c_dt,
                                 []( void* dScnLogo_c, int16_t bFreeThis )
                                 {
                                     // Call the original function immediately, as certain values need to be set first
                                     void* ret = return_dScnLogo_c_dt( dScnLogo_c, bFreeThis );

                                     // Initialize bgWindow since mMain2DArchive should now be set
                                     if ( !bgWindow )
                                     {
                                         void* main2DArchive = libtp::tp::d_com_inf_game::dComIfG_gameInfo.play.mMain2DArchive;
                                         if ( main2DArchive )
                                         {
                                             // Get the image to use for the background window
                                             void* bg = libtp::tp::JKRArchive::JKRArchive_getResource2( main2DArchive,
                                                                                                        0x54494D47,     // TIMG
                                                                                                        "tt_block8x8.bti" );

                                             if ( bg )
                                             {
                                                 bgWindow = new libtp::tp::J2DPicture::J2DPicture( bg );
                                             }
                                         }
                                     }

                                     return ret;
                                 } );

        return_dMenuRing__draw = patch::hookFunction(
            libtp::tp::d_menu_ring::dMenuRing__draw,
            []( void* dMenuRing )
            {
                using namespace libtp::tp::m_do_controller_pad;
                using namespace libtp::data::items;
                using namespace libtp::data::stage;

                // Get the current position of the ring
                const float* ringPos = reinterpret_cast<float*>( reinterpret_cast<uint32_t>( dMenuRing ) + 0x568 );

                const int32_t ringPosX = static_cast<int32_t>( ringPos[0] );
                const int32_t ringPosY = static_cast<int32_t>( ringPos[1] );

                // Get the size of the text
                constexpr float textSize = 16.f;

                // Get the increment amount for manual newlines
                constexpr int32_t increment = 16;

                // Draw the help text stating to press Start or Z to show the window
                constexpr int32_t helpTextPosXOffset = 465;
                constexpr int32_t helpTextPosYOffset = 157;
                constexpr uint32_t mainTextColor = 0xFFFFFFFF;     // White

                events::drawText( "Press Start\nor Z to\ntoggle\nadditional\ndata",
                                  ringPosX + helpTextPosXOffset,
                                  ringPosY + helpTextPosYOffset,
                                  mainTextColor,
                                  textSize );

                // Call the original function now, as everything else should be drawn on top of the vanilla stuff
                return_dMenuRing__draw( dMenuRing );

                // Check if the window should be drawm
                CPadInfo* padInfo = &cpadInfo;

                static bool displayWindow = false;
                bool shouldDisplayWindow = displayWindow;

                // If the ring was already drawn this frame, then dont check the buttons
                if ( !ringDrawnThisFrame )
                {
                    // Check if either Start or Z is pressed
                    if ( padInfo->buttonInputTrg & ( PadInputs::Button_Start | PadInputs::Button_Z ) )
                    {
                        shouldDisplayWindow = !shouldDisplayWindow;
                        displayWindow = shouldDisplayWindow;
                        ringDrawnThisFrame = true;
                    }
                }

                if ( !shouldDisplayWindow )
                {
                    return;
                }

                // Draw the window
                constexpr int32_t windowPosXOffset = 40;
                constexpr int32_t windowPosYOffset = 34;
                constexpr int32_t windowWidth = 528;
                constexpr int32_t windowHeight = 380;
                constexpr uint32_t windowColor = 0x000000FF;

                events::drawWindow( ringPosX + windowPosXOffset,
                                    ringPosY + windowPosYOffset,
                                    windowWidth,
                                    windowHeight,
                                    windowColor );

                // Create a generic buffer to use for string formatting
                char buf[32];

                // Set up an auto function for getting Yes or No text
                auto getYesNoText = []( bool flag )
                {
                    if ( flag )
                    {
                        return "Yes";
                    }
                    else
                    {
                        return "No";
                    }
                };

                // Get the current seed
                bool seedIsLoaded = false;
                if ( randoIsEnabled( randomizer ) )
                {
                    rando::SeedInfo* seedInfo = randomizer->m_SeedInfo;
                    if ( seedInfo )
                    {
                        snprintf( buf, sizeof( buf ), "Seed: 0x%016" PRIx64, seedInfo->header.seed );

                        seedIsLoaded = true;
                    }
                }

                const char* seedText;
                if ( seedIsLoaded )
                {
                    seedText = buf;
                }
                else
                {
                    seedText = "No seed is currently loaded";
                }

                // Draw the current seed
                constexpr int32_t currentSeedPosXOffset = windowPosXOffset + 15;
                constexpr int32_t currentSeedPosYOffset = windowPosYOffset + 28;

                events::drawText( seedText,
                                  ringPosX + currentSeedPosXOffset,
                                  ringPosY + currentSeedPosYOffset,
                                  mainTextColor,
                                  textSize );

                // Draw the main text for the fused shadows and mirror shards
                constexpr int32_t shadowsAndShardsMainPosXOffset = currentSeedPosXOffset;
                constexpr int32_t shadowsAndShardsMainPosYOffset = currentSeedPosYOffset + 40;

                events::drawText( "Fused Shadows\nMirror Shards",
                                  ringPosX + shadowsAndShardsMainPosXOffset,
                                  ringPosY + shadowsAndShardsMainPosYOffset,
                                  mainTextColor,
                                  textSize );

                // Get the counts for the fused shadows and mirror shards
                libtp::tp::d_save::dSv_player_collect_c* playerCollectPtr =
                    &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_collect;

                uint32_t shadowsCount = 0;
                uint32_t shardsCount = 0;
                uint8_t collectedShadows = playerCollectPtr->crystal;
                uint8_t collectedShards = playerCollectPtr->mirror;

                for ( uint32_t b = 5; b < 8; b++ )
                {
                    if ( ( collectedShadows << b ) & 0x80 )
                    {
                        shadowsCount++;
                    }
                }
                for ( uint32_t b = 4; b < 7; b++ )
                {
                    if ( ( collectedShards << b ) & 0x80 )
                    {
                        shardsCount++;
                    }
                }

                snprintf( buf, sizeof( buf ), "%" PRIu32 "/3\n%" PRIu32 "/3", shadowsCount, shardsCount );

                // Draw the counts for the fused shadows and mirror shards
                constexpr int32_t shadowsAndShardsCountsPosXOffset = shadowsAndShardsMainPosXOffset + 135;
                constexpr int32_t shadowsAndShardsCountsPosYOffset = shadowsAndShardsMainPosYOffset;

                events::drawText( buf,
                                  ringPosX + shadowsAndShardsCountsPosXOffset,
                                  ringPosY + shadowsAndShardsCountsPosYOffset,
                                  mainTextColor,
                                  textSize );

                // Get the text for the pumpkin
                snprintf( buf, sizeof( buf ), "Pumpkin\n%s", getYesNoText( events::haveItem( Item::Ordon_Pumpkin ) ) );

                // Draw the text for the pumpkin
                constexpr int32_t pumpkinPosXOffset = shadowsAndShardsCountsPosXOffset + 60;
                constexpr int32_t pumpkinPosYOffset = shadowsAndShardsCountsPosYOffset;

                events::drawText( buf, ringPosX + pumpkinPosXOffset, ringPosY + pumpkinPosYOffset, mainTextColor, textSize );

                // Get the text for the cheese
                snprintf( buf, sizeof( buf ), "Cheese\n%s", getYesNoText( events::haveItem( Item::Ordon_Goat_Cheese ) ) );

                // Draw the text for the cheese
                constexpr int32_t cheesePosXOffset = pumpkinPosXOffset + 90;
                constexpr int32_t cheesePosYOffset = pumpkinPosYOffset;

                events::drawText( buf, ringPosX + cheesePosXOffset, ringPosY + cheesePosYOffset, mainTextColor, textSize );

                // Draw the header text for the areas being tracked, small keys, big keys, dungeon maps, and compasses
                // Areas being tracked
                constexpr int32_t areasMainPosXOffset = shadowsAndShardsMainPosXOffset;
                constexpr int32_t areasMainPosYOffset = shadowsAndShardsMainPosYOffset + 60;

                events::drawText( "Areas",
                                  ringPosX + areasMainPosXOffset,
                                  ringPosY + areasMainPosYOffset,
                                  mainTextColor,
                                  textSize );

                // Small keys
                constexpr int32_t smallKeysMainPosXOffset = areasMainPosXOffset + 155;
                constexpr int32_t smallKeysMainPosYOffset = areasMainPosYOffset;

                events::drawText( "Small\nKeys",
                                  ringPosX + smallKeysMainPosXOffset,
                                  ringPosY + smallKeysMainPosYOffset,
                                  mainTextColor,
                                  textSize );

                // Big keys
                constexpr int32_t bigKeysMainPosXOffset = smallKeysMainPosXOffset + 70;
                constexpr int32_t bigKeysMainPosYOffset = smallKeysMainPosYOffset;

                events::drawText( "Big\nKeys",
                                  ringPosX + bigKeysMainPosXOffset,
                                  ringPosY + bigKeysMainPosYOffset,
                                  mainTextColor,
                                  textSize );

                // Dungeon maps
                constexpr int32_t dungeonMapsMainPosXOffset = bigKeysMainPosXOffset + 70;
                constexpr int32_t dungeonMapsMainPosYOffset = bigKeysMainPosYOffset;

                events::drawText( "Maps",
                                  ringPosX + dungeonMapsMainPosXOffset,
                                  ringPosY + dungeonMapsMainPosYOffset,
                                  mainTextColor,
                                  textSize );

                // Compasses
                constexpr int32_t compassesMainPosXOffset = dungeonMapsMainPosXOffset + 70;
                constexpr int32_t compassesMainPosYOffset = dungeonMapsMainPosYOffset;

                events::drawText( "Compasses",
                                  ringPosX + compassesMainPosXOffset,
                                  ringPosY + compassesMainPosYOffset,
                                  mainTextColor,
                                  textSize );

                // Set up an array to hold each area, as they will use different colors
                static const char* areas[] = { "Forest Temple",
                                               "Goron Mines",
                                               "Lakebed Temple",
                                               "Arbiter's Grounds",
                                               "Snowpeak Ruins",
                                               "Temple of Time",
                                               "City in the Sky",
                                               "Palace of Twilight",
                                               "Hyrule Castle",
                                               "Faron Woods",
                                               "Bublin Camp" };

                // Set up an array to hold each area's color id
                static const uint8_t areaColorIds[] = { MSG_COLOR_GREEN_HEX,
                                                        MSG_COLOR_RED_HEX,
                                                        CUSTOM_MSG_COLOR_BLUE_HEX,
                                                        MSG_COLOR_ORANGE_HEX,
                                                        MSG_COLOR_LIGHT_BLUE_HEX,
                                                        CUSTOM_MSG_COLOR_DARK_GREEN_HEX,
                                                        MSG_COLOR_YELLOW_HEX,
                                                        MSG_COLOR_PURPLE_HEX,
                                                        CUSTOM_MSG_COLOR_SILVER_HEX,
                                                        MSG_COLOR_GREEN_HEX,
                                                        MSG_COLOR_ORANGE_HEX };

                // Set up an array with all of the area node ids that small keys are tracked
                static const AreaNodesID smallKeyAreaNodes[] = { AreaNodesID::Forest_Temple,
                                                                 AreaNodesID::Goron_Mines,
                                                                 AreaNodesID::Lakebed_Temple,
                                                                 AreaNodesID::Arbiters_Grounds,
                                                                 AreaNodesID::Snowpeak_Ruins,
                                                                 AreaNodesID::Temple_of_Time,
                                                                 AreaNodesID::City_in_the_Sky,
                                                                 AreaNodesID::Palace_of_Twilight,
                                                                 AreaNodesID::Hyrule_Castle,
                                                                 AreaNodesID::Faron,
                                                                 AreaNodesID::Gerudo_Desert };

                // Set up an enum for accessing specific areas
                enum TrackedAreas
                {
                    FOREST_TEMPLE = 0,
                    GORON_MINES,
                    LAKEBED_TEMPLE,
                    ARBITERS_GROUNDS,
                    SNOWPEAK_RUINS,
                    TEMPLE_OF_TIME,
                    CITY_IN_THE_SKY,
                    PALACE_OF_TWILIGHT,
                    HYRULE_CASTLE,
                    FARON_WOODS,
                    GERUDO_DESERT
                };

                // Draw the areas, small key counts, big key counts, dungeon map counts, and compass counts
                // Areas position
                constexpr int32_t areasPosXOffset = areasMainPosXOffset;
                constexpr int32_t areasPosYOffset = areasMainPosYOffset + ( increment * 2 );

                // Small key counts position
                constexpr int32_t smallKeyCountsPosXOffset = smallKeysMainPosXOffset + 6;
                // constexpr int32_t smallKeyCountsPosYOffset = areasPosYOffset;

                // Big key counts position
                constexpr int32_t bigKeyCountsPosXOffset = bigKeysMainPosXOffset + 5;
                // constexpr int32_t bigKeyCountsPosYOffset = areasPosYOffset;

                // Dungeon map counts position
                constexpr int32_t dungeonMapCountsPosXOffset = dungeonMapsMainPosXOffset + 6;
                // constexpr int32_t dungeonMapCountsPosYOffset = areasPosYOffset;

                // Compass counts position
                constexpr int32_t compassCountsPosXOffset = compassesMainPosXOffset + 30;
                // constexpr int32_t compassCountsPosYOffset = areasPosYOffset;

                // Get the node id for the current area
                int32_t currentAreaNodeId = events::getCurrentAreaNodeId();

                // Temporary variable to keep track of current y coordinate
                int32_t tempPosY = ringPosY + areasPosYOffset;

                uint32_t loopCount = sizeof( areas ) / sizeof( areas[0] );
                for ( uint32_t i = 0; i < loopCount; i++ )
                {
                    uint32_t currentColor = libtp::tp::d_msg_class::getFontGCColorTable( areaColorIds[i], 0 );

                    // Draw the current area
                    events::drawText( areas[i], ringPosX + areasPosXOffset, tempPosY, currentColor, textSize );

                    // Make sure the node id is valid
                    if ( currentAreaNodeId >= 0 )
                    {
                        // Get the small key count for the current area
                        uint8_t* memoryFlags =
                            events::getNodeMemoryFlags( smallKeyAreaNodes[i], static_cast<AreaNodesID>( currentAreaNodeId ) );

                        uint8_t smallKeyCount = memoryFlags[0x1C];
                        uint8_t dungeonBits = memoryFlags[0x1D];

                        snprintf( buf, sizeof( buf ), "%" PRIu8, smallKeyCount );

                        // Draw the small key count for the current area
                        events::drawText( buf, ringPosX + smallKeyCountsPosXOffset, tempPosY, currentColor, textSize );

                        // Do not show big keys, dungeon maps, or compasses for Faron Woods or Bublin Camp
                        if ( i < TrackedAreas::FARON_WOODS )
                        {
                            // Draw the big key text
                            // Goron Mines has its big key split into 3 parts
                            int32_t bigKeyPosXIncrement = 0;
                            const char* bigKeyText;

                            if ( i == TrackedAreas::GORON_MINES )
                            {
                                bigKeyPosXIncrement = 8;
                                uint32_t bigKeyCount = 0;

                                // If the big key dungeon flag is set, then assume all 3 pieces have been obtained
                                if ( dungeonBits & 0x4 )
                                {
                                    bigKeyCount = 3;
                                }
                                else
                                {
                                    // Only need to track the first two key shards
                                    if ( events::haveItem( Item::Key_Shard_1 ) )
                                    {
                                        bigKeyCount++;
                                    }
                                    if ( events::haveItem( Item::Key_Shard_2 ) )
                                    {
                                        bigKeyCount++;
                                    }
                                }

                                snprintf( buf, sizeof( buf ), "%" PRIu32, bigKeyCount );

                                bigKeyText = buf;
                            }
                            else
                            {
                                bigKeyText = getYesNoText( dungeonBits & 0x4 );
                            }

                            events::drawText( bigKeyText,
                                              ringPosX + bigKeyCountsPosXOffset + bigKeyPosXIncrement,
                                              tempPosY,
                                              currentColor,
                                              textSize );

                            // Draw the dungeon map count
                            events::drawText( getYesNoText( dungeonBits & 0x1 ),
                                              ringPosX + dungeonMapCountsPosXOffset,
                                              tempPosY,
                                              currentColor,
                                              textSize );

                            // Draw the compass count
                            events::drawText( getYesNoText( dungeonBits & 0x2 ),
                                              ringPosX + compassCountsPosXOffset,
                                              tempPosY,
                                              currentColor,
                                              textSize );
                        }
                    }

                    tempPosY += increment;
                }
            } );
    }

    void setScreen( bool state )
    {
        consoleState = state;
        libtp::display::setConsole( state, 0 );
    }

    void doInput( uint32_t input )
    {
        using namespace libtp::tp::m_do_controller_pad;
        auto checkBtn = [&input]( uint32_t combo ) { return ( input & combo ) == combo; };

        if ( input && gameState == GAME_TITLE )
        {
            // Handle seed selection if necessary
            if ( seedList->m_numSeeds > 1 )
            {
                if ( checkBtn( Button_X ) )
                {
                    seedList->m_selectedSeed++;

                    if ( seedList->m_selectedSeed >= seedList->m_numSeeds )
                        seedList->m_selectedSeed = 0;
                }
                else if ( checkBtn( Button_Y ) )
                {
                    if ( seedList->m_selectedSeed == 0 )
                        seedList->m_selectedSeed = seedList->m_numSeeds;

                    seedList->m_selectedSeed--;
                }

                // 8 is the line it typically appears
                console.setLine( 8 );
                console << "\r"
                        << "[" << seedList->m_selectedSeed + 1 << "/" << seedList->m_numSeeds
                        << "] Seed: " << seedList->m_seedInfo[seedList->m_selectedSeed].header.seed << "\n";
            }
        }
        // End of handling title screen inputs
    }

    void handle_fapGm_Execute()
    {
        using namespace libtp;
        using namespace tp::m_do_controller_pad;
        using namespace tp::d_com_inf_game;

        // New frame, so the ring will be redrawn
        ringDrawnThisFrame = false;

        // Load relevant pointers locally for faster access
        CPadInfo* padInfo = &cpadInfo;
        dComIfG_inf_c* gameInfo = &dComIfG_gameInfo;

        // Handle game state updates
        using namespace libtp::tp::f_pc_node_req;

        if ( l_fpcNdRq_Queue )
        {
            // Previous state
            uint8_t prevState = gameState;
            uint8_t state = *reinterpret_cast<uint8_t*>( reinterpret_cast<uint32_t>( l_fpcNdRq_Queue ) + 0x59 );

            // Normal/Loading into game
            if ( prevState != GAME_ACTIVE && state == 11 )
            {
                // check whether we're in title screen CS
                if ( 0 != strcmp( "S_MV000", gameInfo->play.mNextStage.stageValues.mStage ) )
                {
                    gameState = GAME_ACTIVE;
                }
            }
            else if ( prevState != GAME_TITLE && ( state == 12 || state == 13 ) )
            {
                gameState = GAME_TITLE;

                // Handle console differently when the user first loads it
                if ( prevState == GAME_BOOT )
                {
                    switch ( seedList->m_numSeeds )
                    {
                        case 0:
                            // Err, no seeds
                            console << "No seeds available! Please check your memory card and region!\n";
                            setScreen( true );
                            break;

                        case 1:
                            // Only one seed present, auto-select it and disable console for convenience
                            console << "First and only seed automatically applied...\n";
                            setScreen( false );
                            break;

                        default:
                            // User has to select one of the seeds

                            console << "Please select a seed <X/Y>\n";
                            // trigger a dummy input to print the current selection
                            doInput( Button_Start );

                            setScreen( true );
                            break;
                    }
                }
            }
        }
        // End of handling gameStates

        // handle button inputs only if buttons are being held that weren't held last time
        if ( padInfo->buttonInput != lastButtonInput )
        {
            // Store before processing since we (potentially) un-set the padInfo values later
            lastButtonInput = padInfo->buttonInput;

            // Special combo to (de)activate the console should be handled first
            if ( ( padInfo->buttonInput & ( PadInputs::Button_R | PadInputs::Button_Z ) ) ==
                 ( PadInputs::Button_R | PadInputs::Button_Z ) )
            {
                // Disallow during boot as we print copyright info etc.
                // Will automatically disappear if there is no seeds to select from
                setScreen( !consoleState );
            }
            // Handle Inputs if console is already active
            else if ( consoleState )
            {
                doInput( padInfo->buttonInput );

                // Disable input so game doesn't notice
                padInfo->buttonInput = 0;
                padInfo->buttonInputTrg = 0;
            }

            else if ( ( padInfo->buttonInput & ( PadInputs::Button_R | PadInputs::Button_Y ) ) ==
                      ( PadInputs::Button_R | PadInputs::Button_Y ) )
            {
                // Disallow during boot as we print copyright info etc.
                // Will automatically disappear if there is no seeds to select from
                events::handleQuickTransform();
            }
        }

        // Handle rando state
        if ( gameState == GAME_ACTIVE )
        {
            if ( !randoIsEnabled( randomizer ) && ( seedList->m_numSeeds > 0 ) )
            {
                uint8_t selectedSeed = seedList->m_selectedSeed;

                if ( !randomizer )
                {
                    // The randomizer constructor sets m_Enabled to true
                    randomizer = new rando::Randomizer( &seedList->m_seedInfo[selectedSeed], selectedSeed );
                }
                else
                {
                    // Enable the randomizer
                    randomizer->m_Enabled = true;

                    // Check if loading a different seed
                    if ( randomizer->m_CurrentSeed != selectedSeed )
                    {
                        mod::console << "Changing seed:\n";
                        randomizer->changeSeed( &seedList->m_seedInfo[selectedSeed], selectedSeed );
                    }
                    else
                    {
                        // Not loading a different seed, so load checks for first load
                        randomizer->onStageLoad();
                    }
                }

                // Patches need to be applied whenever a seed is loaded.
                mod::console << "Patching game:\n";
                randomizer->m_Seed->applyPatches( true );
            }
        }
        else if ( randoIsEnabled( randomizer ) )
        {
            // Temporarily disable the randomizer
            randomizer->m_Enabled = false;
            randomizer->m_SeedInit = false;
        }

        // Custom events
        if ( !lastLoadingState && tp::f_op_scene_req::isLoading )
        {
            // OnLoad
            events::onLoad( randomizer );
        }

        if ( lastLoadingState && !tp::f_op_scene_req::isLoading )
        {
            // OffLoad
            events::offLoad( randomizer );
        }

        lastLoadingState = tp::f_op_scene_req::isLoading;
        rand( &nextVal );
        // End of custom events

        // Call the original function
        return return_fapGm_Execute();
    }

    uint32_t rand( uint32_t* seed )
    {
        uint32_t externSeed = *seed;
        uint32_t val = ( externSeed * 0x41C64E6D ) + 0x3039;
        *seed = val;
        return val;
    }

    uint32_t ulRand( uint32_t range )
    {
        uint32_t ret;

        if ( range > 0 )
        {
            ret = rand( &nextVal );
            ret -= ( ret / range ) * range;
        }
        else
        {
            ret = 0;
        }

        return ret;
    }
}     // namespace mod
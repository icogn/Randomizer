#include "game_patch/game_patch.h"
#include "main.h"
#include "data/stages.h"
#include "events.h"
#include "tp/d_a_alink.h"
#include "tp/d_com_inf_game.h"
#include "tp/d_kankyo.h"
#include "tp/d_save.h"
#include "data/flags.h"

namespace mod::game_patch
{
    int32_t getCurrentStageId()
    {
        int32_t loopCount = sizeof( libtp::data::stage::allStages ) / sizeof( libtp::data::stage::allStages[0] );
        for ( int32_t i = 0; i < loopCount; i++ )
        {
            if ( libtp::tp::d_a_alink::checkStageName( libtp::data::stage::allStages[i] ) )
            {
                return i;
            }
        }
        // Didn't find the current stage for some reason
        return -1;
    }
    KEEP_FUNC int32_t _01_getLayerNo( const char* stageName, int roomId, int layerOverride )
    {
        int chosenLayer;
        bool condition;
        int32_t stageID;
        unsigned int uVar2;
        libtp::tp::d_save::dSv_player_status_b_c* playerStatusBPtr =
            &libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b;
        using namespace libtp::data;
        using namespace libtp::data::flags;

        chosenLayer = layerOverride;
        stageID = getCurrentStageId();

        if ( layerOverride < 0 )
        {
            condition = libtp::tp::d_kankyo::dKy_darkworld_stage_check( stageName, roomId );
            if ( condition == true )
            {
                chosenLayer = stage::twilightStateIDs::Default_Twilight_State;
            }

            if ( chosenLayer < 0xD )
            {
                switch ( stageID )
                {
                    case stage::stageIDs::Snowpeak_Ruins:
                    {
                        condition =
                            libtp::tp::d_a_alink::dComIfGs_isEventBit( SNOWPEAK_RUINS_CLEARED );     // Snowpeak Ruins Completed
                        if ( condition )
                        {
                            chosenLayer = stage::snowpeakStateIDs::SPR_Dungeon_Completed;
                        }
                        break;
                    }
                    case stage::stageIDs::Snowpeak:
                    {
                        condition =
                            libtp::tp::d_a_alink::dComIfGs_isEventBit( SNOWPEAK_RUINS_CLEARED );     // Snowpeak Ruins Completed
                        if ( condition && ( roomId != 0 ) )
                        {
                            chosenLayer = stage::snowpeakStateIDs::SPR_Dungeon_Completed;
                        }
                        break;
                    }

                    case stage::stageIDs::Faron_Woods:
                    case stage::stageIDs::Faron_Woods_Interiors:
                    {
                        if ( ( roomId == 5 ) || ( roomId == 6 ) )     // North Faron or Mist Area
                        {
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( ORDON_DAY_2_OVER );     // Talo Saved
                            if ( condition )
                            {
                                chosenLayer = stage::faronStateIDs::Faron_MDH_Completed;
                            }
                            else
                            {
                                chosenLayer = stage::faronStateIDs::Faron_Talo_Captured;
                            }
                        }
                        else
                        {
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( ORDON_DAY_2_OVER );     // Talo Saved
                            if ( condition )
                            {
                                condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                    FOREST_TEMPLE_CLEARED );     // Forest Temple Completed
                                if ( condition )
                                {
                                    chosenLayer = stage::faronStateIDs::Faron_Snowpeak_Completed;
                                }
                            }
                            else
                            {
                                chosenLayer = stage::faronStateIDs::Faron_Talo_Captured;
                            }
                        }
                        break;
                    }

                    case stage::stageIDs::Kakariko_Village:
                    {
                        condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                            WATCHED_CUTSCENE_AFTER_GORON_MINES );     // Cutscene after GM
                                                                      // Watched
                        if ( condition == false )
                        {
                            condition =
                                libtp::tp::d_a_alink::dComIfGs_isEventBit( GORON_MINES_CLEARED );     // Goron Mines Completed
                            if ( condition == false )
                            {
                                chosenLayer = stage::kakarikoStateIDs::Kakariko_KB1_Completed;
                                // If it is night, the layer is different.
                                libtp::tp::d_com_inf_game::dComIfG_get_timelayer( &chosenLayer );
                            }
                            else
                            {
                                chosenLayer = stage::kakarikoStateIDs::Kakariko_Goron_Mines_Completed;
                            }
                        }
                        else
                        {
                            chosenLayer = stage::kakarikoStateIDs::Kakariko_KB1_Completed;
                            libtp::tp::d_com_inf_game::dComIfG_get_timelayer( &chosenLayer );
                        }

                        break;
                    }
                    case stage::stageIDs::Kakariko_Graveyard:
                    {
                        condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                            GOT_ZORA_ARMOR_FROM_RUTELA );     // Got Zora Armor from Rutela
                        if ( condition == false )
                        {
                            condition =
                                libtp::tp::d_a_alink::dComIfGs_isEventBit( ZORA_ESCORT_CLEARED );     // Zora Escort Cleared
                            if ( condition == false )
                            {
                                condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                    WATCHED_CUTSCENE_AFTER_GORON_MINES );     // Cutscene after GM
                                                                              // Watched
                                if ( condition == false )
                                {
                                    condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                        GORON_MINES_CLEARED );     // Goron Mines Completed
                                    if ( condition == false )
                                    {
                                        chosenLayer = stage::kakarikoStateIDs::Kakariko_KB1_Completed;
                                        // If it is night, the layer is different.
                                        libtp::tp::d_com_inf_game::dComIfG_get_timelayer( &chosenLayer );
                                    }
                                    else
                                    {
                                        chosenLayer = stage::kakarikoStateIDs::Kakariko_Goron_Mines_Completed;
                                    }
                                }
                                else
                                {
                                    chosenLayer = stage::kakarikoStateIDs::Kakariko_KB1_Completed;
                                    libtp::tp::d_com_inf_game::dComIfG_get_timelayer( &chosenLayer );
                                }
                            }
                            else
                            {
                                chosenLayer = stage::kakarikoStateIDs::Kakariko_Zora_Escort_Cleared;
                            }
                        }
                        else
                        {
                            chosenLayer = stage::kakarikoStateIDs::Kakariko_KB1_Completed;
                            libtp::tp::d_com_inf_game::dComIfG_get_timelayer( &chosenLayer );
                        }
                        break;
                    }

                    case stage::stageIDs::Kakariko_Graveyard_Interiors:
                    {
                        if ( ( ( roomId == 1 &&
                                 ( condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( LAKEBED_TEMPLE_CLEARED ),
                                   condition != false ) ) ) )     // Lakebed Completed
                        {
                            chosenLayer = stage::kakarikoInteriorStateIDs::Kakariko_Int_Lakebed_Completed;
                            libtp::tp::d_com_inf_game::dComIfG_get_timelayer( &chosenLayer );
                        }
                        else
                        {
                            chosenLayer = stage::kakarikoInteriorStateIDs::Kakariko_Int_KB1_Completed;
                            libtp::tp::d_com_inf_game::dComIfG_get_timelayer( &chosenLayer );
                        }
                        break;
                    }

                    case stage::stageIDs::Kakariko_Village_Interiors:
                    {
                        if ( roomId == 1 )     // Lakebed Completed
                        {
                            chosenLayer = stage::kakarikoInteriorStateIDs::Kakariko_Int_Lakebed_Completed;
                            libtp::tp::d_com_inf_game::dComIfG_get_timelayer( &chosenLayer );
                        }
                        else
                        {
                            chosenLayer = stage::kakarikoInteriorStateIDs::Kakariko_Int_KB1_Completed;
                            libtp::tp::d_com_inf_game::dComIfG_get_timelayer( &chosenLayer );
                        }
                        break;
                    }

                    case stage::stageIDs::Death_Mountain:
                    {
                        condition =
                            libtp::tp::d_a_alink::dComIfGs_isEventBit( GORON_MINES_CLEARED );     // Goron Mines Completed
                        if ( condition )
                        {
                            chosenLayer = stage::deathMountainStateIDs::Death_Mountain_Goron_Mines_Completed;
                        }
                        break;
                    }

                    case stage::stageIDs::Death_Mountain_Interiors:
                    {
                        chosenLayer = 0;
                        break;
                    }

                    case stage::stageIDs::Lake_Hylia:
                    {
                        if ( roomId == 1 )     // Lanayru Spring
                        {
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                LAKEBED_TEMPLE_CLEARED );     // Lakebed Temple has been completed
                            if ( condition )
                            {
                                condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                    MIDNAS_DESPERATE_HOUR_STARTED );     // MDH has been started
                                if ( condition == false )
                                {
                                    chosenLayer = stage::lakeHyliaStateIDs::Lake_Int_Lakebed_Completed;
                                }
                                else
                                {
                                    chosenLayer = stage::lakeHyliaStateIDs::Lake_Int_MDH_Started;
                                }
                            }
                        }
                        else
                        {
                            condition =
                                libtp::tp::d_a_alink::dComIfGs_isEventBit( SKY_CANNON_REPAIRED );     // Sky Cannon Repaired
                            if ( condition == false )
                            {
                                condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                    WARPED_SKY_CANNON_TO_LAKE_HYLIA );     // Sky Cannon Warped to Lake Hylia
                                if ( condition == false )
                                {
                                    chosenLayer = stage::lakeHyliaStateIDs::Lake_Hylia_Lakebed_Completed;
                                }
                                else
                                {
                                    chosenLayer = stage::lakeHyliaStateIDs::Lake_Hylia_Warped_Cannon_To_Lake;
                                }
                            }
                            else
                            {
                                chosenLayer = stage::lakeHyliaStateIDs::Lake_Hylia_Cannon_Repaired;
                            }
                        }
                        break;
                    }

                    case stage::stageIDs::Castle_Town_Interiors:
                    {
                        if ( condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( LAKEBED_TEMPLE_CLEARED ),
                             condition )     // Lakebed Temple Completed
                        {
                            chosenLayer = stage::castleTownInteriorsStateIDs::Castle_Town_Int_Lakebed_Completed;
                            if ( condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( MIDNAS_DESPERATE_HOUR_COMPLETED ),
                                 condition )     // MDH Completed
                            {
                                chosenLayer = stage::castleTownInteriorsStateIDs::Castle_Town_Int_Twilight_Cleared;
                            }
                        }
                        if ( roomId == 5 )     // Telma's Bar
                        {
                            chosenLayer = stage::castleTownInteriorsStateIDs::Castle_Town_Int_Watched_MS_Cutscene;
                        }
                        break;
                    }

                    case stage::stageIDs::Castle_Town:
                    {
                        condition =
                            libtp::tp::d_a_alink::dComIfGs_isEventBit( MIDNAS_DESPERATE_HOUR_COMPLETED );     // MDH Completed
                        if ( ( condition == false ) )
                        {
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                LAKEBED_TEMPLE_CLEARED );     // Lakebed Temple Completed
                            if ( condition == false )
                            {
                                if ( ( roomId == 3 ) &&
                                     ( condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( ZORA_ESCORT_CLEARED ),
                                       condition != false ) )     // Zora Escort Cleared
                                {
                                    chosenLayer = stage::castleTownStateIDs::Castle_Town_Finished_Zora_Escort;
                                }
                                else if ( roomId == 4 )
                                {
                                    chosenLayer = stage::castleTownStateIDs::Castle_Town_MDH_Clear;
                                }
                            }
                            else
                            {
                                chosenLayer = stage::castleTownStateIDs::Castle_Town_Lakebed_Completed;
                            }
                        }
                        else
                        {
                            if ( ( ( roomId == 4 ) || ( roomId == 3 ) ) || ( roomId == 1 ) )
                            {
                                chosenLayer = stage::castleTownStateIDs::Castle_Town_MDH_Clear;
                            }
                            else
                            {
                                chosenLayer = stage::castleTownStateIDs::Castle_Town_West_MDH_Clear;
                            }
                        }
                        break;
                    }

                    case stage::stageIDs::Zoras_Domain:
                    {
                        condition =
                            libtp::tp::d_a_alink::dComIfGs_isEventBit( SNOWPEAK_RUINS_CLEARED );     // Snowpeak Ruins Completed
                        if ( condition != false )
                        {
                            chosenLayer = stage::zorasDomainStateIDs::Domain_Snowpeak_Ruins_Completed;
                        }
                        break;
                    }

                    case stage::stageIDs::Upper_Zoras_River:
                    {
                        condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( IZA_1_MINIGAME_UNLOCKED );     // Iza 1 Unlocked
                        if ( condition != false )
                        {
                            chosenLayer = stage::upperZorasRiverStateIDs::Upper_Zoras_River_Iza_1_Started;
                        }
                        break;
                    }

                    case stage::stageIDs::Gerudo_Desert:
                    {
                        chosenLayer = stage::gerudoDesertStateIDs::Desert_Entrance_Cutscene_Not_Watched;
                        condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                            VISITED_DESERT_FOR_THE_FIRST_TIME );     // Have been to desert
                        if ( condition != false )
                        {
                            chosenLayer = stage::gerudoDesertStateIDs::Desert_Entrance_Cutscene_Watched;
                        }
                        break;
                    }

                    case stage::stageIDs::Zoras_River:
                    {
                        condition =
                            libtp::tp::d_a_alink::dComIfGs_isEventBit( IZA_1_MINIGAME_DONE );     // Iza 1 Minigame Completed
                        if ( condition == false )
                        {
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                STARTED_IZA_1_MINIGAME );     // Iza 1 Minigame Started
                            if ( condition != false )
                            {
                                chosenLayer = stage::zorasRiverStateIDs::Zoras_River_Iza_1_Started;
                            }
                        }
                        else
                        {
                            chosenLayer = stage::zorasRiverStateIDs::Zoras_River_Iza_1_Completed;
                        }
                        break;
                    }

                    case stage::stageIDs::Ordon_Village:
                    {
                        if ( roomId == 0 )
                        {
                            condition =
                                libtp::tp::d_a_alink::dComIfGs_isEventBit( ORDON_DAY_1_FINISHED );     // Ordon Day 1 done
                            if ( condition )
                            {
                                condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( ORDON_DAY_2_OVER );     // Talo Saved
                                if ( condition )
                                {
                                    condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                        FINISHED_SEWERS );     // First trip to Sewers done
                                    if ( condition )
                                    {
                                        uVar2 = libtp::tp::d_save::isDarkClearLV( playerStatusBPtr, 0 );
                                        if ( uVar2 == 0 )
                                        {
                                            chosenLayer = stage::ordonVillageStateIDs::Ordon_Finished_Sewers;
                                        }
                                        else
                                        {
                                            chosenLayer = stage::ordonVillageStateIDs::Ordon_Epona_Tamed;
                                            libtp::tp::d_com_inf_game::dComIfG_get_timelayer( &chosenLayer );
                                        }
                                    }
                                    else
                                    {
                                        chosenLayer = stage::ordonVillageStateIDs::Ordon_Talo_Rescued;
                                    }
                                }
                                else
                                {
                                    chosenLayer = stage::ordonVillageStateIDs::Ordon_Goats_1_Completed;
                                }
                            }
                            else
                            {
                                chosenLayer = stage::ordonVillageStateIDs::Ordon_New_Game;
                            }
                        }
                        else
                        {
                            if ( roomId == 1 )
                            {
                                condition =
                                    libtp::tp::d_a_alink::dComIfGs_isEventBit( ORDON_DAY_1_FINISHED );     // Ordon Day 1 done
                                if ( condition )
                                {
                                    condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( ORDON_DAY_2_OVER );     // Talo Saved
                                    if ( condition )
                                    {
                                        condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                            FINISHED_SEWERS );     // First trip to Sewers done
                                        if ( condition )
                                        {
                                            uVar2 = libtp::tp::d_save::isDarkClearLV( playerStatusBPtr, 0 );
                                            if ( uVar2 == 0 )
                                            {
                                                chosenLayer = stage::ordonVillageStateIDs::Ordon_Finished_Sewers;
                                            }
                                            else
                                            {
                                                chosenLayer = stage::ordonVillageStateIDs::Ordon_Faron_Twilight_Cleared;
                                            }
                                        }
                                        else
                                        {
                                            chosenLayer = stage::ordonVillageStateIDs::Ordon_Link_House_Talo_Rescued;
                                        }
                                    }
                                    else
                                    {
                                        chosenLayer = stage::ordonVillageStateIDs::Ordon_Link_House_Goats_1_Completed;
                                    }
                                }
                                else
                                {
                                    chosenLayer = stage::ordonVillageStateIDs::Ordon_Link_House_New_Game;
                                }
                            }
                        }
                        break;
                    }

                    case stage::stageIDs::Ordon_Village_Interiors:
                    {
                        if ( roomId == 1 )     // Sera's Shop
                        {
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                BOUGHT_SLINGSHOT_FROM_SERA );     // Bought slinghot from Sera
                            if ( condition )
                            {
                                chosenLayer = stage::ordonInteriorsStateIDs::Ordon_Int_Faron_Twilight_Cleared;
                            }
                        }
                        else
                        {
                            if ( roomId == 2 )     // Jaggle's House
                            {
                                uVar2 = libtp::tp::d_save::isDarkClearLV( playerStatusBPtr, 0 );
                                if ( uVar2 == 0 )
                                {
                                    condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                        FINISHED_SEWERS );     // First Trip to Sewers done
                                    if ( condition != false )
                                    {
                                        chosenLayer = stage::ordonInteriorsStateIDs::Ordon_Int_Finished_Sewers;
                                    }
                                }
                                else
                                {
                                    chosenLayer = stage::ordonInteriorsStateIDs::Ordon_Int_Faron_Twilight_Cleared;
                                }
                            }
                            else
                            {
                                if ( roomId == 5 )     // Rusl's House
                                {
                                    uVar2 = libtp::tp::d_save::isDarkClearLV( playerStatusBPtr, 0 );
                                    if ( uVar2 != 0 )
                                    {
                                        chosenLayer = stage::ordonInteriorsStateIDs::Ordon_Int_Faron_Twilight_Cleared;
                                    }
                                }
                            }
                        }
                        break;
                    }

                    case stage::stageIDs::Ordon_Spring:
                    {
                        if ( roomId == 1 )
                        {
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                TALO_CHASES_MONKEY );     // Sword training done on Ordon Day 2
                            if ( condition )
                            {
                                condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( ORDON_DAY_2_OVER );     // Talo saved
                                if ( condition )
                                {
                                    condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                        FINISHED_SEWERS );     // First trip to Sewers done
                                    if ( condition )
                                    {
                                        uVar2 = libtp::tp::d_save::isDarkClearLV( playerStatusBPtr, 0 );
                                        if ( uVar2 != 0 )
                                        {
                                            chosenLayer = stage::ordonSpringStateIDs::Ordon_Spring_Faron_Twilight_Cleared;
                                        }
                                        else
                                        {
                                            chosenLayer = stage::ordonSpringStateIDs::Ordon_Spring_Finished_Sewers;
                                        }
                                    }
                                    else
                                    {
                                        chosenLayer = stage::ordonSpringStateIDs::Ordon_Spring_Talo_Rescued;
                                    }
                                }
                                else
                                {
                                    chosenLayer = stage::ordonSpringStateIDs::Ordon_Spring_Sword_Tutorial_Completed;
                                }
                            }
                            else
                            {
                                chosenLayer = stage::ordonSpringStateIDs::Ordon_Spring_New_Game;
                            }
                        }
                        break;
                    }

                    case stage::stageIDs::Ordon_Ranch:
                    {
                        condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( ORDON_DAY_1_FINISHED );     // Day 1 done
                        if ( condition )
                        {
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( ORDON_DAY_2_OVER );     // Talo Saved
                            if ( condition )
                            {
                                condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                    WATCHED_CUTSCENE_AFTER_GOATS_2 );     // Saw CS after Goats 2 done
                                if ( condition )
                                {
                                    condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                        FINISHED_SEWERS );     // First trip to Sewers done
                                    if ( condition )
                                    {
                                        uVar2 = libtp::tp::d_save::isDarkClearLV( playerStatusBPtr, 0 );
                                        if ( uVar2 == 1 )
                                        {
                                            chosenLayer = stage::ordonRanchStateIDs::Ordon_Ranch_Faron_Twilight_Cleared;
                                            libtp::tp::d_com_inf_game::dComIfG_get_timelayer( &chosenLayer );
                                        }
                                        else
                                        {
                                            chosenLayer = stage::ordonRanchStateIDs::Ordon_Ranch_Finished_Sewers;
                                        }
                                    }
                                    else
                                    {
                                        chosenLayer = stage::ordonRanchStateIDs::Ordon_Ranch_Goats_2_Completed;
                                    }
                                }
                                else
                                {
                                    chosenLayer = stage::ordonRanchStateIDs::Ordon_Ranch_Talo_Rescued;
                                }
                            }
                            else
                            {
                                chosenLayer = stage::ordonRanchStateIDs::Ordon_Ranch_Goats_1_Completed;
                            }
                        }
                        else
                        {
                            chosenLayer = stage::ordonRanchStateIDs::Ordon_Ranch_New_Game;
                        }
                        break;
                    }

                    case stage::stageIDs::Hyrule_Field:
                    {
                        if ( libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.save_file.player.player_status_b
                                 .dark_clear_level_flag >= 0x7 )
                        {
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                MIDNAS_DESPERATE_HOUR_STARTED );     // MDH State Activated
                            if ( condition )
                            {
                                condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                    MIDNAS_DESPERATE_HOUR_COMPLETED );     // MDH Completed
                                if ( condition )
                                {
                                    chosenLayer = stage::hyruleFieldStateIDs::Hyrule_Field_MDH_Completed;
                                }
                                else
                                {
                                    chosenLayer = stage::hyruleFieldStateIDs::Hyrule_Field_MDH_Started;
                                }
                            }
                            else
                            {
                                chosenLayer = stage::hyruleFieldStateIDs::Hyrule_Field_New_Game;
                            }
                        }
                        else
                        {
                            chosenLayer = stage::hyruleFieldStateIDs::Hyrule_Field_New_Game;
                        }
                        break;
                    }

                    case stage::stageIDs::Outside_Castle_Town:
                    {
                        if ( roomId == 8 )
                        {
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                MIDNAS_DESPERATE_HOUR_COMPLETED );     // MDH Completed
                            if ( condition == false )
                            {
                                condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                    MIDNAS_DESPERATE_HOUR_STARTED );     // MDH State Activated
                                if ( condition != false )
                                {
                                    chosenLayer = stage::outsideCastleTownStateIDs::Outside_Castle_Town_MDH_Started;
                                }
                            }
                            else
                            {
                                chosenLayer = stage::outsideCastleTownStateIDs::Outside_Castle_Town_MDH_Completed;
                            }
                        }
                        else
                        {
                            if ( roomId == 0x10 )
                            {
                                condition =
                                    libtp::tp::d_a_alink::dComIfGs_isEventBit( GOT_WOOD_STATUE );     // Wooden Statue Gotten
                                if ( condition == false )
                                {
                                    condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                        TALKED_TO_LOUISE_ABOUT_THE_STOLEN_STATUE );     // Talked to Louise after Medicine Scent
                                    if ( condition == false )
                                    {
                                        condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                            MIDNAS_DESPERATE_HOUR_COMPLETED );     // MDH Completed
                                        if ( condition == false )
                                        {
                                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                                MIDNAS_DESPERATE_HOUR_STARTED );     // MDH State Activated
                                            if ( condition != false )
                                            {
                                                chosenLayer = stage::outsideCastleTownStateIDs::Outside_Castle_Town_MDH_Started;
                                            }
                                            else
                                            {
                                                chosenLayer =
                                                    stage::outsideCastleTownStateIDs::Outside_Castle_Town_MDH_Completed;
                                            }
                                        }
                                        else
                                        {
                                            chosenLayer = stage::outsideCastleTownStateIDs::Outside_Castle_Town_MDH_Completed;
                                        }
                                    }
                                    else
                                    {
                                        chosenLayer = stage::outsideCastleTownStateIDs::
                                            Outside_Castle_Town_Talked_To_Louise_Before_Statue;
                                    }
                                }
                                else
                                {
                                    chosenLayer = stage::outsideCastleTownStateIDs::Outside_Castle_Town_Got_Wooden_Statue_South;
                                }
                            }
                            else
                            {
                                if ( roomId == 0x11 )
                                {
                                    condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                        MIDNAS_DESPERATE_HOUR_COMPLETED );     // MDH Completed
                                    if ( condition == false )
                                    {
                                        condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                            MIDNAS_DESPERATE_HOUR_STARTED );     // MDH State Activated
                                        if ( condition != false )
                                        {
                                            chosenLayer = stage::outsideCastleTownStateIDs::Outside_Castle_Town_MDH_Started;
                                        }
                                    }
                                    else
                                    {
                                        chosenLayer = stage::outsideCastleTownStateIDs::Outside_Castle_Town_MDH_Completed_East;
                                    }
                                }
                            }
                        }
                        break;
                    }

                    case stage::stageIDs::Hidden_Village:
                    {
                        condition =
                            libtp::tp::d_a_alink::dComIfGs_isEventBit( GAVE_ILIA_HER_CHARM );     // Ilia shown Ilia's Charm
                        if ( condition != false )
                        {
                            chosenLayer = stage::hiddenVillageStateIDs::Hidden_Village_Showed_Ilia_Charm;
                        }
                        break;
                    }

                    case stage::stageIDs::Castle_Town_Shops:
                    {
                        if ( roomId == 5 )
                        {
                            chosenLayer = stage::castleTownShopsStateIDs::Castle_Town_Int_Jovani_MDH_Completed;
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( MIDNAS_DESPERATE_HOUR_STARTED );
                            if ( condition )
                            {
                                chosenLayer = stage::castleTownShopsStateIDs::Castle_Town_Int_Jovani_New_Game;
                                condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( MIDNAS_DESPERATE_HOUR_COMPLETED );
                                if ( condition )
                                {
                                    chosenLayer = stage::castleTownShopsStateIDs::Castle_Town_Int_Jovani_MDH_Completed;
                                }
                            }
                        }
                        else
                        {
                            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                                MALO_MART_CASTLE_TOWN_BRANCH_IS_OPEN );     // CT Shop is Malo Mart
                            if ( condition != false )
                            {
                                chosenLayer = stage::castleTownShopsStateIDs::Castle_Town_Int_Malo_Mart;
                            }
                        }
                        break;
                    }

                    case stage::stageIDs::Sacred_Grove:
                    {
                        chosenLayer = stage::sacredGroveStateIDs::Grove_2;
                        break;
                    }

                    case stage::stageIDs::Bulblin_Camp:
                    {
                        condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                            ESCAPED_BURNING_TENT_IN_BUBLIN_CAMP );     // Escaped Burning Tent in Bublin Camp
                        if ( condition )
                        {
                            if ( roomId == 3 )     // Other states for this room are very similar, but do not have the boar
                                                   // in the dzx.
                            {     // Setting state 1 solves for any potential softlocks regarding the boar in that area.
                                chosenLayer = stage::bulblinCampStateIDs::Bulblin_Camp_KB3_Completed;
                            }
                            else
                            {
                                chosenLayer = stage::bulblinCampStateIDs::Bulblin_Camp_Watched_Mirror_Cutscene;
                                libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.memory.temp_flags.memoryFlags[0x1C] = 0x0;
                            }
                        }
                        else
                        {
                            if ( events::haveItem( libtp::data::items::Bulblin_Camp_Key ) )
                            {
                                libtp::tp::d_com_inf_game::dComIfG_gameInfo.save.memory.temp_flags.memoryFlags[0x1C] = 0x1;
                            }
                        }

                        break;
                    }

                    case stage::stageIDs::Faron_Woods_Cave:
                    {
                        condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( ORDON_DAY_2_OVER );     // Talo saved
                        if ( condition != false )
                        {
                            chosenLayer = stage::faronWoodsCaveStateIDs::Faron_Woods_Cave_Talo_Rescued;
                        }
                        break;
                    }

                    case stage::stageIDs::Hyrule_Castle_Sewers:
                    {
                        condition = libtp::tp::d_a_alink::dComIfGs_isEventBit( FINISHED_SEWERS );     // Sewers Finished
                        if ( condition )
                        {
                            chosenLayer = stage::sewersStateIDs::Sewers_Midna_On_Back;
                        }
                        else
                        {
                            chosenLayer = stage::twilightStateIDs::Default_Twilight_State;
                        }
                        break;
                    }

                    case stage::stageIDs::Hyrule_Castle:
                    {
                        if ( ( ( roomId != 0xb ) && ( roomId != 0xd ) ) && ( roomId != 0xe ) )
                        {
                            chosenLayer = stage::hyruleCastleStateIDs::Hyrule_Castle_Interior_State;
                        }
                        break;
                    }

                    case stage::stageIDs::Fishing_Pond:
                    case stage::stageIDs::Fishing_Pond_Interiors:
                    {
                        if ( libtp::tp::d_kankyo::env_light.unk_1051[0x2AD] == 3 )
                        {
                            chosenLayer = stage::fishingHoleStateIDs::Fishing_Hole_Autumn;
                        }
                        else
                        {
                            if ( libtp::tp::d_kankyo::env_light.unk_1051[0x2AD] < 3 )
                            {
                                if ( libtp::tp::d_kankyo::env_light.unk_1051[0x2AD] == 1 )
                                {
                                    chosenLayer = stage::fishingHoleStateIDs::Fishing_Hole_Spring;
                                }
                                else
                                {
                                    if ( libtp::tp::d_kankyo::env_light.unk_1051[0x2AD] != 0 )
                                    {
                                        chosenLayer = stage::fishingHoleStateIDs::Fishing_Hole_Summer;
                                    }
                                }
                            }
                            else
                            {
                                if ( libtp::tp::d_kankyo::env_light.unk_1051[0x2AD] < 5 )
                                {
                                    chosenLayer = stage::fishingHoleStateIDs::Fishing_Hole_Winter;
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }

        if ( chosenLayer == stage::twilightStateIDs::Default_Twilight_State )
        {
            condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                WARPED_METEOR_TO_ZORAS_DOMAIN );     // Warped meteor to Zora's Domain
            switch ( stageID )
            {
                case stage::stageIDs::Zoras_River:
                case stage::stageIDs::Zoras_Domain:
                case stage::stageIDs::Lake_Hylia:
                case stage::stageIDs::Castle_Town:
                case stage::stageIDs::Castle_Town_Interiors:
                case stage::stageIDs::Upper_Zoras_River:
                case stage::stageIDs::Outside_Castle_Town:
                {
                    if ( condition != false )
                    {
                        chosenLayer = stage::zorasDomainStateIDs::Domain_Meteor_Warped;
                    }
                    break;
                }

                case stage::stageIDs::Hyrule_Field:
                {
                    if ( ( 8 < roomId ) && ( roomId < 0xf ) && ( condition != false ) )
                    {
                        chosenLayer = stage::hyruleFieldStateIDs::Hyrule_Field_Meteor_Warped;
                    }
                    break;
                }

                case stage::stageIDs::Hyrule_Castle_Sewers:
                {
                    condition = libtp::tp::d_a_alink::dComIfGs_isEventBit(
                        WATCHED_CUTSCENE_AFTER_BEING_CAPTURED_IN_FARON_TWILIGHT );     // Watched CS after being captured in
                                                                                       // Faron Twilight
                    if ( condition == false )
                    {
                        chosenLayer = stage::sewersStateIDs::Sewers_First_Time;
                    }
                    break;
                }

                case stage::stageIDs::Zant_Main_Room:
                {
                    if ( roomId == 10 )
                    {
                        condition =
                            libtp::tp::d_a_alink::dComIfGs_isEventBit( PALACE_OF_TWILIGHT_CLEARED );     // Zant Defeated
                        if ( condition == false )
                        {
                            chosenLayer = stage::twilightStateIDs::Palace_of_Twilight_Zant_Fight;
                        }
                        else
                        {
                            chosenLayer = stage::twilightStateIDs::Palace_of_Twilight_Zant_Defeated;
                        }
                    }
                    break;
                }
            }
        }
        return chosenLayer;
    }
}     // namespace mod::game_patch
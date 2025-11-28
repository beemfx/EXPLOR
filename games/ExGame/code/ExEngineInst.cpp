// (c) 2016 Beem Media

#include "ExEngineInst.h"
#include "ExGame.h"
#include "ExDialogMenu.h"
#include "ExConversationMenu.h"
#include "EGDisplayList.h"
#include "ExUiSounds.h"
#include "ExPortraits.h"
#include "ExQuests.h"
#include "ExSpellBook.h"
#include "ExQuestItems.h"
#include "ExMapInfos.h"
#include "ExBeastiary.h"
#include "ExArmory.h"
#include "EGEngineConfig.h"
#include "ExEnt.h"
#include "ExSaveMgr.h"
#include "ExClient.h"
#include "EGServer.h"
#include "ExMenu.h"
#include "EGSmMgr.h"
#include "ExDayNightCycles.h"
#include "ExShopData.h"
#include "ExCombatEncounter.h"
#include "ExGlobalData.h"
#include "EGGlobalConfig.h"
#include "ExLoadingHints.h"
#include "EGLoader.h"
#include "EGSettings2Types.h"
#include "ExAchievements.h"
#include "ExNpcGossip.h"
#include "EGPlatform.h"

EG_CLASS_DECL( ExMenu )
EG_CLASS_DECL( ExEngineInst )

static const eg_int EX_STATS_VERSION = 3; // Increment this to reset stats across all users

eg_s_string_sml8 ExEngineInst::s_BuildVersion;

static EGSettingsReal ExEngineInst_MaxAspectRatio( "ExEngineInst.MaxAspectRatio" , CT_Clear , 16.f/9.f , EGS_F_SYS_SAVED );
static EGSettingsBool ExEngineInst_ClassicAspectRatio( "ExEngineInst.ClassicAspectRatio" , eg_loc("ExSettingsClassicAspectRatio","Classic Aspect Ratio (4:3)") , false , EGS_F_USER_SAVED|EGS_F_EDITABLE );

void ExEngineInst::OnGameEvent( eg_game_event Event )
{
	Super::OnGameEvent( Event );

	switch( Event )
	{
	case eg_game_event::INIT:
	{
		NetConfig_DefaultServer.SetFromString( "HTTPS://EG.BEEMSOFT.NET" );

		EGSmMgr_Init();
		ExUiSounds::Get().Init();
		eg_uint64 StartTime = Timer_GetRawTime();
		ExGlobalData::Init( GAME_DATA_PATH "/gamedata/GlobalData.egasset" );
		ExPortraits::Init( GAME_DATA_PATH "/gamedata/PortraitsData.egasset" );
		ExQuests::Init( GAME_DATA_PATH "/gamedata/Quests.egasset" );
		ExMapInfos::Init( GAME_DATA_PATH "/gamedata/MapInfos.egasset" );
		ExBeastiary::Init( GAME_DATA_PATH "/gamedata/Beastiary.egasset" );
		ExSpellBook::Init( GAME_DATA_PATH "/gamedata/SpellBook.egasset" );
		ExArmory::Init( GAME_DATA_PATH "/gamedata/Armory.egasset" );
		ExShopData::Init( GAME_DATA_PATH "/gamedata/ShopData.egasset" );
		ExQuestItems::Init( GAME_DATA_PATH "/gamedata/QuestItems.egasset" );
		ExDayNightCycles::Init( GAME_DATA_PATH "/gamedata/DayNightCycles.egasset" );
		ExCombatEncounter::Init( GAME_DATA_PATH "/gamedata/CombatEncounters.egasset" );
		ExLoadingHints::Init( GAME_DATA_PATH "/gamedata/LoadingHints.egasset" );
		ExAchievements::Init( GAME_DATA_PATH "/gamedata/Achievements.egasset" );
		ExNpcGossip::Init( GAME_DATA_PATH "/gamedata/NpcGossip.egasset" );
		eg_uint64 EndTime = Timer_GetRawTime();
		eg_real LoadingSeconds = Timer_GetRawTimeElapsedSec( StartTime , EndTime );
		EGLogf( eg_log_t::Performance , "Game data loaded in %g seconds." , LoadingSeconds );

		const exGlobalDataUiColors& UiColors = ExGlobalData::Get().GetUiColors();

		#define REGISTER_UI_COLOR( _name_ ) EGTextNode::RegisterColor( eg_string_crc(#_name_) , UiColors._name_ )

		REGISTER_UI_COLOR( EX_GOLD );
		REGISTER_UI_COLOR( EX_INPARTY );
		REGISTER_UI_COLOR( EX_ITEM );
		REGISTER_UI_COLOR( EX_RARE_1 );
		REGISTER_UI_COLOR( EX_RARE_2 );
		REGISTER_UI_COLOR( EX_RARE_3 );
		REGISTER_UI_COLOR( EX_RARE_4 );
		REGISTER_UI_COLOR( EX_RARE_5 );
		REGISTER_UI_COLOR( EX_CLASS );
		REGISTER_UI_COLOR( EX_ATTR );
		REGISTER_UI_COLOR( EX_ATTRV );
		REGISTER_UI_COLOR( EX_H1 );
		REGISTER_UI_COLOR( EX_H2 );
		REGISTER_UI_COLOR( EX_HEAL );
		REGISTER_UI_COLOR( EX_HURT );
		REGISTER_UI_COLOR( EX_INC );
		REGISTER_UI_COLOR( EX_DEC );
		REGISTER_UI_COLOR( EX_WARN );

		REGISTER_UI_COLOR( EX_PDMG );
		REGISTER_UI_COLOR( EX_FDMG );
		REGISTER_UI_COLOR( EX_WDMG );
		REGISTER_UI_COLOR( EX_EDMG );
		REGISTER_UI_COLOR( EX_ADMG );
		REGISTER_UI_COLOR( EX_SPELLDESC );
		REGISTER_UI_COLOR( EX_CLUE );

		REGISTER_UI_COLOR( EX_DISABLED_OPTION );

		#undef REGISTER_UI_COLOR

		// Build version
		if( MainLoader )
		{
			s_BuildVersion = "Local Build";
			EGFileData BuildVersionFileData( eg_file_data_init_t::HasOwnMemory );
			MainLoader->LoadNowTo( "/PAD.ini" , BuildVersionFileData );
			if( BuildVersionFileData.GetSize() > 0 )
			{
				s_BuildVersion = "";
				BuildVersionFileData.Write<eg_char8>( '\0' );
				eg_cpstr8 BuildStr = BuildVersionFileData.GetDataAs<eg_char8>();
				eg_size_t BuildStrLen = EGString_StrLen( BuildStr );
				for( eg_size_t i=0; i<BuildStrLen; i++ )
				{
					if( BuildStr[i] == '\r' || BuildStr[i] == '\n' )
					{
						break;
					}
					else
					{
						s_BuildVersion.Append( BuildStr[i] );
					}
				}
			}
		}

		EGFontMgr::Get()->SetDefaultFont( eg_crc("ex_body") );

		EGPlatform_GetPlatform().SetStatsVersion( EX_STATS_VERSION );
	} break;
	case eg_game_event::DEINIT:
	{
		ExNpcGossip::Deinit();
		ExAchievements::Deinit();
		ExLoadingHints::Deinit();
		ExCombatEncounter::Deinit();
		ExDayNightCycles::Deinit();
		ExQuestItems::Deinit();
		ExShopData::Deinit();
		ExArmory::Deinit();
		ExSpellBook::Deinit();
		ExBeastiary::Deinit();
		ExMapInfos::Deinit();
		ExQuests::Deinit();
		ExPortraits::Deinit();
		ExUiSounds::Get().Deinit();
		ExGlobalData::Deinit();
		EGSmMgr_Deinit();
	} break;
	}
}

void ExEngineInst::OnRegisterInput( class ISdkInputRegistrar* Registrar )
{
	for( const exInputMapping& Mapping : EX_INPUT_MAPPING )
	{
		Registrar->SDK_RegisterInput( Mapping.Cmd, Mapping.Name, Mapping.GpButton, Mapping.KbButton );
	}
}

void ExEngineInst::SetupViewports( eg_real ScreenAspect, egScreenViewport* aVpOut, eg_uint VpCount ) const
{
	//If it's two we'll do one to the left and one to the right
	const eg_real MaxScreenAspect = ExEngineInst_ClassicAspectRatio.GetValue() ? 4.f/3.f : ExEngineInst_MaxAspectRatio.GetValue();

	if( VpCount == 1 && ScreenAspect > (MaxScreenAspect + EG_SMALL_NUMBER) )
	{
		aVpOut[0].Left = -MaxScreenAspect;
		aVpOut[0].Right = MaxScreenAspect;
	}

	if( VpCount == 2 )
	{
		aVpOut[0].Left = -ScreenAspect;
		aVpOut[0].Right = aVpOut[0].Left + ScreenAspect*1.5f;

		aVpOut[1].Right = ScreenAspect;
		aVpOut[1].Left = aVpOut[1].Right - ScreenAspect*1.5f;
	}
}

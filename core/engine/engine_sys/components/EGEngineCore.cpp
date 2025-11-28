// (c) 2017 Beem Media

#include "EGEngineCore.h"
#include "EGEngineInst.h"
#include "EGEngineConfig.h"
#include "EGInput.h"
#include "EGRenderer.h"
#include "EGEngine.h"
#include "EGAlwaysLoaded.h"
#include "EGSaveMgr.h"
#include "EGEngineInst.h"
#include "EGNetOnline.h"
#include "EGNetLocal.h"
#include "EGConsole.h"
#include "EGFileData.h"
#include "EGLoader.h"
#include "EGParse.h"
#include "EGAudio.h"
#include "EGLocalize.h"
#include "EGWorkerThreads.h"
#include "EGNetRequest.h"
#include "EGPhysMemMgr.h"
#include "EGDebugText.h"
#include "EGEngineFS.h"
#include "EGSettings2.h"
#include "EGPlatform.h"

void EGEngineCore::Init( EGEngineInst* InEngineInstance )
{
	assert( !m_EngineInst.IsValid() );
	m_EngineInst = InEngineInstance;
	
	EGEngineFS_Init( egEngineFSInitParms( m_EngineInst->GetPlatformExeDir() , m_EngineInst->GetGameName() , m_EngineInst->GetPlatformUserDir() , m_EngineInst->GetPlatformSysCfgDir() ) );
	EGLogToScreen::Get().Init();
	EGPhysMemMgr::Get().Init();
	m_EngineInst->GetInput().Init();
	AlwaysLoaded_Init();
	SaveMgr_Init();
	//Load the configuration file:
	m_EngineInst->OnRegisterInput( &m_EngineInst->GetInput() );
	ProcessSettings(true);
	NetOnline_Init();
	EGWorkerThreads_Init();
	NetRequest_Init( m_EngineInst->GetGameName() );

	//Audio:
	eg_string_crc AudioDriverId = eg_crc("DEFAULT");
	if( m_EngineInst->IsTool() )
	{
		AudioDriverId = eg_crc("DEFAULT");
	}
	else if( m_EngineInst->IsServerOnly() )
	{
		AudioDriverId = eg_crc("NULL");
	}
	EGAudio_Init( AudioDriverId );
	EGLocalize_Init( eg_loc_lang::ENUS );
}

void EGEngineCore::Deinit()
{
	EGLocalize_Deinit();
	EGAudio_Deinit();
	EGWorkerThreads_Deinit();
	NetRequest_Deinit(); // Deinit net requests after stopping the worker thread, that way we can purge any request that may have been in progress when we canceled.
	NetLocal_FlushAll(); //Make sure all straggling messages are purge.
	NetOnline_Deinit();
	SaveMgr_Deinit();
	AlwaysLoaded_Deinit();
	m_EngineInst->GetInput().Deinit();
	EGPhysMemMgr::Get().Deinit();
	EGLogToScreen::Get().Deinit();
	EGEngineFS_Deinit();
}

void EGEngineCore::Update( eg_real DeltaTime )
{
	const eg_real GameAspectRatio = EGRenderer::Get().GetAspectRatio(); // We get the aspect ratio every frame because it's possible it changed.

	EGInput::Get().Update( DeltaTime , Engine_WantsMouseCursor() , GameAspectRatio );

	EGWorkerThreads_Update( DeltaTime );
	NetRequest_Update( DeltaTime );
	EGLogToScreen::Get().Update( DeltaTime );
	EGPlatform_GetPlatform().Update( DeltaTime );

	MainLoader->ProcessCompletedLoads( EGLoader::LOAD_THREAD_MAIN );
}

void EGEngineCore::PrintDir()
{
	EGEngineFS_ListRoot();
}

eg_bool EGEngineCore::DoesFileExist( eg_cpstr Filename )
{
	return EGEngineFS_DoesFileExist( Filename );
}

void EGEngineCore::ProcessSettings_LoadFromFileData( const EGFileData& InFileData )
{
	InFileData.Seek( eg_file_data_seek_t::Begin , 0 );

	// Process each line.
	eg_s_string_sml8 Line;
	eg_string ConHistory[EGConsole::HISTORY_SIZE];
	eg_size_t ConHistorySize = 0;

	for(eg_uint i=0; i <= InFileData.GetSize(); i++)
	{
		const eg_char8 c = InFileData.Read<eg_char8>();

		if(';' == c)
		{
			//Blank lines are okay.
			if(Line.Len() > 0)
			{
				egParseFuncInfo RawInfo;
				EGPARSE_RESULT r = EGParse_ParseFunction( *Line , &RawInfo );
				if(EGPARSE_OKAY == r)
				{
					egParseFuncInfoAsEgStrings Info( RawInfo );
					if( "set" == Info.FunctionName && 2 == Info.NumParms )
					{
						EGSettingsVar* Var = EGSettingsVar::GetSettingByName( Info.Parms[0] );
						if( Var )
						{
							if( Var->GetFlags().IsSet( EGS_F_SYS_SAVED ) || Var->GetFlags().IsSet( EGS_F_USER_SAVED ) )
							{
								Var->SetFromString( Info.Parms[1] );
							}
						}
					}
					else if( "bind" == Info.FunctionName && 4 == Info.NumParms )
					{
						eg_string ClientIdStr = Info.Parms[0];

						Engine_BindInput( ClientIdStr , EGInput::Get() , Info.Parms[1] , Info.Parms[2] , Info.Parms[3] );
					}
					else if( "conhistory" == Info.FunctionName && 1 == Info.NumParms )
					{
						if( ConHistorySize < countof(ConHistory) )
						{
							ConHistory[ConHistorySize] = Info.Parms[0];
							ConHistorySize++;
						}
					}
					else
					{
						EGLogf( eg_log_t::Warning , "Invalid setting %s." , Info.FunctionName.String() );
					}
				}
				else
				{
					EGLogf( eg_log_t::Warning , "Bad setting (%s): \"%s\"" , EGParse_GetParseResultString(r) , *Line );
				}
			}
			Line.Clear();
		}
		else
		{
			Line.Append( c );
		}
	}

	if( ConHistorySize ) //This avoids setting the setting when autoexec.cfg runs.
	{
		MainConsole.SetCmdLineHistory( ConHistory , ConHistorySize );
	}
}

void EGEngineCore::ProcessSettings( eg_bool bLoad )
{
	const eg_d_string8 UserSettingsFilename = EGSFormat8( "{0}/user.cfg" , GAME_USER_PATH );
	const eg_d_string8 SystemSettingsFilename = EGSFormat8( "{0}/system.cfg" , GAME_SYSCFG_PATH );
	const eg_d_string8 InputSettingsFilename = EGSFormat8( "{0}/inputbinds.cfg" , GAME_USER_PATH );
	const eg_d_string8 ConHistorySettingsFilename = EGSFormat8( "{0}/conhistory.cfg" , GAME_SYSCFG_PATH );

	if(bLoad)
	{
		auto LoadSettingsFile = [this]( eg_cpstr8 Filename ) -> void
		{
			EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
			if( EGEngineCore::DoesFileExist( Filename ) )
			{
				MainLoader->LoadNowTo( Filename , FileData );
				ProcessSettings_LoadFromFileData( FileData );
			}
			else
			{
				EGLogf( eg_log_t::General , __FUNCTION__ ": Did not load %s, using defaults." , Filename );
			}

		};

		LoadSettingsFile( *UserSettingsFilename );
		LoadSettingsFile( *SystemSettingsFilename );
		LoadSettingsFile( *InputSettingsFilename );
		LoadSettingsFile( *ConHistorySettingsFilename );
	}
	else
	{
		auto WriteSettingsMatchingFlag = []( eg_cpstr8 Filename , eg_flag Flag , eg_bool bWriteBindings , eg_bool bWriteConsoleHistory ) -> void
		{
			EGFileData FileData( eg_file_data_init_t::HasOwnMemory );

			if( bWriteConsoleHistory )
			{
				eg_string ConHistory[EGConsole::HISTORY_SIZE];
				const eg_size_t ConHistorySize = MainConsole.GetCmdLineHistory( ConHistory , countof(ConHistory) );

				for( eg_size_t i=0; i<ConHistorySize; i++ )
				{
					eg_string ConHistoryT = EGString_Format( "conhistory( \"%s\" );\r\n" , ConHistory[i].String() );
					FileData.Write( ConHistoryT.String() , ConHistoryT.Len() );
				}
			}

			if( Flag != 0 )
			{
				EGArray<EGSettingsVar*> SettingsVars;
				EGSettingsVar::GetAllSettingsMatchingFlags( SettingsVars , Flag );
				for( EGSettingsVar* Var : SettingsVars )
				{
					if( Var )
					{
						FileData.WriteStr8( EGString_Format( "set(\"%s\",\"%s\");\r\n" , Var->GetVarName() , *Var->ToString() ) );
					}
				}
			}

			if( bWriteBindings )
			{
				Engine_WriteBindings( FileData );
			}

			MainLoader->SaveData( Filename , FileData.GetDataAs<eg_byte>() , FileData.GetSize() );
		};

		WriteSettingsMatchingFlag( *UserSettingsFilename , EGS_F_USER_SAVED , false , false );
		WriteSettingsMatchingFlag( *SystemSettingsFilename , EGS_F_SYS_SAVED , false , false );
		WriteSettingsMatchingFlag( *InputSettingsFilename , 0 , true , false );
		WriteSettingsMatchingFlag( *ConHistorySettingsFilename , 0 , false , true );
	}
}

void EGEngineCore::LoadKeyBindings()
{
	const eg_d_string8 InputSettingsFilename = EGSFormat8( "{0}/inputbinds.cfg" , GAME_USER_PATH );

	auto LoadSettingsFile = [this]( eg_cpstr8 Filename ) -> void
	{
		EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
		if( EGEngineCore::DoesFileExist( Filename ) )
		{
			MainLoader->LoadNowTo( Filename , FileData );
			ProcessSettings_LoadFromFileData( FileData );
		}
		else
		{
			EGLogf( eg_log_t::General , __FUNCTION__ ": Did not load %s, using defaults." , Filename );
		}

	};

	LoadSettingsFile( *InputSettingsFilename );
}
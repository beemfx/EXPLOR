// (c) 2020 Beem Media. All Rights Reserved.

#include "ExSaveMgr.h"
#include "EGClient.h"
#include "EGLoader.h"
#include "EGMenuStack.h"
#include "EGSaveGame.h"
#include "EGSaveMgr.h"
#include "EGTextFormat.h"
#include "ExDialogMenu.h"
#include "ExGame.h"
#include "ExGlobalData.h"
#include "ExNewGameFlowMenu.h"
#include "ExProfileData.h"
#include "ExTime.h"

ExSaveMgr ExSaveMgr::s_Instance;
ex_save_name ExSaveMgr::s_NextSaveName = { '\0' };

eg_d_string ExSaveMgr::GetNextSaveName()
{
	s_NextSaveName[countof(s_NextSaveName)-1] = '\0'; // Just in case;
	eg_d_string Out;
	Out = s_NextSaveName;
	return Out;
}

void ExSaveMgr::Init( class EGClient* Client )
{
	if( Client )
	{
		for( eg_uint i=0; i<EX_SAVE_SLOTS; i++ )
		{
			exSlotData& SlotData = m_SaveSlots[i];
			SlotData = exSlotData();
			eg_s_string_sml8 Filename = GetSaveSlotFilename( i+1);
			eg_d_string16 SaveName;
			EGTimer::egSystemTime SaveTime;
			ex_save_status SaveStatus = GetSaveStatus( *Filename , SaveName , SaveTime );

			if( SaveStatus == ex_save_status::Exists )
			{
				SlotData.bUsed = true;
				SlotData.SaveTime = SaveTime;
				eg_loc_text LocText = EGFormat( L"{0}" , *SaveName );
				EGString_Copy( SlotData.SaveName , LocText.GetString() , countof(SlotData.SaveName) );
			}
			else if( SaveStatus == ex_save_status::IsCorrupted )
			{
				SlotData.bUsed = true;
				SlotData.bCorrupted = true;
				eg_loc_text LocText( eg_loc("SaveMgrCorruptedSave","Corrupted Save") );
				EGString_Copy( SlotData.SaveName , LocText.GetString() , countof(SlotData.SaveName) );
			}
		}
	}
}

void ExSaveMgr::Deinit( class EGClient* Client )
{
	unused( Client );
}

eg_bool ExSaveMgr::DoesSaveExist( EGClient* Client, eg_uint Slot )
{
	unused( Client );

	if( EG_IsBetween<eg_uint>( Slot , 1 , EX_SAVE_SLOTS ) )
	{
		return m_SaveSlots[Slot-1].bUsed;
	}
	return false;
}

eg_bool ExSaveMgr::IsSaveCorrupted( EGClient* Client , eg_uint Slot )
{
	unused( Client );

	if( EG_IsBetween<eg_uint>( Slot , 1 , EX_SAVE_SLOTS ) )
	{
		return m_SaveSlots[Slot-1].bCorrupted;
	}
	return false;
}

void ExSaveMgr::GetSortedAvailableSaveSlots( EGClient* Client , EGArray<eg_uint>& Out )
{
	unused( Client );

	for( eg_uint i=0; i<EX_SAVE_SLOTS; i++ )
	{
		if( DoesSaveExist( Client , i+1 ) )
		{
			Out.Append( i+1 );
		}
	}

	// Sort the list:
	Out.Sort( [this]( const eg_uint& LeftIndex , const eg_uint& RightIndex ) -> eg_bool
	{
		// Since we just formed this list we'll assume the indexes are valid...
		const exSlotData& LeftData = m_SaveSlots[LeftIndex-1];
		const exSlotData& RightData = m_SaveSlots[RightIndex-1];

		if( LeftData.bCorrupted != RightData.bCorrupted )
		{
			return RightData.bCorrupted;
		}

		return RightData.SaveTime < LeftData.SaveTime;
	} );
}

eg_bool ExSaveMgr::DoAnySavesExist( EGClient* Client )
{
	for( eg_uint i=0; i<EX_SAVE_SLOTS; i++ )
	{
		if( DoesSaveExist( Client , i+1 ) )
		{
			return true;
		}
	}
	return false;
}

eg_uint ExSaveMgr::FindFreeSaveSlot( EGClient* Client )
{
	unused( Client );

	for( eg_uint i=0; i<EX_SAVE_SLOTS; i++ )
	{
		if( !m_SaveSlots[i].bUsed )
		{
			return (i+1);
		}
	}
	// No free saves
	return 0;
}

void ExSaveMgr::DeleteSave( EGClient* Client, eg_uint Slot )
{
	unused( Client );

	if( EG_IsBetween<eg_uint>( Slot , 1 , EX_SAVE_SLOTS ) )
	{
		m_SaveSlots[Slot-1].bDeleted = true;
		m_SaveSlots[Slot-1].bUsed = false;
		if( ExProfileData_GetLastSaveSlot() == Slot )
		{
			ExProfileData_SetLastSaveSlot( 0 );
		}

		// Write an empty file to the save location:
		if( MainLoader )
		{	
			EGFileData EmptyFile( eg_file_data_init_t::HasOwnMemory );
			eg_d_string FullSaveFile = EGSFormat8( "{0}/{1}.esave" , GAME_USER_PATH , *GetSaveSlotFilename( Slot ) );
			MainLoader->SaveData( *FullSaveFile , EmptyFile.GetDataAs<eg_byte>() , EmptyFile.GetSize() );
		}
	}
}

eg_cplocstr ExSaveMgr::GetSaveName( EGClient* Client, eg_uint Slot )
{
	unused( Client );

	if( EG_IsBetween<eg_uint>( Slot , 1 , EX_SAVE_SLOTS ) && m_SaveSlots[Slot-1].bUsed )
	{
		return m_SaveSlots[Slot-1].SaveName;
	}

	assert( false );
	return L"-ERROR: No such save-";
}

EGTimer::egSystemTime ExSaveMgr::GetSaveTime( EGClient* Client , eg_uint Slot )
{
	unused( Client );

	if( EG_IsBetween<eg_uint>( Slot , 1 , EX_SAVE_SLOTS ) && m_SaveSlots[Slot-1].bUsed )
	{
		return m_SaveSlots[Slot-1].SaveTime;
	}

	assert( false );
	return EGTimer::egSystemTime();
}

eg_loc_text ExSaveMgr::GetFormattedSaveName( EGClient* Client , eg_uint Slot )
{
	eg_loc_text Out = EGFormat( L"{0}" , GetSaveName( Client , Slot ) );
	return Out;
}

eg_loc_text ExSaveMgr::GetFormattedSaveDateText( EGClient* Client , eg_uint Slot )
{
	unused( Client );

	eg_loc_text Out;
	if( EG_IsBetween<eg_uint>( Slot , 1 , EX_SAVE_SLOTS ) && m_SaveSlots[Slot-1].bUsed )
	{
		const exSlotData& SlotData = m_SaveSlots[Slot-1];
		if( SlotData.bUsed && !SlotData.bCorrupted )
		{
			Out = EGFormat( L"{0:FULL:ABBR} {0:TIME}" , &ExTimeFormatter( &SlotData.SaveTime , nullptr ) );
		}
	}
	return Out;
}

void ExSaveMgr::CreateSave( EGClient* Client , const exPackedCreateGameInfo& CreateInfo , eg_cplocstr SaveName )
{
	if( EG_IsBetween<eg_uint>( CreateInfo.Slot , 1 , EX_SAVE_SLOTS ) )
	{
		// It's possible we're overwriting, but we probably don't want to be.
		exSlotData& SlotData = m_SaveSlots[CreateInfo.Slot-1];

		assert( !SlotData.bUsed );
		SlotData.bUsed = true;
		SlotData.bDeleted = false;
		SlotData.bCorrupted = false;
		EGString_Copy( SlotData.SaveName , SaveName , countof(SlotData.SaveName) );
		ExProfileData_SetLastSaveSlot( CreateInfo.Slot );
		EGString_Copy( s_NextSaveName , SaveName , countof(s_NextSaveName) );
		Client->SDK_RunServerEvent( egRemoteEvent( eg_crc("CreateNewGame") , CreateInfo.AsEventParm ) );
		if( EGMenuStack* MenuStack = Client->SDK_GetMenuStack() )
		{
			MenuStack->PopTo( MenuStack->FindMenuByClass<ExNewGameFlowMenu>() );
		}
	}
	else
	{
		assert( false ); // Invalid slot.
	}
}

void ExSaveMgr::LoadSave( EGClient* Client, eg_uint Slot )
{
	if( EG_IsBetween<eg_uint>( Slot , 1 , EX_SAVE_SLOTS ) && m_SaveSlots[Slot-1].bUsed )
	{
		if( !m_SaveSlots[Slot-1].bCorrupted )
		{
			ExProfileData_SetLastSaveSlot( Slot );

			Client->SDK_RunServerEvent( egRemoteEvent( eg_crc("LoadSaveSlot") , Slot ) );
			Client->SDK_ClearMenuStack();
			Client->SDK_RunClientEvent( egRemoteEvent(eg_crc("ShowFullScreenLoading")) );
		}
		else
		{
			ExDialogMenu_PushDialogMenu( Client , exMessageDialogParms( eg_loc("OutOfDateSave","The requested save file was saved with an older version of the game and cannot be loaded.") ) );
		}
	}
	else
	{
		assert( false ); // No such save
		ExDialogMenu_PushDialogMenu( Client , exMessageDialogParms( eg_loc("NoSuchSave","The requested save file did not exist.") ) );
	}
}

void ExSaveMgr::SaveSave( ExGame* Game , eg_uint Slot )
{
	if( Game && EG_IsBetween<eg_uint>( Slot , 1 , EX_SAVE_SLOTS ) )
	{
		exSlotData& SlotData = m_SaveSlots[Slot-1];
		SlotData.SaveTime = Game->GetSaveTime();
		Game->SDK_SaveGameState( *GetSaveSlotFilename(Slot) );
	}
	else
	{
		assert( false ); // Attempting to save to an invalid slot.
	}
}

const eg_s_string_sml8 ExSaveMgr::GetSaveSlotFilename( eg_uint Slot )
{
	assert( EG_IsBetween<eg_uint>( Slot , 1 , EX_SAVE_SLOTS ) );
	return *EGSFormat8( "{1}slot{0}" , Slot-1 , *ExGlobalData::Get().GetSaveGamePrefix() );
}

ExSaveMgr::ex_save_status ExSaveMgr::GetSaveStatus( eg_cpstr Filename , eg_d_string16& SaveNameOut , EGTimer::egSystemTime& SaveTimeOut )
{
	const eg_d_string8 FullSaveFile = EGSFormat8( "{0}/{1}.esave" , GAME_USER_PATH , Filename );

	ex_save_status Result = ex_save_status::DoesNotExist;

	if( SaveMgr_DoesSaveExist( Filename ) )
	{
		EGFileData SaveGameFileData( eg_file_data_init_t::HasOwnMemory );
		MainLoader->LoadNowTo( *FullSaveFile , SaveGameFileData );
		if( SaveGameFileData.GetSize() == 0 )
		{
			Result = ex_save_status::WasDeleted;
		}
		else
		{
			EGSaveGame SaveGame;
			SaveGame.Load( SaveGameFileData , *FullSaveFile );

			const eg_size_t GlobalDataSize = SaveGame.m_GlobalDataInfos.IsValidIndex( 0 ) ? SaveGame.m_GlobalDataInfos[0].BinaryData.Len() : 0;
			const eg_size_t GameDataSize = sizeof(ExGame);
			const ExGame* GameData = GlobalDataSize == GameDataSize ? reinterpret_cast<const ExGame*>(SaveGame.m_GlobalDataInfos[0].BinaryData.GetArray()) : nullptr;

			if( GameData )
			{
				eg_bool bValid = GameData->GetSaveId() == ExGame::SAVE_ID && GameData->GetSaveVersion() == ExGame::SAVE_VERSION;
				if( bValid )
				{
					Result = ex_save_status::Exists;
					SaveNameOut = GameData->GetSaveName();
					SaveTimeOut = GameData->GetSaveTime();
				}
				else
				{
					Result = ex_save_status::IsCorrupted;
				}
			}
			else
			{
				Result = ex_save_status::IsCorrupted;
			}
		}
	}
	return Result;
}

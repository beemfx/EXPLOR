#include "EGSaveMgr.h"
#include "EGList.h"
#include "EGEngineConfig.h"
#include <fs_sys2/fs_sys2.h>

static eg_cpstr16 SaveMgr_EXT = L"esave";

static class EGSaveMgr
{
private:
	struct egListItem: public IListable
	{
		egSaveInfo Info;
	};
	static const eg_uint LIST_ID = 4453698;

	eg_uint            m_NumUserSaves;
	eg_uint            m_NumLevels;
	EGList<egListItem> m_List;
	egSaveInfo**       m_UserSaveList;
	egSaveInfo**       m_LevelList;
	

public:
	EGSaveMgr(): m_List(LIST_ID), m_NumUserSaves(0), m_NumLevels(0), m_UserSaveList(nullptr), m_LevelList(nullptr){ }

	void Init()
	{
		Deinit();
		FS_EnumerateFilesOfType( SaveMgr_EXT , EnumerateSaveFiles , this );
		
		m_UserSaveList = EGMem2_NewArray<egSaveInfo*>( m_NumUserSaves , eg_mem_pool::System );
		m_LevelList = EGMem2_NewArray<egSaveInfo*>( m_NumLevels , eg_mem_pool::System );

		if( nullptr == m_UserSaveList ||  nullptr == m_LevelList )
		{
			assert( false ); // Out of memory?
			m_NumUserSaves = 0;
			m_NumLevels = 0;
			if( m_UserSaveList )EGMem2_Free( m_UserSaveList );
			if( m_LevelList )EGMem2_Free( m_LevelList );
			m_UserSaveList = nullptr;
			m_LevelList = nullptr;
			Deinit();
			return;
		}

		eg_uint UserSavesInserted = 0;
		eg_uint LevelsInserted = 0;
		for( egListItem* Li : m_List )
		{
			// If it's a user save, insert it into that list.
			if( Li->Info.IsUserSave )
			{
				if( UserSavesInserted < m_NumUserSaves )
				{
					m_UserSaveList[UserSavesInserted] = &Li->Info;
					UserSavesInserted++;
				}
				else
				{
					assert( false ); // Counted user saves wrong?
				}
			}

			// If it's a level, insert it into that list.
			if( Li->Info.IsLevel)
			{
				if( LevelsInserted < m_NumLevels)
				{
					m_LevelList[LevelsInserted] = &Li->Info;
					LevelsInserted++;
				}
				else
				{
					assert( false ); // Counted levels wrong?
				}
			}
		}

		assert( m_NumLevels == LevelsInserted );
		assert( m_NumUserSaves == UserSavesInserted );

		m_NumLevels = LevelsInserted;
		m_NumUserSaves = UserSavesInserted;
	}

	void Deinit()
	{
		while( m_List.HasItems() )
		{
			egListItem* Item = m_List.GetFirst();
			m_List.Remove( Item );
			EGMem2_Free( Item );
		}
		m_NumLevels = 0;
		m_NumUserSaves = 0;
		if( m_UserSaveList )EGMem2_Free( m_UserSaveList );
		if( m_LevelList )EGMem2_Free( m_LevelList );
		m_UserSaveList = nullptr;
		m_LevelList = nullptr;
	}

	eg_uint GetNumUserSaves()const
	{
		return m_NumUserSaves;
	}

	eg_uint GetNumLevels()const
	{
		return m_NumLevels;
	}

	void GetUserSaveInfo( eg_uint Index, egSaveInfo* SaveInfo )const
	{
		EGLogf( eg_log_t::Warning , __FUNCTION__ ": This functionality is currently debug only as new saves are not included here." );
		if( Index < m_NumUserSaves )
		{
			*SaveInfo = *m_UserSaveList[Index];
		}
		else
		{
			EGLogf( eg_log_t::Error , __FUNCTION__ ": Tried to get save info for an invalid index." );
			zero( SaveInfo );
		}
	}

	void GetLevelInfo( eg_uint Index, egSaveInfo* SaveInfo )const
	{
		if( Index < m_NumLevels)
		{
			*SaveInfo = *m_LevelList[Index];
		}
		else
		{
			EGLogf( eg_log_t::Error, __FUNCTION__ ": Tried to get level info for an invalid index." );
			zero( SaveInfo );
		}
	}

private:
	void Init_AddFile( eg_cpstr Filename )
	{
		//K, we have the memory, so there should be no problem doing everything from here.
		egListItem* NewItem = EGMem2_New<egListItem>( eg_mem_pool::System );
		if( nullptr != NewItem )
		{
			zero( NewItem );

			eg_string FilenameHash = Filename;
			FilenameHash.ConvertToLower();
			FilenameHash.ClampEnd( eg_string( SaveMgr_EXT ).Len() + 1 ); //Cut off ext and ".".
			FilenameHash.CopyTo( NewItem->Info.File , countof(NewItem->Info.File) );

			if( EGString_EqualsICount( GAME_USER_PATH, FilenameHash, EGString_StrLen( GAME_USER_PATH ) ) )
			{
				NewItem->Info.IsUserSave = true;
				m_NumUserSaves++;
			}
			else
			{
				NewItem->Info.IsLevel = true;
				m_NumLevels++;
			}

			m_List.Insert( NewItem );
		}
		else
		{
			assert( false ); //Out of system memory?
		}
	}

	static void EnumerateSaveFiles( eg_cpstr16 Filename, void* Data )
	{
		EGSaveMgr* _this = reinterpret_cast<EGSaveMgr*>( Data );
		_this->Init_AddFile( EGString_ToMultibyte(Filename) );
	}

} SaveMgr;

void SaveMgr_Init()
{
	SaveMgr.Init();
}

void SaveMgr_Deinit()
{
	SaveMgr.Deinit();
}

eg_uint SaveMgr_GetNumUserSaves()
{
	return SaveMgr.GetNumUserSaves();
}

eg_uint SaveMgr_GetNumLevels()
{
	return SaveMgr.GetNumLevels();
}

void SaveMgr_GetUserSaveInfo( eg_uint Index, egSaveInfo* SaveInfo )
{
	SaveMgr.GetUserSaveInfo( Index , SaveInfo );
}

void SaveMgr_GetLevelInfo( eg_uint Index, egSaveInfo* SaveInfo )
{
	SaveMgr.GetLevelInfo( Index , SaveInfo );
}

eg_bool SaveMgr_DoesSaveExist( eg_cpstr Filename )
{
	eg_string FullFilename = EGString_Format( "%s/%s.esave" , GAME_USER_PATH , Filename );
	return FS_Exists( EGString_ToWide(FullFilename) ) == FS_TRUE;
}

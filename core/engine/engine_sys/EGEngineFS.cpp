// (c) 2018 Beem Media

#include "EGEngineFS.h"
#include "EGEngineConfig.h"
#include "EGAssetPath.h"
#include "EGPath2.h"
#include "EGAudio.h"
#include "EGFont.h"
#include "EGEntDict.h"
#include <fs_sys2/fs_sys2.h>

class EGEngineFS
{
private:

	typedef EGArray<eg_string> EGLpkList;

private:

public:

	void Init( const egEngineFSInitParms& InitParms )
	{
		if( !FS_Initialize( FS_Alloc , FS_Free ) )
		{
			EGLogf( eg_log_t::Error , __FUNCTION__ ": Failed to create file system." );
			return;
		}
		FS_SetErrPrintFn(FS_Print);

		//Should probably set the error level based on a cvar.
		FS_SetErrLevel(ERR_LEVEL_NOTICE);
		FS_MountBase( *InitParms.RootDir );

		eg_cpstr DBASE_PATH = *InitParms.GameName;
		FS_Mount(EGString_ToWide(DBASE_PATH), EGString_ToWide(GAME_DATA_PATH), MOUNT_FILE_OVERWRITE|MOUNT_MOUNT_SUBDIRS);


		eg_cpstr16 EGDATA_PATH = L"egdata";
		FS_Mount(EGDATA_PATH , L"/egdata" , MOUNT_FILE_OVERWRITE|MOUNT_MOUNT_SUBDIRS);

		//Get the names all of all lpk files for loading (we can't load from the enumerate function since it would modify the search for lpk files).
		EGLpkList LpkList;
		FS_EnumerateFilesOfType( L"lpk" , Init_FS_EnumerateLPKs , &LpkList );

		//Mount all lpks that don't start with the word "patch" since the patches need
		//to be mounted last so they will override any previous versions of files in them.
		eg_char PatchLpkName[] = ("patch");
		for( eg_uint i=0; i<LpkList.Len(); i++ )
		{
			const eg_string& LpkName = LpkList[i];
			if( EGString_EqualsCount( LpkName , PatchLpkName , countof(PatchLpkName)-1 ) )
				continue;

			FS_MountLPK( EGString_ToWide(LpkName) , MOUNT_FILE_OVERWRITELPKONLY );
		}

		for(eg_uint i=1; i<99; i++)
		{
			eg_string strPakFile = EGString_Format(("%s/%s%u.lpk"), GAME_DATA_PATH, PatchLpkName, i);

			if(FS_Exists( EGString_ToWide(strPakFile) ) )
				FS_MountLPK( EGString_ToWide(strPakFile), MOUNT_FILE_OVERWRITELPKONLY);
			else
				break;
		}

		//Mount the user directory (if it doesn't exist it should have been
		//created in WinMain.cpp::WindowsLoop
		//::EGLogf(("The user directory is %s"), strUserDir);
		if( InitParms.UserDir.Len() > 0 )
		{
			FS_Mount( *InitParms.UserDir , EGString_ToWide(GAME_USER_PATH) , MOUNT_FILE_OVERWRITE );
		}
		if( InitParms.SysCfgDir.Len() > 0 )
		{
			FS_Mount( *InitParms.SysCfgDir , EGString_ToWide(GAME_SYSCFG_PATH) , MOUNT_FILE_OVERWRITE );
		}
		FS_SetErrLevel(ERR_LEVEL_ERROR);

		eg_asset_path::SetGetAssetPathsFn( GetAssetPaths );
	}

	void Deinit()
	{
		FS_Shutdown();
	}

	void ListRoot()
	{
		FS_PrintMountInfo( PRINT_MOUNT_INFO_FULL );
	}

	eg_bool DoesFileExist( eg_cpstr Filename )
	{
		return FS_Exists( EGString_ToWide( Filename ) ) == FS_TRUE;
	}

private:

	static void* FS_Alloc(fs_size_t size, LF_ALLOC_REASON reason, const fs_char8*const type, const fs_char8*const file, const fs_uint line)
	{
		unused( type , file , line );

		switch( reason )
		{
		case LF_ALLOC_REASON_SYSTEM : return EGMem2_Alloc( size , eg_mem_pool::FsSystem );
		case LF_ALLOC_REASON_FILE   : return EGMem2_Alloc( size , eg_mem_pool::FsTemp );
		case LF_ALLOC_REASON_SCRATCH: return EGMem2_Alloc( size , eg_mem_pool::FsTemp );
		}

		assert( false );
		return nullptr;
	}

	static void FS_Free(void* p, LF_ALLOC_REASON reason)
	{
		switch( reason )
		{
		case LF_ALLOC_REASON_SYSTEM : return EGMem2_Free( p );
		case LF_ALLOC_REASON_FILE   : return EGMem2_Free( p );
		case LF_ALLOC_REASON_SCRATCH: return EGMem2_Free( p );
		}

		assert( false );
	}

	static void FS_Print( eg_cpstr16 strMessage )
	{
		eg_string sTemp;
		sTemp = strMessage;
		EGLogf( eg_log_t::FileSys , "%s" , sTemp.String() );
	}

	static void Init_FS_EnumerateLPKs( eg_cpstr16 Filename , void* Data )
	{
		EGLpkList* LpkList = reinterpret_cast<EGLpkList*>(Data);
		LpkList->Push( Filename );
	}

	static void GetAssetPaths( eg_asset_path_special_t SpecialType , eg_cpstr Ext , EGArray<eg_d_string>& Out )
	{
		switch( SpecialType )
		{
			case eg_asset_path_special_t::None:
			{
				FS_EnumerateFilesOfType( EGString_ToWide(Ext) , EnumerateAssetPaths , &Out );
			} break;
			case eg_asset_path_special_t::Sound:
			{
				EGArray<eg_string_small> AllSounds;
				EGAudio_QueryAllSounds( AllSounds );
				for( const eg_string_small& Sound : AllSounds )
				{
					Out.Append( Sound.String() );
				}
				Out.Sort();
			} break;
			case eg_asset_path_special_t::Font:
			{
				EGArray<eg_string_crc> AllFonts;
				EGFontMgr::Get()->QueryFonts( AllFonts );
				for( const eg_string_crc& Font : AllFonts )
				{
					Out.Append( EGCrcDb::CrcToString( Font ).String() );
				}
				Out.Sort();
			} break;
			case eg_asset_path_special_t::EntityDefinition:
			case eg_asset_path_special_t::GameEntities:
			case eg_asset_path_special_t::UiEntities:
			{
				const eg_bool bWantGameClasses = SpecialType == eg_asset_path_special_t::GameEntities || SpecialType == eg_asset_path_special_t::EntityDefinition;
				const eg_bool bWantUiClasses = SpecialType == eg_asset_path_special_t::UiEntities || SpecialType == eg_asset_path_special_t::EntityDefinition;
				EGArray<eg_string> EntDefs;
				EntDict_GetDefStrings( EntDefs , bWantGameClasses , bWantUiClasses );
				for( const eg_string& Str : EntDefs )
				{
					Out.Append( Str.String() );
				}
				Out.Sort();
			} break;
		}
	}

	static void EnumerateAssetPaths( eg_cpstr16 Filename , void* Data )
	{
		EGArray<eg_d_string>* pOut = reinterpret_cast<EGArray<eg_d_string>*>(Data);
		egPathParts2 FilePath = EGPath2_BreakPath( EGString_ToMultibyte(Filename) );
		pOut->Append( *FilePath.ToString( false , '/' ) );
	}
};

static EGEngineFS EGEngineFS_Inst;

void EGEngineFS_Init( const egEngineFSInitParms& InitParms )
{
	EGEngineFS_Inst.Init( InitParms );
}

void EGEngineFS_Deinit()
{
	EGEngineFS_Inst.Deinit();
}

eg_bool EGEngineFS_DoesFileExist( eg_cpstr Filename )
{
	return EGEngineFS_Inst.DoesFileExist( Filename );
}

void EGEngineFS_ListRoot()
{
	EGEngineFS_Inst.ListRoot();
}

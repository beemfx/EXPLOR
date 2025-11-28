#include "EGAlwaysLoaded.h"
#include "EGStringMap.h"
#include "EGFileData.h"
#include "EGLoader.h"
#include <fs_sys2/fs_sys2.h>

static eg_cpstr AlwaysLoaded_Filenames[] =
{
	"/egdata/shaders/default_EGVert.evs5",
	"/egdata/shaders/default_EGVert_Simple.evs5",
	"/egdata/shaders/default_EGVert_Terrain.evs5",
};

static class EGAlwaysLoaded
{
private:
	struct egFileData
	{
		void*     Data;
		eg_size_t Size;

		egFileData(): Data(nullptr), Size(0){ }
	};

	typedef EGStringCrcMapFixedSize<egFileData,countof(AlwaysLoaded_Filenames)> EGMap;

	EGMap      m_Map;
	egFileData m_Default;
public:
	EGAlwaysLoaded(): m_Map(CT_Clear , m_Default){}
	~EGAlwaysLoaded(){}

	void Init()
	{
		eg_size_t TotalSize = 0;

		for( eg_uint i=0; i<countof(AlwaysLoaded_Filenames); i++ )
		{
			if( m_Map.IsFull() )
			{
				assert( false );
				continue;
			}
			EGFileData File( eg_file_data_init_t::HasOwnMemory );
			MainLoader->LoadNowTo( AlwaysLoaded_Filenames[i] , File );
			if( File.GetSize() <= 0 )
			{
				assert( false );
				continue;
			}
			egFileData NewData;
			NewData.Data = EGMem2_Alloc( EG_AlignUp(File.GetSize()) , eg_mem_pool::System );
			if( nullptr == NewData.Data )
			{
				assert( false );
				continue;
			}
			NewData.Size = File.GetSize();
			EGMem_Copy( NewData.Data , File.GetData() , NewData.Size );
			eg_string_crc CrcId = FilenameToCrc( AlwaysLoaded_Filenames[i] );
			m_Map.Insert( CrcId , NewData );
			TotalSize += NewData.Size;
		}

		EGLogf( eg_log_t::Verbose , "AlwaysLoaded module took %u bytes (%uMB)" , TotalSize , TotalSize/(1024*1024) );
	}

	void Deinit()
	{
		for( eg_uint i=0; i<m_Map.Len(); i++ )
		{
			EGMem2_Free( m_Map.GetByIndex(i).Data );
		}
	}

	void* GetFile( eg_cpstr Filename , eg_size_t* SizeOut )
	{
		eg_string_crc CrcId = FilenameToCrc( Filename );
		egFileData Data;

		if( m_Map.Contains( CrcId ) )
		{
			Data = m_Map.Get( CrcId );
		}
		else
		{
			assert( false ); //No such always loaded file.
			Data.Data = nullptr;
			Data.Size = 0;
		}

		if( SizeOut ){ *SizeOut = Data.Size; }
		return Data.Data;
	}
private:
	eg_string_crc FilenameToCrc( eg_cpstr Filename )
	{
		eg_string S( Filename );
		S.ConvertToLower();
		return eg_string_crc(S);
	}

} AlwaysLoaded;


void AlwaysLoaded_Init()
{
	AlwaysLoaded.Init();
}

void AlwaysLoaded_Deinit()
{
	AlwaysLoaded.Deinit();
}

void* AlwaysLoaded_GetFile( eg_cpstr Filename , eg_size_t* SizeOut )
{
	return AlwaysLoaded.GetFile( Filename , SizeOut );
}

static void AlwaysLoaded_EnumerateFilenames( eg_cpstr16 Filename , void* Data )
{
	EGAlwaysLoadedFilenameList* ItemList = static_cast<EGAlwaysLoadedFilenameList*>(Data);
	ItemList->InsertFilename( EGString_ToMultibyte(Filename) );
}

void AlwaysLoaded_GetFileList( class EGAlwaysLoadedFilenameList* List , eg_cpstr Ext )
{
	List->ClearFilenames();
	FS_EnumerateFilesOfType( EGString_ToWide(Ext) , AlwaysLoaded_EnumerateFilenames , List );
}
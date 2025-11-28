///////////////////////////////////////////////////////////////////////////////
// Localize Database
// Basically a really big chunk of text organized by lookup keys.
// (c) 2016 Beem Media
///////////////////////////////////////////////////////////////////////////////

#include "EGLocalize.h"
#include "EGItemMap.h"
#include "EGFileData.h"
#include "EGLoader.h"
#include "EGEngineConfig.h"
#include "EGAlwaysLoaded.h"
#include "EGEngineTemplates.h"
#include "EGLocText.h"
#if defined(__EGEDITOR__)
#include "EGLibFile.h"
#include <EGWindowsAPI.h>
#endif

static class EGLocalize
{
private:
	struct egLocChunk: IListable
	{
		void*     Chunk;
		eg_size_t ChunkSize;
		eg_size_t NumEntries;

		egLocChunk( eg_size_t Size)
		: ChunkSize( Size )
		, Chunk( nullptr )
		, NumEntries( 0 )
		{
			Chunk = EGMem2_Alloc( Size , eg_mem_pool::System );
		}

		~egLocChunk()
		{
			EGMem2_Free( Chunk );
		}
	};

	struct egLocItem
	{
		const eg_loc_char*  String = nullptr;
		eg_size_t           Length = 0;
		eg_string_crc       Key = eg_string_crc( CT_Clear );
	};

	typedef EGSysMemItemMap<egLocItem> EGLocDb;

public:
	EGLocalize()
	: m_Initialized(false)
	, m_LocChunks( EGList<egLocChunk>::DEFAULT_ID )
	, m_LocDb( egLocItem() )
	{
	}

public:
	void Init( eg_loc_lang Language )
	{
		m_Lang = Language;
		eg_loc_text::SetLocFunction( LocalizeFunction );
		Init_LoadLocFile();
		m_Initialized = true;
	}

	void Deinit( void )
	{
		m_LocDb.Deinit();

		while( m_LocChunks.HasItems() )
		{
			egLocChunk* LocChunk = m_LocChunks.GetFirst();
			m_LocChunks.Remove( LocChunk );
			delete LocChunk;
		}
		eg_loc_text::SetLocFunction( nullptr );
		m_Lang = eg_loc_lang::UNK;
		m_Initialized  = false;
	}

	void Localize( eg_string_crc TextCrc , class eg_loc_text& LocTextOut   )
	{
		assert(m_Initialized);

		LocTextOut = eg_loc_text( L"" );

		if( TextCrc == eg_crc("") )
		{
			return;
		}

		if( !m_LocDb.Contains( TextCrc ) )
		{
			LocTextOut = eg_loc_text( EGString_ToWide(EGString_Format( ("!!!(0x%08X)!!!") , TextCrc.ToUint32() )) );
			return;
		}

		const egLocItem& LocText = m_LocDb.Get( TextCrc );
		LocTextOut = eg_loc_text( LocText.String , LocText.Length );
		return;
	}

	void DumpDb()
	{
#if defined(__EGEDITOR__)
		EGFileData TextFileData( eg_file_data_init_t::HasOwnMemory );

		EGArray<eg_char> Dest;
		EGArray<eg_char16> CleanSrc;

		TextFileData.WriteStr8( "---\r\n" );

		for( eg_size_t i=0; i<m_LocDb.Len(); i++ )
		{
			const egLocItem& LocItem = m_LocDb.GetByIndex( i );
			CleanSrc.Clear( false );
			eg_bool bInBars = false;
			eg_bool bInFormat = false;
			for( eg_size_t StrPos = 0; StrPos < LocItem.Length; StrPos++ )
			{
				const eg_char16 c = LocItem.String[StrPos];

				if( bInBars )
				{
					if( c == '|' )
					{
						bInBars = false;
					}
				}
				else if( bInFormat )
				{
					if( c == '}' )
					{
						bInFormat = false;
					}
				}
				else
				{
					if( c == '|' )
					{
						bInBars = true;
					}
					else if( c == '{' )
					{
						CleanSrc.Append( 'V' );
						CleanSrc.Append( 'a' );
						CleanSrc.Append( 'r' );
						bInFormat = true;
					}
					else
					{
						CleanSrc.Append( c );
					}
				}
			}
			CleanSrc.Append( '\0' );
			if( CleanSrc.Len() > 1 )
			{
				Dest.Resize( CleanSrc.Len() * 8 ); // Certainly 8X bigger is large enough.
				const int CharsCopied = WideCharToMultiByte( CP_UTF8 , 0 , CleanSrc.GetArray() , static_cast<int>(CleanSrc.Len()-1) , Dest.GetArray() , Dest.LenAs<int>() , NULL , NULL );
				TextFileData.Write( Dest.GetArray() , CharsCopied );
				TextFileData.WriteStr8( "\r\n---\r\n" );
			}
		}

		EGLibFile_SaveFile( L"../LocDump.txt" , eg_lib_file_t::OS , TextFileData );
#endif
	}

private:

	void Init_LoadLocFile()
	{
		EGAlwaysLoadedFilenameList FileList;
		AlwaysLoaded_GetFileList( &FileList , "eloc" );

		for( const egAlwaysLoadedFilename* Filename : FileList )
		{
			if( Filename )
			{
				Init_LoadLocFile_File( Filename->Filename );
			}
		}

		Init_CreateDb();
	}

	void Init_CreateDb()
	{
		eg_size_t TotalEntries = 0;
		for( eg_uint i=0; i<m_LocChunks.Len(); i++ )
		{
			TotalEntries += m_LocChunks.GetByIndex(i)->NumEntries;
		}

		m_LocDb.Init( TotalEntries );

		for( eg_uint i=0; i<m_LocChunks.Len(); i++ )
		{
			egLocChunk* Chunk = m_LocChunks.GetByIndex( i );
			EGFileData ChunkReader( eg_file_data_init_t::SetableUserPointer );
			ChunkReader.SetData( reinterpret_cast<eg_byte*>(Chunk->Chunk) , Chunk->ChunkSize );

			//Reading the data must match the writing of data in locdb.cpp of egresource.exe.
			ChunkReader.Seek( eg_file_data_seek_t::Begin , 0 );
			//Now just do the binary read.
			eg_size_t NumEntries = 0;
			eg_size_t SizeRead = 0;

			eg_loc_lang Lang = eg_loc_lang::ENUS;
			SizeRead = ChunkReader.Read( &Lang , sizeof(eg_loc_lang) );
			assert( Lang == m_Lang ); // We shouldn't have gotten this far if the language for this file was not the same.
			SizeRead = ChunkReader.Read( &NumEntries , sizeof(NumEntries) );
			assert( sizeof(NumEntries) == SizeRead );
			assert( NumEntries == Chunk->NumEntries ); // something went wrong?

			for( eg_uint i=0; i<NumEntries; i++ )
			{
				egLocItem NewItem;
				SizeRead = ChunkReader.Read( &NewItem.Key , sizeof(NewItem.Key) );
				assert( SizeRead == sizeof(NewItem.Key) );
				SizeRead = ChunkReader.Read( &NewItem.Length , sizeof(NewItem.Length) );
				assert( SizeRead == sizeof(NewItem.Length) ); //Something may have changed.

				// Little weird here, we don't actually read this data, we just set the pointer.
				NewItem.String = ChunkReader.GetDataAtReadPosAs<eg_loc_char>();
				ChunkReader.Seek(eg_file_data_seek_t::Current , static_cast<eg_int>(NewItem.Length*sizeof(eg_loc_char)) );

				if( m_LocDb.Contains( NewItem.Key ) && !EGString_EqualsCount(m_LocDb[NewItem.Key].String , NewItem.String , NewItem.Length) )
				{
					EGLogf( eg_log_t::Warning , "The loc key 0x%08X appears more than once, some text will be incorrect." , NewItem.Key.ToUint32() );
					eg_char MbString1[1024];
					EGString_Copy( MbString1 , m_LocDb[NewItem.Key].String , EG_Min( countof(MbString1) , m_LocDb[NewItem.Key].Length+1 ) );
					eg_char MbString2[1024];
					EGString_Copy( MbString2 , NewItem.String , EG_Min( countof(MbString1) , NewItem.Length+1 ) );

					EGLogf( eg_log_t::Warning , "\"%s\" and \"%s\"" , MbString1 , MbString2 );
					assert( false );
				}
				else
				{
					m_LocDb.Insert( NewItem.Key , NewItem );
				}
			}
		}

		if( m_LocDb.Len() != TotalEntries )
		{
			EGLogf( eg_log_t::Warning , "There are a few duplicate strings in the loc database." );
		}
	}

	void Init_LoadLocFile_File( eg_cpstr Filename )
	{
		//First thing to do is to load our localization table
		EGFileData LoadedData( eg_file_data_init_t::HasOwnMemory );
		MainLoader->LoadNowTo( Filename , LoadedData );

		if( LoadedData.GetSize() != 0 )
		{
			eg_loc_lang Langauge = eg_loc_lang::ENUS;
			LoadedData.Seek( eg_file_data_seek_t::Begin , 0 );
			LoadedData.Read( &Langauge , sizeof(eg_loc_lang) );
			if( Langauge == m_Lang )
			{
				egLocChunk* NewLocChunk = new ( eg_mem_pool::System ) egLocChunk( LoadedData.GetSize() );
				if( NewLocChunk && NewLocChunk->ChunkSize >= LoadedData.GetSize() )
				{
					m_LocChunks.InsertLast( NewLocChunk );

					assert( LoadedData.GetSize() == NewLocChunk->ChunkSize );
					EGMem_Copy( NewLocChunk->Chunk , LoadedData.GetData() , LoadedData.GetSize() );

					// The number of entires should be an eg_size_t at the beginning.
					EGMem_Copy( &NewLocChunk->NumEntries , static_cast<const eg_byte*>(NewLocChunk->Chunk) + sizeof(eg_loc_lang) , sizeof(eg_size_t) );
				}
				else
				{
					assert( false ); // Out of memory?
				}
			}
		}
		else
		{
			EGLogf( eg_log_t::Error , __FUNCTION__ ": Localization file \"%s\" is missing. Text may not display correctly." , Filename );
			//assert(false); //failed to load localization file, no text will be localized.
		}
	}

	static eg_loc_text LocalizeFunction( const eg_string_crc& TextCrc )
	{
		eg_loc_text Out;
		EGLocalize_Localize( TextCrc, Out );
		return Out;
	}

private:
	EGLocDb            m_LocDb;
	EGList<egLocChunk> m_LocChunks;
	eg_loc_lang        m_Lang;
	eg_bool            m_Initialized:1;

} EGLocalize_Data;

void EGLocalize_Init( eg_loc_lang Language )
{
	EGLocalize_Data.Init( Language );
}

void EGLocalize_Deinit( void )
{
	EGLocalize_Data.Deinit();
}

void EGLocalize_Localize( eg_string_crc TextCrc , class eg_loc_text& LocTextOut   )
{
	EGLocalize_Data.Localize( TextCrc , LocTextOut );
}

void EGLocalize_DumpDb()
{
	EGLocalize_Data.DumpDb();
}


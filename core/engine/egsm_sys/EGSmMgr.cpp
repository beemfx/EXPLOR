// (c) 2017 Beem Media

#include "EGSmMgr.h"
#include "EGAlwaysLoaded.h"
#include "EGSmFile2.h"
#include "EGFileData.h"
#include "EGSmProc.h"
#include "EGSmVars.h"
#include "EGLoader.h"
#include "EGLibExtern.h"

static class EGSmMgr
{
private:

	EGList<EGSmProc> m_Procs;
	egsmVarDeclScr*  m_VarItems = nullptr;
	eg_size_t        m_VarItemsCount = 0;
	egsmVarDeclScr*  m_FnItems = nullptr;
	eg_size_t        m_FnItemsCount = 0;

public:

	EGSmMgr()
	: m_Procs( EGList<EGSmProc>::DEFAULT_ID )
	{

	}

	void Init()
	{
		InitScripts();
		InitGameState();
	}

	void Deinit()
	{
		while( m_Procs.HasItems() )
		{
			EGSmProc* Proc = m_Procs.GetOne();
			m_Procs.Remove( Proc );
			EGDeleteObject( Proc );
		}

		EG_SafeDeleteArray( m_VarItems );
		EG_SafeDeleteArray( m_FnItems );
	}

	const EGSmProc* GetProc( eg_string_crc ProcId )
	{
		for( EGSmProc* Proc : m_Procs )
		{
			if( Proc->GetId() == ProcId )
			{
				return Proc;
			}
		}
		return nullptr;
	}

	void GetVars( EGArray<egsmVarPair>& Out )
	{
		if( m_VarItems )
		{
			for( eg_size_t i=0; i<m_VarItemsCount; i++ )
			{
				egsmVarPair Pair;
				Pair.Id = eg_string_crc(m_VarItems[i].Name);
				Pair.Value = m_VarItems[i].DefaultValue;
				Out.Append( Pair );
			}
		}
	}

	eg_string_small VarCrcToStr( eg_string_crc Crc ) const
	{
		if( m_VarItems )
		{
			for( eg_size_t i=0; i<m_VarItemsCount; i++ )
			{
				if( eg_string_crc(m_VarItems[i].Name) == Crc )
				{
					return m_VarItems[i].Name;
				}
			}
		}
		return "(unknown)";
	}

	egsm_var_t GetVarType( eg_string_crc Crc ) const
	{
		if( m_VarItems )
		{
			for( eg_size_t i=0; i<m_VarItemsCount; i++ )
			{
				if( eg_string_crc(m_VarItems[i].Name) == Crc )
				{
					return m_VarItems[i].VarType;
				}
			}
		}
		return egsm_var_t::UNK;
	}

private:

	void InitGameState()
	{
		EGAlwaysLoadedFilenameList FileList;

		// Load all game state files
		{
			FileList.ClearFilenames();
			AlwaysLoaded_GetFileList( &FileList , "egclass" );

			EGArray<egsmVarDeclScr> VarList;
			EGArray<egsmVarDeclScr> FnList;

			for( const egAlwaysLoadedFilename* StateFile : FileList )
			{
				LoadGameStateFile( StateFile->Filename , VarList , FnList );
			}

			if( VarList.Len() > 0 )
			{
				m_VarItemsCount = VarList.Len();
				m_VarItems = new ( eg_mem_pool::System ) egsmVarDeclScr[ m_VarItemsCount ];
				if( m_VarItems )
				{
					for( eg_size_t i=0; i<m_VarItemsCount; i++ )
					{
						m_VarItems[i] = VarList[i];
					}
				}
				else
				{
					assert( false ); // Out of memory, no game vars!!!
					m_VarItemsCount = 0;
				}
			}

			if( FnList.Len() > 0 )
			{
				m_FnItemsCount = FnList.Len();
				m_FnItems = new ( eg_mem_pool::System )  egsmVarDeclScr[ m_FnItemsCount ];
				if( m_FnItems )
				{
					for( eg_size_t i=0; i<m_FnItemsCount; i++ )
					{
						m_FnItems[i] = FnList[i];
					}
				}
				else
				{
					assert( false ); // Out of memory, no game vars!!!
					m_FnItemsCount = 0;
				}
			}
		}
	}

	void LoadGameStateFile( eg_cpstr Filename , EGArray<egsmVarDeclScr>& VarList , EGArray<egsmVarDeclScr>& FnList )
	{
		EGFileData File( eg_file_data_init_t::HasOwnMemory );
		MainLoader->LoadNowTo( Filename , File );

		eg_size_t Pos = 0;
		const eg_char8* FileAsString = File.GetDataAs<eg_char8>();
		const eg_size_t FileAsStringLen = File.GetSize()/sizeof(FileAsString[0]);

		auto DoesItemExist = []( EGArray<egsmVarDeclScr>& VarList , eg_string_crc Id ) -> eg_bool
		{
			eg_bool bExists = false;

			for( eg_size_t i=0; i<VarList.Len() && !bExists; i++ )
			{
				if( eg_string_crc(VarList[i].Name) == Id )
				{
					bExists = true;
				}
			}

			return bExists;
		};

		egsmVarDeclCallback HandleVarCb = [&VarList,&FnList,&DoesItemExist]( const egsmVarDeclScr& VarDecl ) -> void
		{
			if( DoesItemExist( VarList , eg_string_crc(VarDecl.Name) ) )
			{
				EGLogf( eg_log_t::Error , __FUNCTION__ ": %s was declared more than once (or had the same crc)." , VarDecl.Name );
				assert( false );
			}
			else
			{
				if( VarDecl.DeclType == egsm_var_decl_t::VAR )
				{
					VarList.Append( VarDecl );
				}
				else if( VarDecl.DeclType == egsm_var_decl_t::FUNCTION )
				{
					FnList.Append( VarDecl );
				}
			}
		};

		EGSmVars_LoadVarDecl( FileAsString , FileAsStringLen , HandleVarCb );
	}

	void InitScripts()
	{
		EGAlwaysLoadedFilenameList FileList;
		// Load all conv files
		{
			FileList.ClearFilenames();
			AlwaysLoaded_GetFileList( &FileList , "egsmbin" );

			for( const egAlwaysLoadedFilename* File : FileList )
			{
				InsertScript( File->Filename );
			}
		}
	}

	void InsertScript( eg_cpstr Filename )
	{
		EGFileData ByteCode( eg_file_data_init_t::HasOwnMemory );
		EGLibExtern_LoadNowTo( Filename , ByteCode );

		if( ByteCode.GetSize() == 0 )
		{
			EGLogf( eg_log_t::Error , __FUNCTION__": The byte-code for %s could not be compiled." , Filename );
			assert( false );
			return;
		}

		EGSmProc* NewProc = EGNewObject<EGSmProc>( eg_mem_pool::System );
		if( NewProc )
		{
			eg_bool bInited = NewProc->Init( ByteCode.GetData() , ByteCode.GetSize() );

			if( bInited && GetProc(NewProc->GetId()) )
			{
				bInited = false;
				EGLogf( eg_log_t::Error , __FUNCTION__": Duplicate script id in %s they must be unique." , Filename );
				assert( false ); // Duplicate script id, they must be unique.
			}

			if( bInited )
			{
				m_Procs.Insert( NewProc );
			}
			else
			{
				EGDeleteObject( NewProc );
				EGLogf( eg_log_t::Error , __FUNCTION__": Failed to initialize %s from byte-code." , Filename );
			}
		}		
	}
}
SmMgr;

void EGSmMgr_Init()
{
	SmMgr.Init();
}

void EGSmMgr_Deinit()
{
	SmMgr.Deinit();
}

const EGSmProc* EGSmMgr_GetProc( eg_string_crc ProcId )
{
	return SmMgr.GetProc( ProcId );
}

void EGSmMgr_GetVars( EGArray<egsmVarPair>& Out )
{
	SmMgr.GetVars( Out );
}

eg_string_small EGSmMgr_VarCrcToStr( eg_string_crc Crc )
{
	return SmMgr.VarCrcToStr( Crc );
}

egsm_var_t EGSmMgr_GetVarType( eg_string_crc Crc )
{
	return SmMgr.GetVarType( Crc );
}

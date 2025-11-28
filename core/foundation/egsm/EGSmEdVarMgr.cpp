// (c) 2016 Beem Media

#include "EGSmEdVarMgr.h"
#include "EGFileData.h"
#include "EGSmFile2.h"
#include "EGToolsHelper.h"
#include "EGSmVars.h"
#include "EGWnd.h"
#include "EGResourceLib.h"

static class EGSmEdVarMgr
{
private:

	EGArray<egsmVarDeclScr> m_Vars;

public:

	void InitForFile( eg_cpstr16 Filename )
	{
		m_Vars.Clear();

		egsmVarDeclCallback HandleVarCb = [this]( const egsmVarDeclScr& VarDecl ) -> void
		{
			m_Vars.Append( VarDecl );
		};

		auto LoadEGStateFunctions = [&HandleVarCb,&Filename]() -> void
		{		
			eg_string_big Filename8( Filename );
			eg_string_big FileStr;

			FileStr.SetToDirectoryFromPath( Filename8 );
			eg_string_big SearchStr = FileStr;
			SearchStr.Append( "*.egclass" );

			WIN32_FIND_DATAA FindData;
			zero( &FindData );
			HANDLE hFind = FindFirstFileA( SearchStr , &FindData );

			if( INVALID_HANDLE_VALUE != hFind )
			{
				do 
				{
					eg_string_big FoundFile = EGString_Format( "%s%s" , FileStr.String() , FindData.cFileName );

					EGFileData VarFile( eg_file_data_init_t::HasOwnMemory );
					eg_char16 VarFilePath[256];
					FoundFile.CopyTo( VarFilePath , countof(VarFilePath) );
					EGToolsHelper_OpenFile( VarFilePath , VarFile );

					EGSmVars_LoadVarDecl( VarFile.GetDataAs<eg_char8>() , VarFile.GetSize()/sizeof(eg_char8) , HandleVarCb );

				} while ( FindNextFileA( hFind , &FindData) );
				FindClose( hFind );
			}
		};

		// Intrinsic functions:
		auto LoadIntrinsicFunctions = [&HandleVarCb]() -> void
		{
			HRSRC ResInfo = FindResourceW( EGResourceLib_GetLibrary() , L"EGSM_INTRISIC_FUNCTIONS_FILE" , RT_RCDATA );
			if( ResInfo != nullptr )
			{
				HGLOBAL Res = LoadResource( EGResourceLib_GetLibrary() , ResInfo );
				if( Res != nullptr )
				{
					const eg_char8* ResData = reinterpret_cast<const eg_char8*>(LockResource( Res ));
					const eg_size_t ResSize = SizeofResource( EGResourceLib_GetLibrary() , ResInfo );
					if( ResData && ResSize > 0 )
					{
						EGSmVars_LoadVarDecl( ResData , ResSize / sizeof( eg_char8 ) , HandleVarCb );	
					}
				}
			}
		};

		LoadEGStateFunctions();
		LoadIntrinsicFunctions();
	}

	void GetVarsOfType( egsm_var_t VarType, EGArray<egsmVarDeclScr>& Out )
	{
		for( const egsmVarDeclScr& Info : m_Vars )
		{
			if( Info.DeclType == egsm_var_decl_t::VAR )
			{
				if( Info.VarType == VarType )
				{
					Out.Append( Info );
				}
				else if( VarType == egsm_var_t::NUMBER && (Info.VarType == egsm_var_t::INT || Info.VarType == egsm_var_t::REAL) )
				{
					Out.Append( Info );
				}
			}
		}
	}

	void GetFunctions( EGArray<egsmVarDeclScr>& Out )
	{
		for( const egsmVarDeclScr& Info : m_Vars )
		{
			if( Info.DeclType == egsm_var_decl_t::FUNCTION )
			{
				Out.Append( Info );
			}
		}
	}

	egsmVarDeclScr GetFunctionInfo( eg_string_crc Name )
	{
		for( const egsmVarDeclScr& Info : m_Vars )
		{
			if( Name == eg_string_crc(Info.Name) )
			{
				return Info;
			}
		}

		egsmVarDeclScr Garbage;
		Garbage.DeclType = egsm_var_decl_t::UNK;
		Garbage.ReturnType = egsm_var_t::UNK;
		return Garbage;
	}

	void Clear()
	{
		m_Vars.Clear();
	}
}
SmVarMgr;

void EGSmEdVarMgr_InitForFile( eg_cpstr16 Filename )
{
	SmVarMgr.InitForFile( Filename );
}

void EGSmEdVarMgr_GetVarsOfType( egsm_var_t VarType, EGArray<egsmVarDeclScr>& Out )
{
	SmVarMgr.GetVarsOfType( VarType , Out );
}

void EGSmEdVarMgr_GetFunctions( EGArray<egsmVarDeclScr>& Out )
{
	SmVarMgr.GetFunctions( Out );
}

egsmVarDeclScr EGSmEdVarMgr_GetFunctionInfo( eg_string_crc Name )
{
	return SmVarMgr.GetFunctionInfo( Name );
}

void EGSmEdVarMgr_Deinit()
{
	SmVarMgr.Clear();
}

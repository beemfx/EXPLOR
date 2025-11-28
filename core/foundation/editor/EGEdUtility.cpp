// (c) 2018 Beem Media

#include "EGEdUtility.h"
#include "EGToolsHelper.h"
#include "../../tools/EGScc/EGPerforce.h"
#include "EGWindowsAPI.h"
#include "EGOsFile.h"
#include "EGPath2.h"
#include "EGLibFile.h"
#include "EGFileData.h"
#include "EGFileData.h"

class EGEdUtility
{
private:

	typedef eg_bool ( * egCheckoutFileFn )( eg_cpstr16 Filename , EGPerforceLogFn LogFn );
	typedef eg_bool ( * egRevertFileFn )( eg_cpstr16 Filename , eg_bool bOnlyIfUnchanged , EGPerforceLogFn LogFn );
	typedef eg_bool ( * egAddFileFn )( eg_cpstr16 Filename , EGPerforceLogFn LogFn );
	typedef eg_bool ( * egDeleteFileFn )( eg_cpstr16 Filename , EGPerforceLogFn LogFn );

private:

	HMODULE m_P4Lib = nullptr;
	egCheckoutFileFn m_CheckoutFile = nullptr;
	egRevertFileFn m_RevertFile = nullptr;
	egAddFileFn m_AddFile = nullptr;
	egDeleteFileFn m_DeleteFile = nullptr;
	eg_int m_InitCount = 0;

public:

	void Init()
	{
		if( 0 == m_InitCount )
		{
			m_CheckoutFile = EGFakeSourceControl_CheckoutFile;
			m_RevertFile = EGFakeSourceControl_RevertFile;
			m_AddFile = EGFakeSourceControl_AddFile;
			m_DeleteFile = EGFakeSourceControl_DeleteFile;

			// EX-Release: No longer connecting to source control.
			/*
			#if defined( __WIN64__ )
			m_P4Lib = LoadLibraryW( L"EGScc_x64.dll");
			#elif defined( __WIN32__ )
			m_P4Lib = LoadLibraryW( L"EGScc_x86.dll");
			#endif

			if( m_P4Lib )
			{
				m_CheckoutFile = reinterpret_cast<egCheckoutFileFn>(GetProcAddress( m_P4Lib , "EGPerforce_CheckoutFile" ));
				m_RevertFile = reinterpret_cast<egRevertFileFn>(GetProcAddress( m_P4Lib , "EGPerforce_RevertFile" ));
				m_AddFile = reinterpret_cast<egAddFileFn>(GetProcAddress( m_P4Lib , "EGPerforce_AddFile" ));
				m_DeleteFile = reinterpret_cast<egDeleteFileFn>(GetProcAddress( m_P4Lib , "EGPerforce_DeleteFile" ));
			}
			*/
		}

		m_InitCount++;
	}

	void Deinit()
	{
		m_InitCount--;
		assert( m_InitCount >= 0 );

		if( 0 == m_InitCount )
		{
			m_CheckoutFile = nullptr;
			m_RevertFile = nullptr;
			m_AddFile = nullptr;
			m_DeleteFile = nullptr;

			if( m_P4Lib )
			{
				FreeLibrary( m_P4Lib );
				m_P4Lib = nullptr;
			}
		}
	}

	eg_bool SaveFile( eg_cpstr16 Filename, const EGFileData& OutFile )
	{
		if( m_CheckoutFile )
		{
			EGPerforceLogFn LogFn = []( eg_cpstr String ) -> void
			{
				EGLogf( eg_log_t::SourceControl, "%s", String );
			};

			const eg_bool bCheckedOut = m_CheckoutFile( Filename, LogFn );
		}

		eg_bool bWasSaved = EGToolsHelper_SaveFile( Filename, OutFile );
		if( bWasSaved )
		{
			AddFile( Filename );
		}
		return bWasSaved;
	}

	eg_bool CheckoutFile( eg_cpstr16 Filename )
	{
		eg_bool bCheckedOut = false;

		if( m_CheckoutFile )
		{
			EGPerforceLogFn LogFn = []( eg_cpstr String ) -> void
			{
				EGLogf(  eg_log_t::SourceControl , "%s" , String );
			};

			bCheckedOut = m_CheckoutFile( Filename , LogFn );
		}

		return bCheckedOut;
	}

	eg_bool RevertFile( eg_cpstr16 Filename, eg_bool bOnlyIfUnchanged )
	{
		eg_bool bReverted = false;

		if( m_RevertFile )
		{
			EGPerforceLogFn LogFn = []( eg_cpstr String ) -> void
			{
				EGLogf(  eg_log_t::SourceControl , "%s" , String );
			};

			bReverted = m_RevertFile( Filename , bOnlyIfUnchanged , LogFn );
		}

		return bReverted;
	}

	eg_bool AddFile( eg_cpstr16 Filename )
	{
		eg_bool bAdded = false;

		if( m_AddFile )
		{
			EGPerforceLogFn LogFn = []( eg_cpstr String ) -> void
			{
				EGLogf(  eg_log_t::SourceControl , "%s" , String );
			};

			bAdded = m_AddFile( Filename , LogFn );
		}

		return bAdded;
	}

	eg_bool DeleteFile( eg_cpstr16 Filename )
	{
		eg_bool bAdded = false;

		if( m_DeleteFile )
		{
			EGPerforceLogFn LogFn = []( eg_cpstr String ) -> void
			{
				EGLogf(  eg_log_t::SourceControl , "%s" , String );
			};

			bAdded = m_DeleteFile( Filename , LogFn );
		}

		return bAdded;
	}

	static eg_bool EGFakeSourceControl_CheckoutFile(eg_cpstr16 Filename , EGPerforceLogFn LogFn)
	{
		unused(LogFn);

		if (EGOsFile_DoesFileExist(Filename))
		{
			if (EGOsFile_RemoveReadOnlyAttribute(Filename))
			{
				return true;
			}
		}

		return false;
	}

	static eg_bool EGFakeSourceControl_RevertFile(eg_cpstr16 Filename , eg_bool bOnlyIfUnchanged , EGPerforceLogFn LogFn)
	{
		unused(Filename, bOnlyIfUnchanged, LogFn);
		return false;
	}

	static eg_bool EGFakeSourceControl_AddFile(eg_cpstr16 Filename , EGPerforceLogFn LogFn)
	{
		unused(LogFn);
		return EGOsFile_DoesFileExist(Filename);;
	}

	static eg_bool EGFakeSourceControl_DeleteFile(eg_cpstr16 Filename , EGPerforceLogFn LogFn)
	{
		unused(LogFn);
		EGOsFile_RemoveReadOnlyAttribute(Filename);
		EGOsFile_DeleteFile(Filename);
		return !EGOsFile_DoesFileExist(Filename);
	}
};

static EGEdUtility EdUtility;

void EGEdUtility_Init()
{
	EdUtility.Init();
}

void EGEdUtility_Deinit()
{
	EdUtility.Deinit();
}

eg_bool EGEdUtility_SaveFile( eg_cpstr16 Filename, const EGFileData& OutFile )
{
	return EdUtility.SaveFile( Filename , OutFile );
}

eg_bool EGEdUtility_CheckoutFile( eg_cpstr16 Filename )
{
	return EdUtility.CheckoutFile( Filename );
}

eg_bool EGEdUtility_RevertFile( eg_cpstr16 Filename, eg_bool bOnlyIfUnchanged )
{
	return EdUtility.RevertFile( Filename , bOnlyIfUnchanged );
}

eg_bool EGEdUtility_AddFile( eg_cpstr16 Filename )
{
	return EdUtility.AddFile( Filename );
}


eg_bool EGEdUtility_DeleteFile( eg_cpstr16 Filename )
{
	return EdUtility.DeleteFile( Filename );
}

void EGEdUtility_CleanGameData( eg_bool bConfirm )
{
	eg_d_string OutRoot = EGToolsHelper_GetEnvVar("EGOUT").String();
	eg_d_string GameName = EGToolsHelper_GetEnvVar("EGGAME").String();
	eg_d_string DataBuildPath = EGPath2_CleanPath( EGString_Format( "%s/databuild/" , *OutRoot ) , '/' );
	eg_d_string CoreBuiltData = EGPath2_CleanPath( EGString_Format( "%s/bin/egdata/" , *OutRoot ) , '/' );
	eg_d_string GameBuiltData = EGPath2_CleanPath( EGString_Format( "%s/bin/%s/" , *OutRoot , *GameName ) , '/' );
	eg_d_string DiffDbData = EGPath2_CleanPath( EGString_Format( "%s/dbdiff/" , *OutRoot ) , '/' );

	EGLogf( eg_log_t::General , "Cleaning game data for %s..." , *GameName );

	eg_d_string Question = EGString_Format( "Are you sure you want to delete these these folders?\n%s\n%s\n%s\n%s" , *DataBuildPath , *CoreBuiltData , *GameBuiltData , *DiffDbData );
	if( !bConfirm || IDYES == MessageBoxA( nullptr , *Question , "EG Editor" , MB_YESNO|MB_ICONQUESTION|MB_SYSTEMMODAL ) )
	{
		EGOsFile_DeleteDirectory( *DataBuildPath );
		EGOsFile_DeleteDirectory( *CoreBuiltData );
		EGOsFile_DeleteDirectory( *GameBuiltData );
		EGOsFile_DeleteDirectory( *DiffDbData );
	};

	EGLogf( eg_log_t::General , "Game data deleted." );
}

eg_bool EGEdUtility_SaveIfUnchanged( eg_cpstr Filename , const EGFileData& FileData , eg_bool bCheckInToScc /*= false */ )
{
	return EGEdUtility_SaveIfUnchanged( *eg_d_string16( Filename ) , FileData , bCheckInToScc );
}

eg_bool EGEdUtility_SaveIfUnchanged( eg_cpstr16 Filename , const EGFileData& FileData , eg_bool bCheckInToScc /*= false */ )
{
	eg_bool bDoesFileExist = EGOsFile_DoesFileExist( Filename );
	eg_bool bHasChanged = true;

	if( bDoesFileExist )
	{
		EGFileData CurrentFileData( eg_file_data_init_t::HasOwnMemory );
		EGLibFile_OpenFile( Filename , eg_lib_file_t::OS , CurrentFileData );
		bHasChanged = !FileData.Equals( CurrentFileData );
	}

	eg_bool bSuccess = true;
	if( bHasChanged )
	{
		EGLogf( eg_log_t::General, "Writing to \"%s\"...", *eg_d_string8(Filename) );
		if( bCheckInToScc )
		{
			bSuccess = EGEdUtility_SaveFile( Filename , FileData );
			EGEdUtility_AddFile( Filename );
		}
		else
		{
			bSuccess = EGLibFile_SaveFile( Filename , eg_lib_file_t::OS , FileData );
		}
	}
	else
	{
		EGLogf( eg_log_t::Verbose , "\"%s\" was unchanged." , Filename );
	}
	return bSuccess;
}

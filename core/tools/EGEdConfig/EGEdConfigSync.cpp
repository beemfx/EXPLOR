// (c) 2018 Beem Media

#include "EGEdConfigSync.h"
#include "EGEdConfig.h"
#include "EGWndAppWindow.h"
#include "EGThread.h"
#include "EGThreadProc.h"
#include "EGNetCore.h"
#include "EGFileData.h"
#include "EGLibFile.h"
#include "EGPath2.h"
#include "EGOsFile.h"
#include "EGEdUtility.h"
#include "EGSync.h"
#include <fs_sys2/fs_lpk.h>

static void* EGEdConfigSync_Malloc(fs_size_t size, LF_ALLOC_REASON reason, const fs_char8*const type, const fs_char8*const file, const fs_uint line)
{
	unused( reason , type , file , line );
	return new fs_byte[size];
}

static void EGEdConfigSync_Free(void* p, LF_ALLOC_REASON reason)
{
	unused( reason );
	delete[]p;
}

static void EGEdConfigSync_SaveFile( eg_cpstr FullPath , const EGArray<eg_byte>& FileData )
{
	EGLogf( eg_log_t::Verbose , "\t-> %s" , FullPath );

	eg_d_string FileDir = *EGPath2_BreakPath( FullPath ).GetDirectory( '/' );
	eg_bool bCreatedFileDir = EGOsFile_CreateDirectory( *FileDir );

	if( bCreatedFileDir )
	{
		EGFileData FileDataMemFile( FileData );
		eg_bool bSaved = EGEdUtility_SaveIfUnchanged( FullPath , FileDataMemFile , false );
		if( bSaved )
		{
		
		}
		else
		{
			EGLogf( eg_log_t::Error , "Could not save output file." );
		}
	}
	else
	{
		EGLogf( eg_log_t::Error , "Could not create directory for file." );
	}
}

static void EGEdConfigSync_ExtractFile( eg_cpstr OutDir , const EGArray<eg_byte>& ArchiveData )
{
	eg_d_string FullOutDir = OutDir;
	FullOutDir += "/bin/";
	FullOutDir = EGPath2_CleanPath( *FullOutDir , '/' );
	const eg_bool bCreatedRootDir = EGOsFile_CreateDirectory( *FullOutDir );

	if( bCreatedRootDir )
	{
		FS_SetMemFuncs( EGEdConfigSync_Malloc , EGEdConfigSync_Free );
		{
			CLArchive FileArchive;
			eg_bool bOpenedArchive = FS_TRUE == FileArchive.Open( ArchiveData.GetArray() , ArchiveData.LenAs<fs_dword>() );
			if( bOpenedArchive )
			{
				const eg_uint NumFiles = FileArchive.GetNumFiles();
				for( eg_uint i=0; i<NumFiles; i++ )
				{
					const LPK_FILE_INFO* FileInfo = FileArchive.GetFileInfo( i );
					if( FileInfo )
					{
						eg_s_string_big16 OutFile = FileInfo->szFilename;
						EGLogf( eg_log_t::General , "Extracting \"%s\"..." , EGString_ToMultibyte(*OutFile) );
						EGArray<eg_byte> FileData;
						FileData.Resize( FileInfo->nSize );
						if( FileData.Len() == FileInfo->nSize )
						{
							eg_bool bExtractedFile = FS_TRUE == FileArchive.ExtractFile( i , FileData.GetArray() , FileData.LenAs<fs_dword>() );
							if( bExtractedFile )
							{
								eg_d_string FullFileOut = EGString_Format( "%s/%s" , *FullOutDir , EGString_ToMultibyte(*OutFile) );
								FullFileOut = EGPath2_CleanPath( *FullFileOut , '/' );
								EGEdConfigSync_SaveFile( *FullFileOut , FileData );
							}
							else
							{
								EGLogf( eg_log_t::Error , "Could not extract file." );
							}
						}
						else
						{
							EGLogf( eg_log_t::Error , "Could not allocate memory for file extraction." );
						}
					}
				}
			}
			else
			{
				EGLogf( eg_log_t::Error , "Could not open archive file." );
			}
		}
		FS_SetMemFuncs( nullptr , nullptr );
	}
	else
	{
		EGLogf( eg_log_t::Error , "Could not create root directory." );
	}
}

static eg_bool EGEdConfigSync_IsCharAllowed( eg_char Char )
{
	if( EG_IsBetween( Char , 'A' , 'Z' ) )
	{
		return true;
	}

	if( EG_IsBetween( Char , 'a' , 'z' ) )
	{
		return true;
	}

	if( EG_IsBetween( Char , '0' , '9' ) )
	{
		return true;
	}

	static const eg_char AdditionalChars[] = "_-.:";

	for( const eg_char& CompareChar : AdditionalChars )
	{
		if( Char == CompareChar )
		{
			return true;
		}
	}

	return false;
}

static void EGEdConfigSync_ReadDataFileToString( const EGArray<eg_byte>& Data , eg_d_string& Out )
{
	EGFileData DataFile( Data );
	DataFile.Seek( eg_file_data_seek_t::Begin , 0 );
	Out.Clear();
	eg_char NextChar;
	while( DataFile.Read( &NextChar , sizeof(NextChar) ) )
	{
		if( EGEdConfigSync_IsCharAllowed( NextChar ) )
		{
			Out.Append( NextChar );
		}
		else
		{
			break;
		}
	}
}

static void EGEdConfigSync_DoSync_Binaries( eg_cpstr SyncHost , eg_cpstr OutDir )
{
	EGLogf( eg_log_t::General , "Obtaining manifest..." );

	EGArray<eg_byte> DistributionData;
	eg_d_string16 FullSyncPath = SyncHost;
	FullSyncPath += L"/sync/bindistid.html";

	const eg_bool bGotManifest = EGNetCore_MakeRequest( *FullSyncPath , eg_d_string8() , DistributionData ); 
	if( bGotManifest )
	{
		EGLogf( eg_log_t::General , "Manifest obtained." );

		eg_d_string DistributionString;
		EGEdConfigSync_ReadDataFileToString( DistributionData, DistributionString );
		eg_cpstr Prefix = "DistID:";
		const eg_size_t PrefixLen = EGString_StrLen( Prefix );
		if( DistributionString.Len() > PrefixLen && EGString_EqualsCount( *DistributionString, Prefix, PrefixLen ) )
		{
			eg_int DistributionValue = EGString_ToInt( &DistributionString[PrefixLen] );

			eg_d_string SyncFile = EGString_Format( "bin_%d/bin.lpk" , DistributionValue );
		
			EGLogf( eg_log_t::General , "Syncing \"%s\"..." , *SyncFile );
			EGArray<eg_byte> DataFile;
			eg_d_string16 FullFilePath = SyncHost;
			FullFilePath += L"/sync/";
			FullFilePath += *SyncFile;

			eg_bool bGotSyncFile = EGNetCore_MakeRequest( *FullFilePath , eg_d_string8() , DataFile );
			if( bGotSyncFile )
			{
				EGEdConfigSync_ExtractFile( OutDir , DataFile );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Failed to get sync file." );
			}
		}
		else
		{
			EGLogf( eg_log_t::Error , "Failed to get manifest." );
		}
	}
	else
	{
		EGLogf( eg_log_t::Error , "Failed to get manifest." );
	}
}

static void EGEdConfigSync_DoSync_Full( eg_cpstr SyncHost , eg_cpstr OutDir , std::function<eg_bool()>& IsCancelled )
{
	EGLogf( eg_log_t::General , "Obtaining distribution data..." );

	EGArray<eg_byte> DistributionData;
	eg_d_string16 SyncRootUrl = SyncHost;
	SyncRootUrl += L"/sync/";
	eg_d_string16 DistributionFilePath = SyncRootUrl + L"fulldistid.html";

	eg_d_string TempDir = OutDir;
	TempDir += "/sync_files/";

	eg_d_string BinDir = OutDir;
	BinDir += "/bin/";

	EGOsFile_CreateDirectory( *TempDir );
	EGOsFile_ClearDirectory( *TempDir );

	const eg_bool bGotDistInfo = EGNetCore_MakeRequest( *DistributionFilePath , eg_d_string8() , DistributionData ); 
	if( bGotDistInfo )
	{
		eg_d_string DistributionString;
		EGEdConfigSync_ReadDataFileToString( DistributionData , DistributionString );
		eg_cpstr Prefix="DistID:";
		const eg_size_t PrefixLen = EGString_StrLen( Prefix );
		if( DistributionString.Len() > PrefixLen && EGString_EqualsCount( *DistributionString , Prefix , PrefixLen ) )
		{
			eg_int DistributionValue = EGString_ToInt( &DistributionString[PrefixLen] );
		
			eg_d_string16 DistRootUrl = SyncRootUrl + "dist_";
			DistRootUrl += EGString_Format( "%d/" , DistributionValue ).String();

			eg_d_string16 FileManifestUrl = DistRootUrl + "manifest.html";
			EGArray<eg_byte> FileData;
			const eg_bool bGotManifest = EGNetCore_MakeRequest( *FileManifestUrl , eg_d_string8() , FileData );
			if( bGotManifest )
			{
				eg_d_string OutputFilename;
				OutputFilename = TempDir + "manifest.html";
				EGLibFile_SaveFile( *OutputFilename , eg_lib_file_t::OS , FileData );
				EGArray<eg_s_string_sml8> Manifest;
				EgSync_ReadManifest( FileData , Manifest );

				{
					eg_d_string16 FileUrl;

					for( const eg_s_string_sml8& Filename : Manifest )
					{
						if( IsCancelled() )
						{
							EGLogf( eg_log_t::General , "Sync canceled." );
							return;
						}
						EGLogf( eg_log_t::General , "Downloading \"%s\"..." , *Filename );
						FileUrl = DistRootUrl + *Filename;
						FileData.Clear();
						const eg_bool bGotFile = EGNetCore_MakeRequest( *FileUrl , eg_d_string8() , FileData );
						if( bGotFile )
						{
							OutputFilename = TempDir + *Filename;
							EGLibFile_SaveFile( *OutputFilename , eg_lib_file_t::OS , FileData );
						}
						else
						{
							EGLogf( eg_log_t::Error , "Failed to download \"%s\"." , *Filename );
							return;
						}
					}
				}

				EGLogf( eg_log_t::General , "Assembling big file..." );
				eg_d_string BigFilename = TempDir + "fulldata.egbig";
				EGSync_AssembleBigFile( *TempDir , *BigFilename );
				EGSync_UnpackBigFile( *BigFilename , *BinDir );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Bad data in manifest." );
			}
		}
		else
		{
			EGLogf( eg_log_t::Error , "Failed to get manifest." );
		}
	}
	else
	{
		EGLogf( eg_log_t::Error , "Failed to get distribution data." );
	}
}

static void EGEdConfigSync_DoSync( eg_cpstr SyncHost , eg_cpstr OutDir , eg_bool bBinariesOnly , std::function<eg_bool()>& IsCancelled )
{
	EGLogf( eg_log_t::General , "Syncing data..." );

	EGNetCore_Init();

	if( bBinariesOnly )
	{
		EGEdConfigSync_DoSync_Binaries( SyncHost , OutDir );
	}
	else
	{
		EGEdConfigSync_DoSync_Full( SyncHost , OutDir , IsCancelled );
	}

	EGNetCore_Deinit();
}

class EGEdConfigSyncProc : public IThreadProc
{
private:

	mutable EGMutex m_ReadMutex;
	const eg_d_string m_SyncHost;
	const eg_d_string m_OutDir;
	const eg_bool m_bBinariesOnly;
	eg_bool m_bIsDone = false;
	eg_bool m_bCancelled = false;

public:

	EGEdConfigSyncProc( eg_cpstr InSyncHost , eg_cpstr InOutDir , eg_bool bInBinariesOnly )
	: m_SyncHost( InSyncHost )
	, m_OutDir( InOutDir )
	, m_bBinariesOnly( bInBinariesOnly )
	{

	}

	virtual void OnStart() override
	{
		assert( !m_bIsDone ); // Should have started not done.


		EGLogf( eg_log_t::General , "Syncing data from %s..." , *m_SyncHost );

		std::function<eg_bool()> IsCancelled = [this]() -> eg_bool
		{
			eg_bool Out = false;
			m_ReadMutex.Lock();
			Out = m_bCancelled;
			m_ReadMutex.Unlock();
			return Out;
		};

		EGEdConfigSync_DoSync( *m_SyncHost , *m_OutDir , m_bBinariesOnly , IsCancelled );
		
		EGLogf( eg_log_t::General , "Sync complete." );

		m_ReadMutex.Lock();
		m_bIsDone = true;
		m_ReadMutex.Unlock();
	}

	eg_bool IsDone() const { EGFunctionLock Lock( &m_ReadMutex ); return m_bIsDone; }

	eg_bool Cancel()
	{
		EGFunctionLock Lock( &m_ReadMutex );
		m_bCancelled = true;
	}
};


EGEdConfigSyncPanel::EGEdConfigSyncPanel( EGWndPanel* Parent , EGWndAppWindow* InAppOwner )
: Super( Parent, eg_panel_size_t::Auto, 0 )
, m_AppOwner( *InAppOwner )
{

}

EGEdConfigSyncPanel::~EGEdConfigSyncPanel()
{
	assert( !m_bIsSyncing && nullptr == m_WorkerThread && nullptr == m_WorkerProc ); // Should not have been able to quit while syncing.
}

void EGEdConfigSyncPanel::BeginSync( eg_cpstr SyncHost , eg_cpstr OutDir , eg_bool bBinariesOnly )
{
	if( m_bIsSyncing )
	{
		assert( false );
		return;
	}

	if( EGString_StrLen( SyncHost ) == 0 )
	{
		MessageBoxW( GetWnd() , L"A host (EGSYNCHOST) is required to sync game data." , L"Notice" , MB_ICONINFORMATION|MB_OK );
		PostMessageW( m_AppOwner.GetWnd() , WM_EGEDCONFIG_SYNC_DONE , 0 , 0 );
		return;
	}

	if( EGString_StrLen( OutDir ) == 0 )
	{
		MessageBoxW( GetWnd() , L"An output directory (EGOUT) is required to sync game data." , L"Notice" , MB_ICONINFORMATION|MB_OK );
		PostMessageW( m_AppOwner.GetWnd() , WM_EGEDCONFIG_SYNC_DONE , 0 , 0 );
		return;
	}

	m_bIsSyncing = true;
	m_WorkerThread = new EGThread( EGThread::egInitParms( "SyncThread" , EGThread::eg_init_t::LowPriority ) );
	m_WorkerProc = new EGEdConfigSyncProc( SyncHost , OutDir , bBinariesOnly );

	if( m_WorkerThread && m_WorkerProc )
	{
		m_WorkerThread->RegisterProc( m_WorkerProc );
		m_WorkerThread->Start();
	}

	SetWndTimer( POLL_SYNC_DONE_TIMER_ID , 333 );
	OnWmTimer( POLL_SYNC_DONE_TIMER_ID ); // Force an update right away.
}

void EGEdConfigSyncPanel::OnWmTimer( eg_uint64 TimerId )
{
	if( TimerId == POLL_SYNC_DONE_TIMER_ID )
	{
		assert( m_bIsSyncing );

		if( nullptr == m_WorkerProc || m_WorkerProc->IsDone() )
		{
			if( m_WorkerThread )
			{
				m_WorkerThread->Stop();
				m_WorkerThread->UnregisterProc( m_WorkerProc );
				EG_SafeDelete( m_WorkerProc );
				EG_SafeDelete( m_WorkerThread );
			}

			KillWndTimer( POLL_SYNC_DONE_TIMER_ID );
			m_bIsSyncing = false;
			PostMessageW( m_AppOwner.GetWnd() , WM_EGEDCONFIG_SYNC_DONE , 0 , 0 );
		}

		m_BuildElipses++;
		m_BuildElipses %= 4;
		m_DisplayText = CT_Clear;
		for( eg_int i=0; i<m_BuildElipses; i++ )
		{
			m_DisplayText += ".";
		}
		m_DisplayText += "Syncing";
		for( eg_int i=0; i<m_BuildElipses; i++ )
		{
			m_DisplayText += ".";
		}
		FullRedraw();
	}
}

void EGEdConfigSyncPanel::OnDrawBg( HDC hdc )
{
	FillRect( hdc, &GetViewRect(), EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
}

void EGEdConfigSyncPanel::OnPaint( HDC hdc )
{
	const RECT rcView = GetViewRect();
	SetBkMode( hdc, TRANSPARENT );
	SetBrush( hdc, EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
	SetFont( hdc, EGWnd_GetFont( egw_font_t::DEFAULT ) );
	SetTextColor( hdc, EGWnd_GetColor( egw_color_t::FG_STATIC ) );
	//Rectangle( hdc , rcView.left , rcView.top , rcView.right , rcView.bottom );
	RECT rcText = rcView;
	rcText.left += 2;
	rcText.top += 1;
	DrawTextA( hdc, *m_DisplayText , m_DisplayText.LenAs<int>() , &rcText , DT_SINGLELINE|DT_CENTER|DT_VCENTER );
}

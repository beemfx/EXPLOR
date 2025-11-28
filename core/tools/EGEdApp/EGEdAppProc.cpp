// (c) 2018 Beem Media

#include "EGEdAppProc.h"
#include "EGEdApp.h"
#include "EGParse.h"
#include "EGLocAssets.h"
#include "EGToolsHelper.h"
#include "EGExtProcess.h"
#include "EGEdImportData.h"
#include "EGPath2.h"
#include "EGOsFile.h"
#include "EGWindowsAPI.h"
#include "EGEdUtility.h"

static void EGEdAppProc_BuildWebServer()
{
	EGExtProcess_Run( "egmake2_x64 MAKEWEBSERVICE" , nullptr );
}

void EGEdAppProc::Update( eg_real DeltaTime )
{
	unused( DeltaTime );

	if( HasCommands() )
	{
		SetIsExecuting( true );

		eg_d_string CurrentCommand = "";
		{
			EGFunctionLock FnLock( &m_Mutex );
			if( m_bWantsCancel )
			{
				m_bWantsCancel = false;
				m_MessageQueue.Clear();
			}
			else
			{
				CurrentCommand = m_MessageQueue[0];
				m_MessageQueue.DeleteByIndex( 0 );
			}
		}

		egParseFuncInfo FnCall;
		EGParse_ParseFunction( *CurrentCommand , &FnCall );

		eg_string_crc FnCallAsCrc = FnCall.FunctionName ? eg_string_crc(FnCall.FunctionName) : CT_Clear;

		switch_crc(FnCallAsCrc)
		{
			case_crc("NewWorld"):
			{
				eg_d_string FullExeName = EGEdApp_GetEditorExecutable();
				eg_string_big FullPath = EGString_Format( "%s -worlded"  , *FullExeName );
				EGExtProcess_Run( FullPath , nullptr );
			} break;
			case_crc("NewDataAsset"):
			{
				eg_d_string FullExeName = EGEdApp_GetEditorExecutable();
				eg_string_big FullPath = EGString_Format( "%s -asseted"  , *FullExeName );
				EGExtProcess_Run( FullPath , nullptr );
			} break;
			case_crc("NewEntityDef"):
			{
				eg_d_string FullExeName = EGEdApp_GetEditorExecutable();
				eg_string_big FullPath = EGString_Format( "%s -defed"  , *FullExeName );
				EGExtProcess_Run( FullPath , nullptr );
			} break;
			case_crc("NewUILayout"):
			{
				eg_d_string FullExeName = EGEdApp_GetEditorExecutable();
				eg_string_big FullPath = EGString_Format( "%s -lyted"  , *FullExeName );
				EGExtProcess_Run( FullPath , nullptr );
			} break;
			case_crc("NewEGSMScript"):
			{
				eg_d_string FullExeName = EGEdApp_GetEditorExecutable();
				eg_string_big FullPath = EGString_Format( "%s -egsmed"  , *FullExeName );
				EGExtProcess_Run( FullPath , nullptr );
			} break;
			case_crc("ResaveAssets"):
			{
				eg_d_string FullExeName = EGEdApp_GetEditorExecutable();
				eg_d_string DataDirs[2];
				{
					const eg_d_string SourceRoot = EGToolsHelper_GetEnvVar("EGSRC");
					const eg_d_string GameName = EGToolsHelper_GetEnvVar("EGGAME");
					DataDirs[0] = EGPath2_CleanPath( *(SourceRoot + "/games/" + GameName + "/data/") , '/' );
					DataDirs[1] = EGPath2_CleanPath( *(SourceRoot + "/core/egdata/") , '/' );
				}
				for( const eg_d_string& DataDir : DataDirs )
				{
					EGLogf( eg_log_t::General , "Resaving assets in %s..." , *DataDir );
					eg_string_big FullPath = EGString_Format( "%s -resaveall \"%s\" -nodatabuild"  , *FullExeName , *DataDir);
					EGExtProcess_Run( FullPath , nullptr );
					EGLogf( eg_log_t::General , "Done resaving assets in %s." , *DataDir );
				}
			} break;
			case_crc("Localize"):
				EGLocAssets_Execute();
				break;
			case_crc("EditorConfig"):
			{
				eg_string_big FullPath = EGString_Format( "EGEdConfig_x64.exe"  , EGToolsHelper_GetEnvVar("EGSRC").String() );
				EGExtProcess_Run( FullPath , nullptr );
			} break;
			case_crc("DataBuild"):
			{
				EGLogf( eg_log_t::General , "Building game data for %s..." , EGToolsHelper_GetEnvVar("EGGAME").String() );
				EGExtProcess_Run( "egmake2_x64 DATA" , nullptr );
				// EGExtProcess_Run( "egmake2_x64 MAKEWEBSERVICE" , nullptr );
				EGExtProcess_Run( "egmake2_x64 CREATE_GAME_INI" , nullptr );
				EGLogf( eg_log_t::General , "Data Build complete." );
			} break;
			case_crc("CreateVSProjects"):
			{
				EGLogf( eg_log_t::General , "Creating Visual Studio projects..." );
				EGExtProcess_Run( "egmake2_x64 VSGEN" , nullptr );
				EGLogf( eg_log_t::General , "Completed creating Visual Studio projects." );
			} break;
			case_crc("PackageGame"):
			{
				EGLogf( eg_log_t::General , "Packaging game data for %s..." , *eg_s_string_sml8(*EGToolsHelper_GetBuildVar("EGGAME")) );
				const eg_d_string8 FullCmd = EGSFormat8( "egmake2_x64 BUILD_DIST -game \"{0}\" -root \"{1}/\"" , *EGToolsHelper_GetBuildVar("EGGAME") , *EGToolsHelper_GetBuildVar("EGOUT") );
				EGExtProcess_Run( *FullCmd , nullptr );
				EGLogf( eg_log_t::General , "Packaging complete." );
			} break;
			case_crc("ImportData"):
			{
				m_LastReimportInfo.Type = eg_edapproc_reimport_t::Standard;
				m_LastReimportInfo.Parm = CT_Clear;
				if( FnCall.NumParms >= 1 )
				{
					m_LastReimportInfo.Parm = FnCall.Parms[0];
				}
				EGEdImportData_Execute( *m_LastReimportInfo.Parm );
			} break;
			case_crc("CleanDataBuild"):
			{
				EGEdUtility_CleanGameData( true );
			} break;
			case_crc("BuildWebServer"):
			{	
				EGEdAppProc_BuildWebServer();
			} break;
			case_crc("GenerateBuildConfig"):
			{
				if( FnCall.NumParms >= 1 )
				{
					EGLogf( eg_log_t::General , "Generating .BuildConfig for %s..." , *eg_d_string8(FnCall.Parms[0]) );
					EGExtProcess_Run( *EGSFormat8("egmake2_x64 GENERATE_BUILD_INFO -in \"{0}\"" , FnCall.Parms[0] ) , nullptr );
				}
			}break;
			case_crc("PlayGame"):
			{
				eg_d_string FullExeName = EGEdApp_GetEditorExecutable();
				eg_d_string16 OldCwd = EGOsFile_GetCWD();
				EGOsFile_SetCWD( "_BUILD\\Bin" );
				// eg_d_string FinalExeName = EGSFormat8("_BUILD/Bin/{0}", *FullExeName);
				EGLogf( eg_log_t::General , "Running %s..." , *eg_d_string8( FullExeName ) );
				EGExtProcess_Run( *FullExeName , nullptr );
				EGOsFile_SetCWD(*OldCwd);
			} break;
			case_crc("ReimportFile"):
			{
				if( FnCall.NumParms >= 1 )
				{
					m_LastReimportInfo.Type = eg_edapproc_reimport_t::SingleFile;
					m_LastReimportInfo.Parm = FnCall.Parms[0];
					EGLogf( eg_log_t::General , "Importing source for %s..." , *m_LastReimportInfo.Parm );
					EGEdImportData_ImportSourceFile( *m_LastReimportInfo.Parm );
					EGLogf( eg_log_t::General , "Import complete." );
				}

			} break;
			case_crc("RedoLastImport"):
			{
				switch( m_LastReimportInfo.Type )
				{
					case eg_edapproc_reimport_t::None:
						EGLogf( eg_log_t::General , "Nothing has been re-imported this session. Re-import a file before using this feature." );
						break;
					case eg_edapproc_reimport_t::Standard:
						EGEdImportData_Execute( *m_LastReimportInfo.Parm );
						break;
					case eg_edapproc_reimport_t::SingleFile:
						EGLogf( eg_log_t::General , "Importing source for %s..." , *m_LastReimportInfo.Parm );
						EGEdImportData_ImportSourceFile( *m_LastReimportInfo.Parm );
						break;
				}
				EGLogf( eg_log_t::General , "Import complete." );
			} break;
			default:
				assert( false ); // Unknown command.
				break;
		}

		SetIsExecuting( false );
	}
}

void EGEdAppProc::OnThreadMsg( eg_cpstr strParm )
{
	EGFunctionLock FnLock( &m_Mutex );
	m_MessageQueue.Append( strParm );
}

eg_bool EGEdAppProc::HasCommands() const
{
	EGFunctionLock FnLock( &m_Mutex );
	return m_MessageQueue.HasItems();
}

eg_bool EGEdAppProc::IsExecuting() const
{
	EGFunctionLock FnLock( &m_Mutex );
	return m_bIsExecuting || m_MessageQueue.HasItems();
}

void EGEdAppProc::CancelCommands()
{
	EGFunctionLock FnLock( &m_Mutex );
	m_bWantsCancel = true;
}

void EGEdAppProc::SetIsExecuting( eg_bool bNewValue )
{
	EGFunctionLock FnLock( &m_Mutex );
	m_bIsExecuting = bNewValue;
}

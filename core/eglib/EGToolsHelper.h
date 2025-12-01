#pragma once

class EGFileData;

eg_string_big EGToolsHelper_GetDefaultGameName();
eg_string_big EGToolsHelper_GetEnvVar( eg_cpstr Var , eg_bool bForceFromRegistry = false );
eg_s_string_big16 EGToolsHelper_GetBuildVar( eg_cpstr Var );
void EGToolsHelper_SetBuildVar( eg_cpstr Var , eg_cpstr16 NewValue );
void EGToolsHelper_SetEnvVar( eg_cpstr Var , eg_cpstr NewValue , eg_bool bForceToRegistry = false );
void EGToolsHelper_DeleteEnvVar( eg_cpstr Name );
void EGToolsHelper_SetGameEditorConfigSetting( eg_cpstr Var , eg_cpstr16 NewValue );
eg_d_string16 EGToolsHelper_GetGameEditorConfigSetting( eg_cpstr Var );
eg_bool EGToolsHelper_OpenFile( eg_cpstr16 Filename , EGFileData& Out );
eg_bool EGToolsHelper_SaveFile( eg_cpstr16 Filename , const EGFileData& In );
static inline eg_bool EGToolsHelper_OpenFile( eg_cpstr8 Filename , EGFileData& Out ) { return EGToolsHelper_OpenFile( EGString_ToWide(Filename) , Out ); }
static inline eg_bool EGToolsHelper_SaveFile( eg_cpstr8 Filename , const EGFileData& In ) { return EGToolsHelper_SaveFile( EGString_ToWide(Filename) , In ); }
void EGToolsHelper_SetPathEnv();
eg_string EGToolsHelper_GameAssetPathToSource( eg_cpstr GameAssetPath , eg_cpstr GameName );
eg_d_string16 EGToolsHelper_GetRawAssetPathFromGameAssetPath( eg_cpstr16 GameAssetPathIn );
eg_bool EGToolsHelper_SetDirToSolutionDir();
void EGToolsHelper_InitEnvironment();

class EGCmdLineParms
{
private:

	struct egParmInfo
	{
		eg_s_string_sml16 Type;
		eg_d_string16     Value;
	};

private:

	EGArray<egParmInfo> m_Parms;

public:

	EGCmdLineParms() = default;
	EGCmdLineParms( int argc , char* argv[] );
	EGCmdLineParms( const EGArray<eg_s_string_sml8>& ArgsArray );

	void AddParm( eg_cpstr16 Type , eg_cpstr16 Value );
	void AppendLastParm( eg_cpstr16 Value );
	eg_bool ContainsType( eg_cpstr16 Type ) const;
	eg_d_string16 GetParmValue( eg_cpstr16 Type ) const;
};

void EGToolsHelper_GetCmdLineParms( const EGArray<eg_s_string_sml8>& ArgsArray , EGCmdLineParms& ParmsOut );
void EGToolsHelper_GetCmdLineParms( int argc , char* argv[] , EGCmdLineParms& ParmsOut );


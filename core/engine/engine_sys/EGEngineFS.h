// (c) 2018 Beem Media

#pragma once

struct egEngineFSInitParms
{
	eg_d_string16 RootDir = CT_Clear;
	eg_d_string8 GameName = CT_Clear;
	eg_d_string16 UserDir = CT_Clear;
	eg_d_string16 SysCfgDir = CT_Clear;

	egEngineFSInitParms() = default;
	egEngineFSInitParms( eg_cpstr16 InRootDir , eg_cpstr8 InGameName , eg_cpstr16 InUserDir , eg_cpstr16 InSysCfgDir )
	: RootDir( InRootDir ) 
	, GameName( InGameName ) 
	, UserDir( InUserDir ) 
	, SysCfgDir( InSysCfgDir )
	{ 
	}
};

void EGEngineFS_Init( const egEngineFSInitParms& InitParms );
void EGEngineFS_Deinit();
eg_bool EGEngineFS_DoesFileExist( eg_cpstr Filename );
void EGEngineFS_ListRoot();

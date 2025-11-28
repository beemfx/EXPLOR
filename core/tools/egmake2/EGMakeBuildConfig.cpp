// (c) 2018 Beem Media

#include "EGMakeBuildConfig.h"
#include "EGMake.h"
#include "EGFileData.h"
#include "EGToolsHelper.h"

EGMakeBuildConfig EGMakeBuildConfig::s_Inst;

void EGMakeBuildConfig::LoadGameConfig( eg_cpstr GameName )
{
	eg_string_big GameNamePath = EGString_Format( "%s/games/%s/Game.BuildConfig" , EGMake_GetEGSRC() , GameName );
	EGBuildConfig::LoadConfig( EGString_ToWide(GameNamePath) );
}

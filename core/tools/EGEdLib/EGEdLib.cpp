// (c) 2018 Beem Media

#include "EGPath2.h"
#include "EGToolsHelper.h"
#include "EGEngineConfig.h"

#define EGRFL_SYSTEM_HEADER "edlib.link.reflection.hpp"
#include "EGRflLinkSystem.hpp"
#undef EGRFL_SYSTEM_HEADER

void EGEdLib_Init()
{

}

eg_d_string EGEdLib_GetFilenameAsGameDataFilename( eg_cpstr Filename )
{
	eg_string_big RootDir = "/";
	eg_string_small GameName = EGToolsHelper_GetEnvVar( "EGGAME" );
	eg_string_big SrcPath = EGToolsHelper_GetEnvVar( "EGSRC" );
	SrcPath.Append( '/' );

	eg_string_big PathRelativeToSrc = *EGPath2_GetRelativePathTo( Filename , SrcPath );

	egPathParts2 RelativePathParts = EGPath2_BreakPath( PathRelativeToSrc );

	if( RelativePathParts.Folders.IsValidIndex(0) && RelativePathParts.Folders[0].EqualsI(L"core") )
	{
		RootDir = "/egdata/";
		SrcPath.Append( "core/egdata/" );
	}
	else
	{
		RootDir = GAME_DATA_PATH "/";
		SrcPath.Append( "games/");
		SrcPath.Append( GameName );
		SrcPath.Append( "/data/" );
	}

	eg_string_big RelativePath = *EGPath2_GetRelativePathTo( Filename , SrcPath );
	eg_string_big GamePath = *EGPath2_GetFullPathRelativeTo( RelativePath , RootDir );
	return GamePath.String();
}

eg_bool EGEdLib_IsAutoDataBuildEnabled()
{
	return !EGToolsHelper_GetEnvVar( "EGNOAUTOBUILD" ).ToBool();
}

// (c) 2017 Beem Media

#include "EGMake.h"
#include "EGXMLBase.h"
#include "EGExtProcess.h"
#include "EGEdCubeMapBuild.h"

eg_bool EGMakeRes_ecube()
{
	eg_cpstr FilenameIn = EGMake_GetInputPath( FINFO_FULL_FILE );
	eg_string_big strOutDir(EGMake_GetOutputPath(FINFO_NOEXT_FILE));
	strOutDir.Append( ".DDS" );
	return EGEdCubeMapBuild_Build( FilenameIn , strOutDir );
}
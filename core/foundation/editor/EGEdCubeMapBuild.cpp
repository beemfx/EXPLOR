// (c) 2018 Beem Media

#include "EGEdCubeMapBuild.h"
#include "EGPath2.h"
#include "EGExtProcess.h"
#include "EGToolsHelper.h"
#include "EGFileData.h"

EGEdCubeMapDescFile::EGEdCubeMapDescFile( eg_cpstr Filename ) : m_IsLoaded( false )
{
	EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
	eg_bool bLoadedFile = EGToolsHelper_OpenFile( Filename , FileData );
	if( bLoadedFile )
	{
		XMLLoad( FileData.GetData() , FileData.GetSize() , Filename );
	}
}

void EGEdCubeMapDescFile::OnTag( const eg_string_base& Tag, const EGXmlAttrGetter& Getter )
{
	if( "ecube" == Tag )
	{
		OnTag_ecube( Getter );
	}
}

void EGEdCubeMapDescFile::OnTag_ecube( const EGXmlAttrGetter& AttGet )
{
	m_IsLoaded = true;
	m_XPos = AttGet.GetString( "xpos" );
	m_XNeg = AttGet.GetString( "xneg" );
	m_YPos = AttGet.GetString( "ypos" );
	m_YNeg = AttGet.GetString( "yneg" );
	m_ZPos = AttGet.GetString( "zpos" );
	m_ZNeg = AttGet.GetString( "zneg" );

	m_XPos = EGPath2_GetFullPathRelativeTo( *m_XPos, GetXmlFilename() );
	m_XNeg = EGPath2_GetFullPathRelativeTo( *m_XNeg, GetXmlFilename() );
	m_YPos = EGPath2_GetFullPathRelativeTo( *m_YPos, GetXmlFilename() );
	m_YNeg = EGPath2_GetFullPathRelativeTo( *m_YNeg, GetXmlFilename() );
	m_ZPos = EGPath2_GetFullPathRelativeTo( *m_ZPos, GetXmlFilename() );
	m_ZNeg = EGPath2_GetFullPathRelativeTo( *m_ZNeg, GetXmlFilename() );

	m_Opts = AttGet.GetString( "opts" );
}

eg_bool EGEdCubeMapBuild_Build( eg_cpstr SourceFile , eg_cpstr DestFile )
{
	eg_cpstr FilenameIn = SourceFile;

	EGEdCubeMapDescFile CubeDesc( FilenameIn );

	eg_bool Success = false;

	if( CubeDesc.IsLoaded() )
	{
		eg_string_big strOutDir( DestFile );
		
		eg_char Cmd[2048];

		EGString_FormatToBuffer( 
			Cmd,
			countof(Cmd),
			"texassemble.exe cube %s -nologo -y -f R8G8B8A8_UNORM -o \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"" , 
			CubeDesc.GetOpts(),
			strOutDir.String(),
			CubeDesc.GetXPos(),
			CubeDesc.GetXNeg(),
			CubeDesc.GetYPos(),
			CubeDesc.GetYNeg(),
			CubeDesc.GetZPos(),
			CubeDesc.GetZNeg() );

		Success = EGExtProcess_Run( Cmd , nullptr );
	}

	return Success;
}

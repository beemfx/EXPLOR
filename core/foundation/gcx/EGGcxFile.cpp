// (c) 2018 Beem Media

#include "EGGcxFile.h"
#include "EGPath2.h"
#include "EGGcxBuildData.h"
#include "EGOsFile.h"
#include "EGToolsHelper.h"
#include "EGBuildConfig.h"

EGGcxFile::EGGcxFile( eg_cpstr Filename , eg_cpstr SrcDir ) 
: m_Failed( false )
, m_bReadingRegion( false )
, m_SourceDir( SrcDir )
{
	Clear();

	XMLLoad( Filename );

	// Finalize region tile info.
	for( EGGcxRegion& Region : m_Regions )
	{
		Region.ProcessMarkers();
		Region.SetCustomTiles( m_CustomTiles );
	}
}

EGGcxFile::~EGGcxFile()
{
	Clear();
}

eg_bool EGGcxFile::SaveAsXmlEmap( EGGcxSaveCb& SaveFn , EGGcxWriteMtrlCb& WriteMtrlFn , EGGcxProcessPropsCb& ProcessPropsCb , const gcxSaveProps& SaveProps , eg_bool bCompress )
{
	if( !IsLoaded() )return false;

	eg_bool bSucc = true;

	EGBuildConfig BuildConfig( *SaveProps.BuildConfigFile );

	for( EGGcxRegion& Region : m_Regions )
	{
		Region.ProcessProperties();

		eg_d_string8 BuildDataFilename;

		auto ProcessBuildConfig = [&BuildConfig,&BuildDataFilename,&Region]() -> void
		{
			eg_d_string8 MapProperties = BuildConfig.GetConfigValue( *Region.GetName() );
			EGArray<eg_d_string8> PropsArray = EGString_Split<eg_d_string8>( *MapProperties , ':' , 1 );
			bool bNextIsBuildDataFile = false;
			for( const eg_d_string8& PropItem : PropsArray )
			{
				if( bNextIsBuildDataFile )
				{
					bNextIsBuildDataFile = false;
					BuildDataFilename = PropItem;

				}
				else if( PropItem.EqualsI( "BuildData" ) )
				{
					bNextIsBuildDataFile = true;
				}
			}

			if( BuildDataFilename.Len() == 0 )
			{
				BuildDataFilename = BuildConfig.GetConfigValue( "DefaultBuildData" );
			}
		};

		ProcessBuildConfig();

		eg_string_big BuildDataFile;
		if( BuildDataFilename.Len() > 0 )
		{
			eg_d_string EGSRC = EGToolsHelper_GetEnvVar( "EGSRC" );
			eg_d_string EGGAME = EGToolsHelper_GetEnvVar( "EGGAME" );
			eg_string_big TileInfoPath = EGString_Format( "%s/games/%s/data/%s" , *EGSRC , *EGGAME , *BuildDataFilename );
			TileInfoPath = *EGPath2_CleanPath( TileInfoPath , '\\' );

			BuildDataFile = TileInfoPath;// EGPath_GetFullPathRelativeTo( Region.GetTileInfoFile() , *m_SourceDir );
		}

		if( EGOsFile_DoesFileExist( BuildDataFile ) )
		{
			
		}
		else
		{
			BuildDataFile = *SaveProps.DefaultTileLib;
		}

		EGGcxBuildData::Load( EGString_ToWide(BuildDataFile) );

		EGLogf( eg_log_t::General , "Exporting %s (Build Data: %s)..." , *Region.GetName() , *EGPath2_GetFilename( BuildDataFile.String() , false ) );

		Region.ProcessNotes();

		gcxExtraMapProps ExtraProps;
		bSucc = bSucc && Region.SaveToDir( *SaveProps.OutputDir , SaveFn , WriteMtrlFn , ExtraProps , bCompress );

		eg_d_string PropsFilename = EGPath2_CleanPath( EGString_Format( "%s/%s_meta.egasset" , *SaveProps.OutputDir , *Region.GetName() ) , '\\' );
		ProcessPropsCb( *PropsFilename , ExtraProps );
		
		EGGcxBuildData::Unload();
	}

	return bSucc;
}

void EGGcxFile::Clear()
{
	m_Regions.Clear();
	m_CustomTiles.Clear();
	m_Failed = false;
	m_CurRegionIndex = -1;
	m_bReadingRegion = false;
}

void EGGcxFile::OnTag( const eg_string_base& Tag, const EGXmlAttrGetter& Atts )
{
	if( "map" == Tag )
	{
		Clear();
	}
	else if( "export" == Tag )
	{

	}
	else if( "tile" == Tag )
	{
		if( GetXmlTagUp( 1 ) == "color" && GetXmlTagUp( 2 ) == "custom" )
		{
			gcxCustomTile NewCustomeTile;
			NewCustomeTile.Index = Atts.GetUInt( "index" );
			NewCustomeTile.File[0] = '\0';
			m_CustomTiles.Append( NewCustomeTile );
		}
	}
	else if( "region" == Tag )
	{
		m_bReadingRegion = true;
		m_CurRegionIndex++;
		if( m_CurRegionIndex >= 0 )
		{
			m_Regions.Append( EGGcxRegion() );
			m_Regions[m_CurRegionIndex].SetSourceDir( *m_SourceDir );
			m_Regions[m_CurRegionIndex].HandleXmlTag( Tag, Atts );
		}
	}
	else
	{
		if( m_bReadingRegion && m_CurRegionIndex >= 0 )
		{
			m_Regions[m_CurRegionIndex].HandleXmlTag( Tag, Atts );
		}
	}
}

void EGGcxFile::OnTagEnd( const eg_string_base& Tag )
{
	if( "region" == Tag )
	{
		m_bReadingRegion = false;
	}
}

void EGGcxFile::OnData( eg_cpstr Data, eg_uint Len )
{
	if( m_bReadingRegion && m_CurRegionIndex >= 0 )
	{
		m_Regions[m_CurRegionIndex].HandleXmlData( GetXmlTagUp( 0 ), Data, Len );
	}
	else
	{
		if( m_CustomTiles.Len() > 0 && GetXmlTagUp( 0 ) == "name" && GetXmlTagUp( 1 ) == "tile" && GetXmlTagUp( 2 ) == "color" && GetXmlTagUp( 3 ) == "custom" )
		{
			gcxCustomTile& CustomTile = m_CustomTiles[m_CustomTiles.Len() - 1];
			EGString_Copy( CustomTile.File, EGGcxRegion::GetCleanName( Data, Len ), countof( CustomTile.File ) );
			CustomTile.TerrainType = CustomTileFileToType( CustomTile.File );
		}
	}
}

gcx_terrain_t EGGcxFile::CustomTileFileToType( eg_cpstr File )
{
	gcx_terrain_t Out = gcx_terrain_t::DEFAULT;

	if( false )
	{
	}
	#define DECL_GCT( _name_ , _value_ )
	#define DECL_CUSTOM( _name_ , _file_ ) else if( EGString_EqualsI( File , _file_ ) ){ Out = gcx_terrain_t::CUSTOM_##_name_; }
	#include "EGGcxCustomTiles.items"
	else
	{
		EGLogf( eg_log_t::Warning, __FUNCTION__ ": No tile info for %s.", File );
	}

	return Out;
}

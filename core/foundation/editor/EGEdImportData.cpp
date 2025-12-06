// (c) 2018 Beem Media

#include "EGEdImportData.h"
#include "EGWindowsAPI.h"
#include "EGOsFile.h"
#include "EGToolsHelper.h"
#include "EGPath2.h"
#include "EGMs3DCompiler.h"
#include "EGFileData.h"
#include "EGLibFile.h"
#include "EGEdUtility.h"
#include "EGExtProcess.h"
#include "EGBuildConfig.h"
#include "EGEdCubeMapBuild.h"
#include "EGRendererTypes.h"
#include "EGTerrain2.h"

static void EGEdImportData_ImportFile_ms3d( eg_cpstr Filename , eg_cpstr DestRoot , const egPathParts2& PathParts );
static void EGEdImportData_ImportFile_Image( eg_cpstr Filename , eg_cpstr DestRoot , const egPathParts2& PathParts );
static void EGEdImportData_ImportFile_CubeMap( eg_cpstr Filename , eg_cpstr DestRoot , const egPathParts2& PathParts );
static void EGEdImportData_ImportFile_TerrainBuild( eg_cpstr Filename , eg_cpstr DestRoot , const egPathParts2& PathParts );
static void EGEdImportData_ImportFile_xml( eg_cpstr Filename , eg_cpstr DestRoot , const egPathParts2& PathParts );

static eg_d_string EGEdImportData_GetOutputDir( eg_cpstr DestRoot , const egPathParts2& PathParts )
{
	egPathParts2 NewPath = PathParts;
	NewPath.Folders.DeleteByIndex(0);
	NewPath.Folders.DeleteByIndex(0);
	NewPath.Folders.DeleteByIndex(0);
	eg_d_string OutputDir = EGPath2_CleanPath( EGString_Format( "%s/%s" , DestRoot , *NewPath.GetDirectory8( '/' ) ) , '/' );
	return OutputDir;
}

static void EGEdImportData_ImportFile( eg_cpstr Filename , eg_cpstr DestRoot )
{
	EGLogf( eg_log_t::Verbose , "Importing \"%s\"..." , Filename );

	egPathParts2 PathParts = EGPath2_BreakPath( Filename );
	eg_string_small LowerExt = *PathParts.Ext;
	LowerExt.ConvertToLower();
	eg_string_crc ExtAsCrc = eg_string_crc(LowerExt);

	const eg_d_string ConfigPath = EGPath2_CleanPath( EGString_Format("%s%s.BuildConfig", *PathParts.GetDirectory8() , *PathParts.GetFilename8( false ) ) , '/' );
	EGBuildConfig BuildConfig( *ConfigPath );
	if( EGString_ToBool( *BuildConfig.GetConfigValue("Ignore") ) )
	{
		// Probably part of a Cube Map or something
		EGLogf( eg_log_t::General , "Ignoring %s." , Filename );
		return;
	}

	switch_crc(ExtAsCrc)
	{
		case_crc("ms3d"):
		{
			EGEdImportData_ImportFile_ms3d( Filename , DestRoot , PathParts );
		} break;

		case_crc("png"):
		case_crc("tga"):
		case_crc("bmp"):
		{
			EGEdImportData_ImportFile_Image( Filename , DestRoot , PathParts );
		} break;

		case_crc("cubemap"):
		{
			EGEdImportData_ImportFile_CubeMap( Filename , DestRoot , PathParts );
		} break;

		case_crc("buildterrain"):
		{
			EGEdImportData_ImportFile_TerrainBuild( Filename , DestRoot , PathParts );
		} break;

		case_crc("xml"):
		{
			EGEdImportData_ImportFile_xml( Filename , DestRoot , PathParts );
		} break;

		default:
		{
			// EGLogf( eg_log_t::Warning , "No tool for \"%s\" files." , PathParts.Ext.String() );
		} break;
	}
}

void EGEdImportData_Execute( eg_cpstr FileExt )
{
	const eg_d_string SourceRoot = EGToolsHelper_GetEnvVar("EGSRC");
	const eg_d_string GameName = EGToolsHelper_GetEnvVar("EGGAME");

	EGToolsHelper_SetPathEnv();

	auto ImportProject = [&SourceRoot,&FileExt]( eg_cpstr SourceFolder , eg_cpstr TargetFolder , eg_bool bIsEngine ) -> void
	{
		eg_d_string RawRoot;
		eg_d_string DestRoot;
		RawRoot = SourceRoot;
		if( bIsEngine )
		{
			RawRoot.Append( "/core/egdata_src/" );
			RawRoot = EGPath2_CleanPath( *RawRoot , '/' );
			DestRoot = SourceRoot;
			DestRoot.Append( TargetFolder );
			DestRoot.Append( "/egdata/" );
			DestRoot = EGPath2_CleanPath( *DestRoot , '/' );
		}
		else
		{
			RawRoot.Append( "/games/" );
			RawRoot.Append( SourceFolder );
			RawRoot.Append( "/data_src/" );
			RawRoot.Append( "/" );
			RawRoot = EGPath2_CleanPath( *RawRoot , '/' );
			DestRoot = SourceRoot;
			DestRoot.Append( TargetFolder );
			DestRoot.Append( "/data/" );
			DestRoot = EGPath2_CleanPath( *DestRoot , '/' );
		}

		EGLogf( eg_log_t::General , "Importing from \"%s\" -> \"%s\"" , *RawRoot , *DestRoot );

		EGOsFileWantsFileFn WantsFileFn = [&FileExt]( eg_cpstr Filename , const egOsFileAttributes& FileAttr ) -> eg_bool
		{
			unused( FileAttr );
			egPathParts2 PathParts = EGPath2_BreakPath( Filename );
			if( FileAttr.bHidden || PathParts.Ext.EqualsI(L"BuildConfig") )
			{
				return false;
			}
			if( EGString_StrLen(FileExt) > 0 )
			{
				if( EGString_EqualsI( FileExt , "Texture" ) )
				{
					static const eg_cpstr16 ValidTextureExts[] =
					{
						L"TGA",
						L"PNG",
					};

					for( eg_cpstr16 TextureExt : ValidTextureExts )
					{
						if( PathParts.Ext.EqualsI( TextureExt ) )
						{
							return true;
						}
					}

					return false;
				}
				else if( !PathParts.Ext.EqualsI( *eg_d_string16(FileExt)) )
				{
					return false;
				}
			}
			return true;
		};

		EGArray<eg_d_string> RawAssetFiles;
		EGOsFile_FindAllFiles( *RawRoot , WantsFileFn , RawAssetFiles );
		for( const eg_d_string& Filename : RawAssetFiles )
		{
			eg_d_string FinalFilename = RawRoot + Filename;
			EGEdImportData_ImportFile( *FinalFilename , *DestRoot );
		}

		EGLogf( eg_log_t::General , "Importing complete." );
	};

	ImportProject( *GameName , EGString_Format( "/games/%s/" , *GameName ).String() , false );
	ImportProject( "egdata" , "/core/" , true );
}

void EGEdImportData_ImportSourceFile( eg_cpstr GameFilePath )
{
	EGToolsHelper_SetPathEnv();
	
	const egPathParts2 GameFilePathParts = EGPath2_BreakPath( GameFilePath );
	const eg_d_string16 RawAssetPath = EGToolsHelper_GetRawAssetPathFromGameAssetPath( *eg_d_string16(GameFilePath) );
	const egPathParts2 RawAssetParts = EGPath2_BreakPath( *RawAssetPath );

	// This is a hack to get things working...
	eg_d_string8 ImportRoot;
	const eg_string_big DefGameName = EGToolsHelper_GetDefaultGameName();
	if (GameFilePathParts.Folders.IsValidIndex(2) && GameFilePathParts.Folders[2].EqualsI(*eg_d_string16(DefGameName)))
	{
		ImportRoot = EGSFormat8( "./Games/{0}/data" , *DefGameName );
	}
	else
	{
		ImportRoot = "./Core/egdata";
	}

	egOsFileInfo TargetDirTree;
	EGOsFile_GetDirectoryContents( *RawAssetParts.GetDirectory() , TargetDirTree );

	if( TargetDirTree.Children.HasItems() )
	{
		eg_d_string16 SourceFile;
		eg_int PossibleSourceFileCount = 0;

		for( const egOsFileInfo& FileInfo : TargetDirTree.Children )
		{
			if( !FileInfo.Attributes.bDirectory )
			{
				const egPathParts2 FoundFileParts = EGPath2_BreakPath( *FileInfo.Name );
				if( FoundFileParts.Filename.EqualsI( *RawAssetParts.Filename ) && !FoundFileParts.Ext.EqualsI( L"BuildConfig" ) )
				{
					SourceFile = RawAssetParts.GetDirectory() + FoundFileParts.GetFilename( true );
					PossibleSourceFileCount++;
				}
			}
		}

		eg_bool bImportDirectory = false;
		if( PossibleSourceFileCount > 1 )
		{
			EGLogf( eg_log_t::General , "There was more than one possible source file. Reimporting the entire directory." );
			bImportDirectory = true;
		}
		else if( PossibleSourceFileCount == 1 && SourceFile.Len() > 0 )
		{
			egPathParts2 SourceFileParts = EGPath2_BreakPath( *SourceFile );
			EGLogf( eg_log_t::General , "Reimporting source file %s to %s..." , *SourceFileParts.GetFilename8() , *ImportRoot );
			EGEdImportData_ImportFile( *eg_d_string8(*SourceFile) , *ImportRoot );
		}
		else
		{
			EGLogf( eg_log_t::General , "No matching source file found. Reimporting the entire directory." );
			bImportDirectory = true;
		}

		if( bImportDirectory )
		{
			for( const egOsFileInfo& FileInfo : TargetDirTree.Children )
			{
				if( !FileInfo.Attributes.bDirectory )
				{
					const egPathParts2 FoundFileParts = EGPath2_BreakPath( *FileInfo.Name );
					if( !FoundFileParts.Ext.EqualsI( L"BuildConfig" ) )
					{
						const eg_d_string16 DirFile = RawAssetParts.GetDirectory() + FoundFileParts.GetFilename( true );

						EGLogf( eg_log_t::General , "Reimporting source file %s to %s..." , *eg_d_string8(*FileInfo.Name) , *ImportRoot );
						EGEdImportData_ImportFile( *eg_d_string8(*DirFile) , *ImportRoot );
					}
				}
			}
		}
	}
	else
	{
		EGLogf( eg_log_t::General , "Nothing to import for %s, no source directory found." , GameFilePath );
	}
}

static void EGEdImportData_ImportFile_ms3d( eg_cpstr Filename , eg_cpstr DestRoot , const egPathParts2& PathParts )
{
	eg_d_string ConfigPath = EGPath2_CleanPath( EGString_Format("%s%s.BuildConfig", *PathParts.GetDirectory8() , *PathParts.GetFilename8( false ) ) , '/' );
	EGBuildConfig BuildConfig( *ConfigPath );
	if( EGString_ToBool( *BuildConfig.GetConfigValue("Ignore") ) )
	{
		// Probably part of a Cube Map or something
		EGLogf( eg_log_t::General , "Ignoring %s." , Filename );
		return;
	}
	
	EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
	eg_bool bOpenedFile = EGLibFile_OpenFile( Filename , eg_lib_file_t::OS , FileData );
	if( bOpenedFile )
	{
		eg_d_string OutputDir = EGEdImportData_GetOutputDir( DestRoot , PathParts );
		eg_d_string MeshOutputPath = EGPath2_CleanPath( EGString_Format( "%s/%s.emesh" , *OutputDir , *PathParts.GetFilename8(false)) , '/' );
		eg_d_string SkelOutputPath = EGPath2_CleanPath( EGString_Format( "%s/%s.eskel" , *OutputDir , *PathParts.GetFilename8(false)) , '/' );

		EGMs3DCompiler Ms3DCompiler( FileData );

		if( Ms3DCompiler.HasMesh() )
		{
			EGLogf( eg_log_t::General , "Mesh -> \"%s\"" , *MeshOutputPath );
			EGFileData OutputFileData( eg_file_data_init_t::HasOwnMemory );
			Ms3DCompiler.SaveEMesh( OutputFileData );
			eg_bool bSaved = EGEdUtility_SaveFile( *MeshOutputPath , OutputFileData );
			if( bSaved )
			{
				EGEdUtility_RevertFile( *MeshOutputPath , true );
			}

			if( !bSaved )
			{
				EGLogf( eg_log_t::Error , "Was not able to save \"%s\"." , *MeshOutputPath );
			}
		}

		if( Ms3DCompiler.HasSkel() )
		{
			EGLogf( eg_log_t::General , "Skel -> \"%s\"" , *MeshOutputPath );
			EGFileData OutputFileData( eg_file_data_init_t::HasOwnMemory );
			Ms3DCompiler.SaveESkel( OutputFileData );
			eg_bool bSaved = EGEdUtility_SaveFile( *SkelOutputPath , OutputFileData );
			if( bSaved )
			{
				EGEdUtility_RevertFile( *SkelOutputPath , true );
			}

			if( !bSaved )
			{
				EGLogf( eg_log_t::Error , "Was not able to save \"%s\"." , *SkelOutputPath );
			}
		}
	}
	else
	{
		EGLogf( eg_log_t::Error , "Could not open \"%s\"." , Filename );
	}
}

static void EGEdImportData_ImportFile_Image( eg_cpstr Filename , eg_cpstr DestRoot , const egPathParts2& PathParts )
{
	eg_d_string InputFile = *PathParts.ToString( true , '\\' );
	eg_d_string OutputPath = EGEdImportData_GetOutputDir( DestRoot , PathParts );
	eg_d_string OutputFile = OutputPath;
	OutputFile += *PathParts.Filename;
	OutputFile += ".dds";
	eg_d_string ConfigPath = EGPath2_CleanPath( EGString_Format("%s%s.BuildConfig", *PathParts.GetDirectory8() , *PathParts.GetFilename8( false ) ) , '/' );
	EGBuildConfig BuildConfig( *ConfigPath );
	if( ! BuildConfig.HasConfig() || EGString_ToBool( *BuildConfig.GetConfigValue("Ignore") ) )
	{
		// Probably part of a Cube Map or something
		EGLogf( eg_log_t::General , "Ignoring %s, no build config." , Filename );
		return;
	}

	eg_bool bHasDestDir = EGOsFile_CreateDirectory( *OutputPath );
	if( bHasDestDir )
	{
		EGEdUtility_CheckoutFile( EGString_ToWide(*OutputFile) );
		OutputPath.ClampEnd( 1 );

		eg_d_string Opts = "-nologo -srgb -fl 9.3 -ft DDS -y ";

		// Format
		{
			eg_d_string FormatStr = BuildConfig.GetConfigValue( "Format" );
			if( FormatStr.Len() > 0 )
			{
				Opts += EGSFormat8( "-f {0} " , *FormatStr );
			}
			else
			{
				Opts += "-f R8G8B8A8_UNORM ";
			}
		}

		// Mip Maps
		{
			eg_bool bNoMips = EGString_ToBool( *BuildConfig.GetConfigValue("NoMipMaps") );
			if( bNoMips )
			{
				Opts += "-m 1 ";
			}
		}

		// Filter
		{
			eg_d_string FilterStr = BuildConfig.GetConfigValue( "Filter" );
			if( FilterStr.Len() > 0 )
			{
				Opts += EGSFormat8( "-if {0} " , *FilterStr );
			}
		}

		// Image resize
		{
			{
				eg_d_string WidthStr = BuildConfig.GetConfigValue("Width");
				if( WidthStr.Len() > 0 && !WidthStr.EqualsI( "Default" ) )
				{
					const eg_int Width = EGString_ToInt( *WidthStr );
					if( Width > 0 )
					{
						Opts += EGSFormat8( "-w {0} " , Width );
					}
				}
			}

			{
				eg_d_string HeightStr = BuildConfig.GetConfigValue("Height");
				if( HeightStr.Len() > 0 && !HeightStr.EqualsI( "Default" ) )
				{
					const eg_int Height = EGString_ToInt( *HeightStr );
					if( Height > 0 )
					{
						Opts += EGSFormat8( "-h {0} " , Height );
					}
				}
			}
		}

		eg_d_string strCmd = EGString_Format( "texconv.exe %s -o \"%s\" \"%s\"" , *Opts , *OutputPath , *InputFile ).String();
		EGExtProcess_Run( *strCmd , nullptr );
		EGEdUtility_AddFile( EGString_ToWide(*OutputFile) );
		EGEdUtility_RevertFile( EGString_ToWide(*OutputFile) , true );
	}
}

static void EGEdImportData_ImportFile_CubeMap( eg_cpstr Filename , eg_cpstr DestRoot , const egPathParts2& PathParts )
{
	eg_d_string OutputFolder = EGEdImportData_GetOutputDir( DestRoot , PathParts );
	EGOsFile_CreateDirectory( *OutputFolder );
	eg_d_string DestPath = EGPath2_CleanPath( EGString_Format( "%s/%s.dds" , *OutputFolder , *PathParts.GetFilename8(false) ) , '/' );
	EGEdUtility_CheckoutFile( EGString_ToWide(*DestPath) );
	EGEdCubeMapBuild_Build( Filename , *DestPath );
	EGEdUtility_AddFile( EGString_ToWide(*DestPath) );
	EGEdUtility_RevertFile( EGString_ToWide(*DestPath) , true );
}

static void EGEdImportData_ImportFile_TerrainBuild( eg_cpstr Filename , eg_cpstr DestRoot , const egPathParts2& PathParts )
{
	eg_d_string OutputFolder = EGEdImportData_GetOutputDir( DestRoot , PathParts );
	EGOsFile_CreateDirectory( *OutputFolder );

	EGBuildConfig BuildConfig( Filename );
	eg_d_string HfFile = BuildConfig.GetConfigValue( "HfFile" );
	HfFile = EGPath2_GetFullPathRelativeTo( *HfFile , Filename );
	eg_d_string HfType = BuildConfig.GetConfigValue( "HfType" );
	eg_d_string HfDimsStr = BuildConfig.GetConfigValue( "HfDims" );
	eg_ivec2 HfDims = CT_Clear;
	EGString_GetIntList( *HfDimsStr , HfDimsStr.Len() , &HfDims , 2 );
	eg_d_string WorldDimsStr = BuildConfig.GetConfigValue( "WorldDims" );
	eg_d_string WorldScaleStr = BuildConfig.GetConfigValue( "WorldScale" );
	if( WorldScaleStr.Len() == 0 )
	{
		WorldScaleStr = "1.0";
	}
	eg_vec2 WorldDims = CT_Clear;
	EGString_GetFloatList( *WorldDimsStr , WorldDimsStr.Len() , &WorldDims , 2 );
	EGLogf( eg_log_t::General , "Importing Height Field: %s" , *HfFile );
	EGFileData HfData( eg_file_data_init_t::HasOwnMemory );
	EGToolsHelper_OpenFile( *HfFile , HfData );

	EGMaterialDef Material = CT_Default;
	EGString_Copy( Material.m_strTex[0] , *BuildConfig.GetConfigValue( "Tex0" ) , countof(Material.m_strTex[0]) );
	EGString_Copy( Material.m_strTex[1] , *BuildConfig.GetConfigValue( "Tex1" ) , countof(Material.m_strTex[1]) );
	EGString_Copy( Material.m_strTex[2] , *BuildConfig.GetConfigValue( "Tex2" ) , countof(Material.m_strTex[2]) );
	EGString_Copy( Material.m_strTex[3] , *BuildConfig.GetConfigValue( "Tex3" ) , countof(Material.m_strTex[3]) );

	EGTerrain2::egHeader TerHeader;
	TerHeader.Id = EGTerrain2::ID;
	TerHeader.Version = EGTerrain2::VERSION;
	TerHeader.HfDims = HfDims;
	TerHeader.WorldDims = WorldDims;
	TerHeader.WorldScale = EGString_ToReal( *WorldScaleStr );

	if( HfType.EqualsI("R32") )
	{
		if( HfDims.x * HfDims.y * sizeof(eg_real) == HfData.GetSize() )
		{
			const eg_real* Hf = HfData.GetDataAs<eg_real>();

			EGFileData OutFile( eg_file_data_init_t::HasOwnMemory );
			OutFile.Write( &TerHeader , sizeof(TerHeader) );
			OutFile.Write( &Material , sizeof(Material) );
			OutFile.Write( Hf , HfDims.x * HfDims.y * sizeof(eg_real) );

			eg_d_string DestPath = EGPath2_CleanPath( EGString_Format( "%s/%s.egterrain" , DestRoot , *PathParts.ToString8( false , '/' ) ) , '/' );

			EGLogf( eg_log_t::General , "Exporting terrain to %s." , *DestPath );
			EGEdUtility_SaveFile( *DestPath , OutFile );
			EGEdUtility_RevertFile( EGString_ToWide(*DestPath) , true );
		}
		else
		{
			EGLogf( eg_log_t::Error , "%s data did not match the terrain build." , *HfFile );
		}
	}
}

static void EGEdImportData_ImportFile_xml( eg_cpstr Filename , eg_cpstr DestRoot , const egPathParts2& PathParts )
{
	eg_d_string ConfigPath = EGPath2_CleanPath( EGString_Format( "%s%s.BuildConfig", *PathParts.GetDirectory8() , *PathParts.GetFilename8( false ) ), '/' );
	EGBuildConfig BuildConfig( *ConfigPath );
	if( !BuildConfig.HasConfig() || EGString_ToBool( *BuildConfig.GetConfigValue( "Ignore" ) ) )
	{
		// Probably part of a Cube Map or something
		EGLogf( eg_log_t::General, "Ignoring %s, no build config.", Filename );
		return;
	}

	eg_d_string XmlType = BuildConfig.GetConfigValue( "XmlType" );

	eg_d_string OutputDir = EGEdImportData_GetOutputDir( DestRoot , PathParts );

	EGLogf( eg_log_t::General , "Importing XML %s (%s,%s)..." , Filename , *ConfigPath , *XmlType );

	if( XmlType.EqualsI( "GCX" ) )
	{
		EGLogf( eg_log_t::General , "Compiling Grid Cartographer XML to \"%s\"..." , *OutputDir );
		eg_bool bHasDestDir = EGOsFile_CreateDirectory( *OutputDir );
		if( bHasDestDir )
		{
			eg_d_string GcxBuildCmd = EGString_Format( "egmake2_x64.exe MAP -gcx -in \"%s\" -emap \"%s_unused_.emap\" -compress" , Filename , *OutputDir );
			EGExtProcess_Run( *GcxBuildCmd , nullptr );
		}
	}
}
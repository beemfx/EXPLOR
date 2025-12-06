// (c) 2019 Beem Media. All Rights Reserved.

#include "EGMake_vsgen.h"
#include "EGOsFile.h"
#include "EGFileData.h"
#include "EGEdUtility.h"
#include "EGPath2.h"
#include "EGToolsHelper.h"

#include "EGMake_vsgen_FileTemplates.hpp"

static const eg_cpstr16 EGVSGen_Exts_Ignore[] = { L".user" , L".vcxproj" , L".filters" , L".sdf" , L".suo" , L".opensdf" , L".aps" , L".pyc" , L".vcxprojAssemblyReference.cache" };
static const eg_cpstr16 EGVSGen_Exts_Compile[] = { L".cpp" , L".c" };
static const eg_cpstr16 EGVSGen_Exts_Include[] = { L".h" , L".hpp" , L".inc" , L".grp" , L".inl" , L".items" };
static const eg_cpstr16 EGVSGen_Exts_Resource[] = { L".rc" };

// Now that there is a public demo and release data we don't want project data
// referencing non-existent stuff, so we are going to build the projects without
// data.
static const eg_bool EGVSGen_bIncludeData = false;

enum class eg_vsgen_ext_t
{
	Default,
	Ignore,
	Compile,
	Include,
	Resource,
};

struct egVSGenBuildInfo
{
	eg_d_string8 ProjectName;
	eg_d_string8 TemplateName;
	eg_d_string16 ProjectDir;
	eg_d_string8 Defines;
	EGArray<eg_d_string16> IncludeRoots;
	EGArray<eg_d_string16> IncludeDirs;
	EGArray<eg_d_string16> ExcludeFiles;
	EGArray<eg_d_string16> ProjectRoots;
	eg_bool bGenerateFilters = true;
	eg_bool bForceEmptyProject = false;
};

static eg_vsgen_ext_t EGVSGen_GetExtType( eg_cpstr16 Filename )
{
	
#define EGVSGEN_GETEXT_TYPE_CASE( _type_ ) for( eg_int i=0; i<countof(EGVSGen_Exts_##_type_); i++ ) { if( EGString_EndsWithI( Filename , EGVSGen_Exts_##_type_[i] ) ) { return eg_vsgen_ext_t::_type_; } }

	EGVSGEN_GETEXT_TYPE_CASE( Ignore )
	EGVSGEN_GETEXT_TYPE_CASE( Compile )
	EGVSGEN_GETEXT_TYPE_CASE( Include )
	EGVSGEN_GETEXT_TYPE_CASE( Resource )

#undef EGVSGEN_GETEXT_TYPE_CASE

	return eg_vsgen_ext_t::Default;
}

static eg_bool EGVSGen_WantsFile( eg_cpstr16 Filename , const egOsFileAttributes& FileAttr )
{
	if( FileAttr.bHidden )
	{
		return false;
	}
	if( FileAttr.bTemp )
	{
		return false;
	}
	if( FileAttr.bDirectory )
	{
		return true;
	}

	// Ignore any extensions we don't want:
	if( EGVSGen_GetExtType( Filename ) == eg_vsgen_ext_t::Ignore )
	{
		return false;
	}

	return true;
}

static eg_bool EGVSGen_WantsIncludeRoot( eg_cpstr16 Filename , const egOsFileAttributes& FileAttr )
{
	unused( Filename );

	if( FileAttr.bHidden )
	{
		return false;
	}
	if( FileAttr.bTemp )
	{
		return false;
	}
	if( FileAttr.bDirectory )
	{
		return true;
	}
	return false;
}

class EGVSGenProject
{
private:

	egVSGenBuildInfo       m_BuildInfo;
	EGArray<eg_d_string16> m_DefaultFiles;
	EGArray<eg_d_string16> m_CompileFiles;
	EGArray<eg_d_string16> m_HeaderFiles;
	EGArray<eg_d_string16> m_ResourceFiles;
	EGArray<eg_d_string16> m_AllFilters;
	eg_d_string8           m_IncludePaths;

public:

	EGVSGenProject( const egVSGenBuildInfo& BuildInfo )
	{
		m_BuildInfo = BuildInfo;
		m_BuildInfo.ProjectDir += "/";
		m_BuildInfo.ProjectDir = EGPath2_CleanPath( *m_BuildInfo.ProjectDir , '\\' );
		for( eg_d_string16& ExcludeFile : m_BuildInfo.ExcludeFiles )
		{
			for( eg_size_t i=0; i<ExcludeFile.Len(); i++ )
			{
				if( ExcludeFile[i] == '\\' )
				{
					ExcludeFile[i] = '/';
				}
			}
		}

		EGLogf( eg_log_t::General , "Building project %s..." , *BuildInfo.ProjectName );

		FindFiles();
		FindIncludePaths();

		SaveProjectFile();
		if( m_BuildInfo.bGenerateFilters )
		{
			SaveFiltersFile();
		}
	}

	void SaveProjectFile()
	{
		EGLogf( eg_log_t::General , "Saving project..." );

		EGFileData FileData( eg_file_data_init_t::HasOwnMemory );

		eg_d_string8 ItemGroups;
		auto WriteArray = [&ItemGroups]( eg_cpstr8 CompileType , const EGArray<eg_d_string16>& Files ) -> void
		{
			static const eg_cpstr8 LineFormat = "    <{0} Include=\"{1}\" />\n";

			for( const eg_d_string16& File : Files )
			{
				const eg_d_string8 FinalLine = EGSFormat8( LineFormat , CompileType , *File );
				ItemGroups += FinalLine;
			}
		};

		// Files:
		ItemGroups.Append( VCXPROJ_IG_START );
		WriteArray( "ClCompile" , m_CompileFiles );
		ItemGroups.Append( VCXPROJ_IG_MID );
		WriteArray( "ClInclude" , m_HeaderFiles );
		ItemGroups.Append( VCXPROJ_IG_MID );
		WriteArray( "ResourceCompile" , m_ResourceFiles );
		ItemGroups.Append( VCXPROJ_IG_MID );
		WriteArray( "None" , m_DefaultFiles );
		ItemGroups.Append( VCXPROJ_IG_END );


		EGFileData TemplateFileData( eg_file_data_init_t::HasOwnMemory );
		const eg_d_string16 TemplateFilename = EGPath2_CleanPath( *EGSFormat16( L"{0}/core/BuildTools/vst/{1}.txt" , *EGToolsHelper_GetBuildVar( "EGSRC" ) , *m_BuildInfo.TemplateName ) , '\\' );
		EGToolsHelper_OpenFile( *TemplateFilename , TemplateFileData );
		TemplateFileData.Seek( eg_file_data_seek_t::End , 0 );
		TemplateFileData.Write<eg_char8>( '\0' );
		eg_d_string8 FinalFileStr( TemplateFileData.GetDataAs<eg_char8>() );

		EGStringEx_ReplaceAll( FinalFileStr , "%%%ITEM_GROUPS%%%" , *ItemGroups );
		// TODO:
		EGStringEx_ReplaceAll( FinalFileStr , "%%%INCLUDE_DIRS%%%" , *m_IncludePaths );
		EGStringEx_ReplaceAll( FinalFileStr , "%%%DEFINES%%%" , *m_BuildInfo.Defines );

		// Not final:
		FileData.WriteStr8( *FinalFileStr );

		const eg_d_string16 OutputFilename = GetProjectFilename( false );
		EGEdUtility_SaveIfUnchanged( *OutputFilename , FileData , true );
	}

	void SaveFiltersFile()
	{
		EGLogf( eg_log_t::General , "Saving filters..." );

		EGFileData FileData( eg_file_data_init_t::HasOwnMemory );

		auto WriteArray = [&FileData]( eg_cpstr8 CompileType , const EGArray<eg_d_string16>& Files ) -> void
		{
			static const eg_cpstr8 FilterLineFormat = "    <{0} Include=\"{1}\">\n      <Filter>{2}</Filter>\n    </{0}>\n";

			for( const eg_d_string16& File : Files )
			{
				eg_d_string16 FilterForFile = EGPath2_BreakPath( *File ).GetDirectory( '\\' );
				FilterForFile.ClampEnd( 1 );
				const eg_d_string8 FinalLine = EGSFormat8( FilterLineFormat , CompileType , *File , *FilterForFile );
				FileData.WriteStr8( *FinalLine );
			}
		};

		// Header:
		FileData.WriteStr8( VCXPROJ_FILTER_HEADER );

		// Filters:
		for( const eg_d_string16& Filter : m_AllFilters )
		{
			FileData.WriteStr8( *EGSFormat8( "    <Filter Include=\"{0}\" />\n" , *Filter ) );
		}
		FileData.WriteStr8( VCXPROJ_IG_MID );

		// Files:
		WriteArray( "ClCompile" , m_CompileFiles );
		FileData.WriteStr8( VCXPROJ_IG_MID );
		WriteArray( "ClInclude" , m_HeaderFiles );
		FileData.WriteStr8( VCXPROJ_IG_MID );
		WriteArray( "ResourceCompile" , m_ResourceFiles );
		FileData.WriteStr8( VCXPROJ_IG_MID );
		WriteArray( "None" , m_DefaultFiles );

		// Footer:
		FileData.WriteStr8( VCXPROJ_FILTER_FOOTER );

		const eg_d_string16 OutputFilename = GetProjectFilename( true );
		EGEdUtility_SaveIfUnchanged( *OutputFilename , FileData , true );
	}

private:

	eg_d_string16 GetProjectFilename( eg_bool bFilters ) const
	{
		eg_d_string16 Out = m_BuildInfo.ProjectDir;
		Out += *m_BuildInfo.ProjectName;
		Out += L".vcxproj";
		if( bFilters )
		{
			Out += L".filters";
		}

		Out = EGPath2_CleanPath( *Out , '/' );

		return Out;
	}

	void FindFiles()
	{
		EGLogf( eg_log_t::General , "Finding files..." );

		egOsFileInfo ProjectTree;
		EGOsFile_BuildTree( *m_BuildInfo.ProjectDir , EGVSGen_WantsFile , ProjectTree );
		ProjectTree.Sort( []( const egOsFileInfo& Left , const egOsFileInfo& Right ) -> eg_bool
		{
			if( Left.Attributes.bDirectory && !Right.Attributes.bDirectory )
			{
				return true;
			}
			if( !Left.Attributes.bDirectory && Right.Attributes.bDirectory )
			{
				return false;
			}

			return EGString_CompareI( *Left.Name , *Right.Name ) < 0;
		} );

		ProjectTree.WalkTree( [this]( eg_cpstr16 Filename , const egOsFileAttributes& FileAttr ) -> void
		{
			const eg_d_string16 CorrectedFilename = EGPath2_CleanPath( Filename , '\\' );

			if( m_BuildInfo.bForceEmptyProject )
			{
				return;
			}

			if( !EGVSGen_bIncludeData )
			{
				if( EGString_EqualsI( Filename , L"data" ) || EGString_BeginsWithI( Filename , L"data/" )
					|| EGString_EqualsI( Filename , L"data_src" ) || EGString_BeginsWithI( Filename , L"data_src/" ) )
				{
					return;
				}
			}

			if( m_BuildInfo.ProjectRoots.Len() > 0 )
			{
				eg_bool bInRoot = false;

				for( const eg_d_string16& Root : m_BuildInfo.ProjectRoots )
				{
					if( EGString_EqualsICount( Filename , *Root , Root.Len() ) )
					{
						bInRoot = true;
					}
				}

				if( !bInRoot )
				{
					return;
				}
			}

			if( FileAttr.bDirectory )
			{
				m_AllFilters.Append( *CorrectedFilename );
			}
			else
			{
				const eg_vsgen_ext_t ExtType = EGVSGen_GetExtType( *CorrectedFilename );
				if( !ShouldExclude( Filename ) )
				{
					switch( ExtType )
					{
					case eg_vsgen_ext_t::Default:
						m_DefaultFiles.Append( *CorrectedFilename );
						break;
					case eg_vsgen_ext_t::Ignore:
						assert( false ); // Should never have gotten ignores in this list
						break;
					case eg_vsgen_ext_t::Compile:
						m_CompileFiles.Append( *CorrectedFilename );
						break;
					case eg_vsgen_ext_t::Include:
						m_HeaderFiles.Append( *CorrectedFilename );
						break;
					case eg_vsgen_ext_t::Resource:
						m_ResourceFiles.Append( *CorrectedFilename );
						break;
					default:
						assert( false ); // Shouldn't exist.
						break;

					}
				}
			}
		});
	}
	
	void FindIncludePaths()
	{
		eg_bool bAddedFirst = false;
		auto AddPath = [this,&bAddedFirst]( eg_cpstr16 NewPath ) -> void
		{
			if( bAddedFirst )
			{
				m_IncludePaths += ";";
			}
			bAddedFirst = true;
			m_IncludePaths += NewPath;
		};

		// Directories added directly:
		for( const eg_d_string16& Path : m_BuildInfo.IncludeDirs )
		{
			AddPath( *Path );
		}

		// Roots must be scanned out
		for( const eg_d_string16& Root : m_BuildInfo.IncludeRoots )
		{
			AddPath( *Root );

			const eg_d_string16 FullRoot = EGPath2_GetFullPathRelativeTo( *Root , *m_BuildInfo.ProjectDir );

			egOsFileInfo ProjectTree;
			EGOsFile_BuildTree( *FullRoot , EGVSGen_WantsIncludeRoot , ProjectTree );
			ProjectTree.WalkTree( [this,&ProjectTree,&AddPath]( eg_cpstr16 Filename , const egOsFileAttributes& FileAttr ) -> void
			{
				unused( FileAttr );

				const eg_d_string16 FullFilename = ProjectTree.Name + "/" + Filename;
				const eg_d_string16 RelativeDir = EGPath2_GetRelativePathTo( *FullFilename , *m_BuildInfo.ProjectDir );
				AddPath( *RelativeDir );
			} );
		}
	}

	eg_bool ShouldExclude( eg_cpstr16 Filename )
	{
		for( const eg_d_string16& ExcludeFile : m_BuildInfo.ExcludeFiles )
		{
			if( EGString_EndsWithI( Filename , *ExcludeFile ) )
			{
				return true;
			}
		}

		return false;
	}
};

static void EGVSGen_BuildProject( const egVSGenBuildInfo& BuildInfo )
{
	EGVSGenProject Project( BuildInfo );
}

enum class eg_vsgen_win64_tool_t
{
	App,
	StaticLib,
	SccDLL,
};

static void EGVSGen_BuildWin64ToolProject( eg_cpstr8 ToolName , eg_vsgen_win64_tool_t ToolType , bool bGenerateFilters )
{
	egVSGenBuildInfo ProjectBuildInfo;
	ProjectBuildInfo.ProjectName = ToolName;
	ProjectBuildInfo.IncludeRoots.Append( L"../../eglib" );
	ProjectBuildInfo.IncludeRoots.Append( L"../../foundation" );
	ProjectBuildInfo.IncludeDirs.Append( L"../../lib3p" );
	ProjectBuildInfo.IncludeDirs.Append( L"./" );
	ProjectBuildInfo.ProjectDir = EGSFormat16( L"core/tools/{0}" , ToolName );
	ProjectBuildInfo.bGenerateFilters = bGenerateFilters;
	switch( ToolType )
	{
	case eg_vsgen_win64_tool_t::StaticLib:
		ProjectBuildInfo.TemplateName = "vst_editor_tool_lib";
		ProjectBuildInfo.IncludeRoots.Clear();
		ProjectBuildInfo.IncludeRoots.Append( L"../../eglib" );
		ProjectBuildInfo.IncludeRoots.Append( L"../../foundation" );
		ProjectBuildInfo.IncludeRoots.Append( L"../../engine" );
		ProjectBuildInfo.IncludeRoots.Append( L"./" );

		break;
	case eg_vsgen_win64_tool_t::SccDLL:
		ProjectBuildInfo.TemplateName = "vst_scc_dll";
		break;
	case eg_vsgen_win64_tool_t::App:
		ProjectBuildInfo.TemplateName = "vst_editor_tool_exe";
		break;
	}

	EGVSGen_BuildProject( ProjectBuildInfo );
}

static void EGVSGen_BuildCmd64Tool( eg_cpstr8 ToolName )
{
	egVSGenBuildInfo ProjectBuildInfo;
	ProjectBuildInfo.ProjectName = ToolName;
	ProjectBuildInfo.TemplateName = "vstemplate_cmd64_tool";
	ProjectBuildInfo.ProjectDir = EGSFormat16( L"core/tools/{0}" , ToolName );
	ProjectBuildInfo.Defines = "__EGEDITOR__";
	ProjectBuildInfo.IncludeRoots.Append( L"../../foundation" );
	ProjectBuildInfo.IncludeRoots.Append( L"../../eglib" );
	ProjectBuildInfo.IncludeDirs.Append( L"../../lib3p" );
	ProjectBuildInfo.IncludeDirs.Append( L"../../lib3p/freetype255/include" );
	ProjectBuildInfo.bGenerateFilters = false;
	EGVSGen_BuildProject( ProjectBuildInfo );
}

static void EGVSGen_BuildGameProjects()
{
	auto BuildGameProjectInternal = []( eg_cpstr8 GameName ) -> void
	{
		egVSGenBuildInfo ProjectBuildInfo;
		ProjectBuildInfo.ProjectName = GameName;
		ProjectBuildInfo.TemplateName = "vst_game_exe";
		ProjectBuildInfo.ProjectDir = EGSFormat16( L"games/{0}" , GameName );
		ProjectBuildInfo.Defines = "";
		ProjectBuildInfo.IncludeRoots.Append( L"code" );
		ProjectBuildInfo.IncludeRoots.Append( L"../../core/eglib" );
		ProjectBuildInfo.IncludeRoots.Append( L"../../core/engine" );
		ProjectBuildInfo.IncludeRoots.Append( L"../../core/foundation" );
		ProjectBuildInfo.IncludeDirs.Append( L"../../core/lib3p" );
		EGVSGen_BuildProject( ProjectBuildInfo );
	};

	egOsFileInfo GameFolders;
	EGOsFile_GetDirectoryContents( L"games" , GameFolders );

	GameFolders.WalkTree( [&BuildGameProjectInternal]( eg_cpstr16 Filename , const egOsFileAttributes& FileAttr ) -> void
	{
		if( FileAttr.bDirectory )
		{
			EGLogf( eg_log_t::General , "Building game data for %s..." , *eg_s_string_sml8(Filename) );
			BuildGameProjectInternal( *eg_s_string_sml8(Filename) );
		}
	} );
}

static void EGVSGen_BuildResLib( eg_cpstr8 ToolName )
{
	egVSGenBuildInfo ProjectBuildInfo;
	ProjectBuildInfo.ProjectName = ToolName;
	ProjectBuildInfo.TemplateName = "vst_res_dll";
	ProjectBuildInfo.ProjectDir = EGSFormat16( L"core/tools/{0}" , ToolName );
	ProjectBuildInfo.Defines = "";
	ProjectBuildInfo.IncludeRoots.Append( L"../../eglib" );
	ProjectBuildInfo.IncludeRoots.Append( L"../../foundation" );
	ProjectBuildInfo.IncludeRoots.Append( L"../../engine" );
	ProjectBuildInfo.IncludeDirs.Append( L"../../lib3p" );
	ProjectBuildInfo.bGenerateFilters = false;
	EGVSGen_BuildProject( ProjectBuildInfo );
}

static void EGVSGen_BuildLibs()
{
	// eglib
	{
		egVSGenBuildInfo ProjectBuildInfo;
		ProjectBuildInfo.ProjectName = "eglib";
		ProjectBuildInfo.TemplateName = "vst_lib";
		ProjectBuildInfo.ProjectDir = EGSFormat16( L"core/{0}" , "eglib" );
		ProjectBuildInfo.Defines = "";
		ProjectBuildInfo.IncludeRoots.Append( L"./" );
		ProjectBuildInfo.IncludeDirs.Append( L"../lib3p" );
		ProjectBuildInfo.bGenerateFilters = true;
		EGVSGen_BuildProject( ProjectBuildInfo );
	}

	// egfoundation
	{
		egVSGenBuildInfo ProjectBuildInfo;
		ProjectBuildInfo.ProjectName = "egfoundation";
		ProjectBuildInfo.TemplateName = "vst_lib";
		ProjectBuildInfo.ProjectDir = EGSFormat16( L"core/{0}" , "foundation" );
		ProjectBuildInfo.Defines = "";
		ProjectBuildInfo.IncludeRoots.Append( L"./" );
		ProjectBuildInfo.IncludeRoots.Append( L"../eglib" );
		ProjectBuildInfo.bGenerateFilters = true;
		EGVSGen_BuildProject( ProjectBuildInfo );
	}
}

static void EGVSGen_BuildEngineProject()
{
	egVSGenBuildInfo ProjectBuildInfo;
	ProjectBuildInfo.ProjectName = "eg";
	ProjectBuildInfo.TemplateName = "vst_engine_lib";
	ProjectBuildInfo.ProjectDir = EGSFormat16( L"core/{0}" , "engine" );
	ProjectBuildInfo.Defines = "";
	ProjectBuildInfo.IncludeRoots.Append( L"../eglib" );
	ProjectBuildInfo.IncludeRoots.Append( L"../foundation" );
	ProjectBuildInfo.IncludeRoots.Append( L"./" );
	ProjectBuildInfo.IncludeRoots.Append( L"../tools" );
	ProjectBuildInfo.IncludeRoots.Append( L"../libs/inc" );
	ProjectBuildInfo.IncludeRoots.Append( L"../lib3p/libogg132/include" );
	ProjectBuildInfo.IncludeRoots.Append( L"../lib3p/libvorbis135/include" );
	ProjectBuildInfo.IncludeDirs.Append( L"../lib3p/steamworks/sdk/public" );
	ProjectBuildInfo.IncludeDirs.Append( L"../lib3p/xaudio2/include" );
	ProjectBuildInfo.IncludeDirs.Append( L"../lib3p/xinput/include" );
	ProjectBuildInfo.IncludeDirs.Append( L"../lib3p" );
	ProjectBuildInfo.bGenerateFilters = true;
	EGVSGen_BuildProject( ProjectBuildInfo );
}

static void EGVSGen_BuildLib3P()
{
	egVSGenBuildInfo ProjectBuildInfo;
	ProjectBuildInfo.ProjectName = "lib3p";
	ProjectBuildInfo.TemplateName = "vstemplate_lib3p_x64";
	ProjectBuildInfo.ProjectDir = L"core/lib3p";

	ProjectBuildInfo.ProjectRoots.Append( L"expat" );
	ProjectBuildInfo.ProjectRoots.Append( L"freetype255" );
	ProjectBuildInfo.ProjectRoots.Append( L"libogg132" );
	ProjectBuildInfo.ProjectRoots.Append( L"libvorbis135" );
	ProjectBuildInfo.ProjectRoots.Append( L"zlib128" );
	ProjectBuildInfo.ProjectRoots.Append( L"fs_sys2" );

	ProjectBuildInfo.Defines = "WIN32;FT_CONFIG_OPTION_SYSTEM_ZLIB;FT2_BUILD_LIBRARY;_CRT_SECURE_NO_WARNINGS;COMPILED_FROM_DSP;XML_STATIC;_CRT_NONSTDC_NO_DEPRECATE";
	
	ProjectBuildInfo.IncludeRoots.Append( L"libogg132/include" );
	ProjectBuildInfo.IncludeRoots.Append( L"libvorbis135/include" );

	ProjectBuildInfo.IncludeDirs.Append( L"zlib128" );
	ProjectBuildInfo.IncludeDirs.Append( L"freetype255/include" );

	ProjectBuildInfo.ExcludeFiles.Append( L"lzw/ftzopen.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"autofit/autofit.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"autofit/aflatin2.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"base/ftbase.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"bdf/bdf.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"bzip2/bzip2.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"cache/ftcache.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"cff/cff.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"cid/cid.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"gxvalid/gxvalid.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"otvalid/otvalid.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"pcf/pcf.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"pfr/pfr.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"psaux/psaux.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"pshinter/pshinter.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"psnames/psnames.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"raster/raster.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"sfnt/sfnt.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"smooth/smooth.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"truetype/truetype.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"type1/type1.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"cid/type1cid.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"type42/type42.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"winfonts/winfonts.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"base/ftmac.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"tools/apinames.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"tools/test_afm.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"tools/test_bbox.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"tools/test_trig.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"tools/ftrandom/ftrandom.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"gzip/adler32.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"gzip/infblock.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"gzip/infcodes.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"gzip/inflate.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"gzip/inftrees.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"gzip/infutil.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"gzip/zutil.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"gxvfgen.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"libvorbis135/lib/tone.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"libvorbis135/lib/psytune.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"luac.c" );
	ProjectBuildInfo.ExcludeFiles.Append( L"barkmel.c" );

	ProjectBuildInfo.bGenerateFilters = true;

	EGVSGen_BuildProject( ProjectBuildInfo );
}

static void EGVSGen_BuildPreTools()
{
	egVSGenBuildInfo ProjectBuildInfo;
	ProjectBuildInfo.ProjectName = "BuildTools";
	ProjectBuildInfo.TemplateName = "vst_files_only";
	ProjectBuildInfo.ProjectDir = L"core/BuildTools";
	ProjectBuildInfo.ProjectRoots.Append( L"bat" );
	ProjectBuildInfo.ProjectRoots.Append( L"python" );
	ProjectBuildInfo.ProjectRoots.Append( L"bin" );
	ProjectBuildInfo.ProjectRoots.Append( L"vst" );
	ProjectBuildInfo.bGenerateFilters = true;
	EGVSGen_BuildProject( ProjectBuildInfo );
}

static void EGVSGen_BuildEGData()
{
	egVSGenBuildInfo ProjectBuildInfo;
	ProjectBuildInfo.ProjectName = "egdata";
	ProjectBuildInfo.TemplateName = "vst_files_only";
	ProjectBuildInfo.ProjectDir = L"core/egdata";
	ProjectBuildInfo.bGenerateFilters = true;
	if( !EGVSGen_bIncludeData )
	{
		ProjectBuildInfo.bForceEmptyProject = true;
	}
	EGVSGen_BuildProject( ProjectBuildInfo );
}

int EGMakeVSGen_main( int argc , char* argv[] )
{
	unused( argc , argv );

	EGEdUtility_Init();
	
	EGLogf( eg_log_t::General , "Generating project files..." );

	EGCmdLineParms CmdLineParms( argc , argv );
	const eg_bool bBuild3rdPartyLibs = CmdLineParms.ContainsType( L"-buildlib3p" );

	EGOsFile_SetCWD( *EGToolsHelper_GetBuildVar( "EGSRC" ) );

	EGVSGen_BuildGameProjects();
	EGVSGen_BuildCmd64Tool( "egmake2" );
	EGVSGen_BuildWin64ToolProject( "EGEdLib" , eg_vsgen_win64_tool_t::StaticLib , true );
	EGVSGen_BuildWin64ToolProject( "EGWndTest" , eg_vsgen_win64_tool_t::App , false );
	EGVSGen_BuildResLib( "EGEdResLib" );
	EGVSGen_BuildWin64ToolProject( "EGEdApp" , eg_vsgen_win64_tool_t::App , false );
	EGVSGen_BuildWin64ToolProject( "EGScc" , eg_vsgen_win64_tool_t::SccDLL , false );
	EGVSGen_BuildWin64ToolProject( "EGEdConfig" , eg_vsgen_win64_tool_t::App , false );
	EGVSGen_BuildLibs();
	EGVSGen_BuildEngineProject();
	EGVSGen_BuildPreTools();
	EGVSGen_BuildEGData();
	if( bBuild3rdPartyLibs )
	{
		EGVSGen_BuildLib3P();
	}
	else
	{
		EGLogf( eg_log_t::General , "Skipping 3rd party libraries, use -buildlib3p to build them." );
	}

	EGEdUtility_Deinit();

	return 0;
}

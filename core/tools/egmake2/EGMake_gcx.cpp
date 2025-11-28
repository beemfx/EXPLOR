// (c) 2016 Beem Media

#include "EGGcxTypes.h"
#include "EGGcxFile.h"
#include "EGGcxBuildTypes.h"
#include "EGGcxFloor.h"
#include "EGGcxRegion.h"
#include "EGXmlBase.h"
#include "EGFileData.h"
#include "EGMake.h"
#include "EGTGAWriter.h"
#include "EGRendererTypes.h"
#include "EGParse.h"
#include "EGMakeBuildConfig.h"
#include "EGPath2.h"
#include "EGEngineConfig.h"
#include "EGEdUtility.h"
#include "EGGcxBuildData.h"
#include "EGGcxMapMetaData.h"
#include "EGPath2.h"

eg_bool EGMake_gcx( eg_cpstr strIn, eg_cpstr strOutEmap , eg_bool bCompress )
{
	unused( strOutEmap );
	
	#if 0
	eg_vec4 A( 0 , 0 , 0 , 1.f );
	eg_vec4 B( 3.f , 2.f , 0 , 1.f );

	eg_vec4 Min( 1.f , 1.f , -.5f , 1.f );
	eg_vec4 Max( 2.f , 2.f , .5f , 1.f );

	eg_aabb Aabb;
	Aabb.Min = Min;
	Aabb.Max = Max;

	eg_vec4 AHit;
	eg_vec4 BHit;

	eg_intersect_t ISect = Aabb.ComputeIntersection( A , B , &AHit , &BHit );

	return false;
	#else
	EGLogf( eg_log_t::General , "Gcx Version 2 %s %s" , __DATE__ , __TIME__ );

	EGEdUtility_Init();
	eg_string_big GameToBuild = EGMake_GetGameToBuildFromReg();
	EGMakeBuildConfig::Get().LoadGameConfig( "core" );
	EGMakeBuildConfig::Get().LoadGameConfig( GameToBuild );

	eg_string_big DefaultTileInfoPath = EGString_Format( "%s/games/%s/data/%s" , EGMake_GetEGSRC() , GameToBuild.String() , *EGMakeBuildConfig::Get().GetConfigValue( "GcxDefaultBuildInfo") );
	DefaultTileInfoPath = *EGPath2_CleanPath( DefaultTileInfoPath , '\\' );

	EGGcxFile Compiler( strIn , EGMake_GetInputPath( FINFO_DIR ) );

	gcxSaveProps SaveProps;
	SaveProps.DefaultTileLib = DefaultTileInfoPath;
	SaveProps.OutputDir = EGMake_GetOutputPath( FINFO_DIR );
	SaveProps.BuildConfigFile = EGPath2_CleanPath( EGString_Format( "%s.BuildConfig" , EGMake_GetInputPath( FINFO_NOEXT_FILE ) ).String() , '/' );

	EGGcxSaveCb SaveFn = []( eg_cpstr FullFilename , const class EGFileData& FileData ) -> eg_bool
	{
		eg_bool bSaved = EGEdUtility_SaveFile( FullFilename, FileData );
		EGEdUtility_RevertFile( FullFilename , true );
		return bSaved;
	};

	EGGcxWriteMtrlCb WriteMtrlFn = []( eg_uint Index , const gcxBuildMaterialData& Material ) -> eg_string_big
	{
		EGMaterialDef MtrlDef;
		EGString_Copy( MtrlDef.m_strTex[0], *Material.Texture0Path.FullPath, countof( MtrlDef.m_strTex[0] ) );
		EGString_Copy( MtrlDef.m_strTex[1], *Material.Texture1Path.FullPath, countof( MtrlDef.m_strTex[1] ) );
		EGString_Copy( MtrlDef.m_strTex[2], *Material.Texture2Path.FullPath, countof( MtrlDef.m_strTex[2] ) );
		EGString_Copy( MtrlDef.m_strTex[3], *Material.Texture3Path.FullPath, countof( MtrlDef.m_strTex[3] ) );
		EGString_Copy( MtrlDef.m_strVS, *Material.ShaderPath.FullPath, countof( MtrlDef.m_strVS ) );
		EGString_Copy( MtrlDef.m_strPS, *Material.ShaderPath.FullPath, countof( MtrlDef.m_strPS ) );
		MtrlDef.m_Mtr.Ambient = eg_color( 1, 1, 1, 1 );

		eg_string_big Line = MtrlDef.CreateXmlTag( Index + 1 );
		return Line;
	};

	EGGcxProcessPropsCb ProcessPropsFn = []( eg_cpstr PropsFilename , const gcxExtraMapProps& Props ) -> void
	{
		egRflEditor MetaDataEditor;
		EGGcxMapMetaData* MetaData = EGCast<EGGcxMapMetaData>(EGDataAsset::CreateDataAsset( &EGGcxMapMetaData::GetStaticClass() , eg_mem_pool::DefaultHi , MetaDataEditor ));
		if( MetaData )
		{
			MetaData->SetTo( Props );
			EGDataAsset::SaveDataAsset( EGString_ToWide(PropsFilename) , MetaData , MetaDataEditor );
			EGEdUtility_RevertFile( EGString_ToWide(PropsFilename) , true );
			EGDeleteObject( MetaData );
			MetaData = nullptr;
		}
	};

	eg_bool bSucc = Compiler.SaveAsXmlEmap( SaveFn , WriteMtrlFn , ProcessPropsFn , SaveProps , bCompress );

	EGEdUtility_Deinit();

	EGMakeBuildConfig::Get().ClearConfig();

	return bSucc;
	#endif
}

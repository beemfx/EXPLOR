// (c) 2017 Beem Media

#include "EGClassImport.h"
#include "EGSettings2Types.h"

static EGSettingsBool EGClassImport_ImportBool( "ImportBool" , eg_loc("ImportBool","Debug: Secret Setting") , false , EGS_F_DEBUG_EDITABLE|EGS_F_USER_SAVED );

void EGClassImport_ImportClasses()
{
	#define IMPORT_CLASS( _name_ ) extern EGClass& ##_name_##_GetClass(); ##_name_##_GetClass();

	IMPORT_CLASS( EGGameSettingsMenu );
	IMPORT_CLASS( EGLevelListMenu );
	IMPORT_CLASS( EGHUDLoading );
	IMPORT_CLASS( EGXAudioDevice );
	IMPORT_CLASS( EGNoAudioDevice );
	IMPORT_CLASS( EGOggVorbisFile );
	IMPORT_CLASS( EGD11R_Renderer );
	IMPORT_CLASS( EGBpPhysSim );
	IMPORT_CLASS( EGGcxBuildData );
	IMPORT_CLASS( EGComponent );
	IMPORT_CLASS( EGVisualComponent );
	IMPORT_CLASS( EGPhysBodyComponent );
	IMPORT_CLASS( EGMeshComponent );
	IMPORT_CLASS( EGSkelMeshComponent );
	IMPORT_CLASS( EGSoundPlayerComponent );
	IMPORT_CLASS( EGTextNodeComponent );
	IMPORT_CLASS( EGSkyboxMeshComponent );
	IMPORT_CLASS( EGMeshShadowComponent );
	IMPORT_CLASS( EGSoundEventComponent );
	IMPORT_CLASS( EGStaticMeshComponent );
	IMPORT_CLASS( EGBillboardComponent );
	IMPORT_CLASS( EGAmbientSoundComponent );
	IMPORT_CLASS( EGWorldObjectEntity );
	IMPORT_CLASS( EGWorldObjectMap );
	IMPORT_CLASS( EGWorldObjectTerrain );
	IMPORT_CLASS( EGGcxMapMetaData );
	IMPORT_CLASS( EGDebugDrawCloseLightsComponent );

	#undef IMPORT_CLASS

	eg_string_crc SampleText = eg_loc("Sample Text","Sample Text");
}

#define EGRFL_SYSTEM_HEADER "core.link.reflection.hpp"
#include "EGRflLinkSystem.hpp"
#undef EGRFL_SYSTEM_HEADER

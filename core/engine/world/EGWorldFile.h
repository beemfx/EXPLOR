// (c) 2018 Beem Media

#pragma once

#include "EGXMLBase.h"
#include "EGAssetPath.h"
#include "EGWorldFile.reflection.h"

class EGWorldObject;
class EGFileData;
class EGReflectionDeserializer;
class EGEntWorld;

egreflect enum class eg_world_file_spawn_ent_t
{
	AtCamera ,
	InFrontOfCamera ,
	InFrontOfCameraSnapToGrid ,
};

egreflect struct egWorldFileEditorSpawnAtCameraData : public IEGSFormatHandler
{
	egprop eg_asset_path Ent = eg_asset_path_special_t::GameEntities;
	egprop eg_world_file_spawn_ent_t SpawnMode = eg_world_file_spawn_ent_t::AtCamera;
	egprop eg_int NextIndex = 0;
	egprop eg_d_string LabelStringFmt = "Entity_{0:INDEX}";
	egprop eg_d_string_ml InitStringFmt = "InitFunction({0:INDEX});";

	virtual void FormatText( eg_cpstr16 Flags , EGSParmFormatter& Formatter ) const override
	{
		const eg_string_crc FormatFlag = Formatter.GetNextFlag( &Flags );

		switch_crc(FormatFlag)
		{
			case_crc("INDEX"):
			{
				Formatter.SetNumber( NextIndex , Flags );
			} break;
		}
	}
};

egreflect struct egWorldFileEditorSettings
{
	egprop eg_bool bDrawGrid = true;
	egprop eg_vec3 GridOffset = eg_vec3(0.f,0.f,0.f);
	egprop eg_real GridSpacing = 1.f;
	egprop eg_ivec2 GridSize = eg_ivec2(20,20);
	egprop eg_color32 GridColor = eg_color32(128,128,128);
	egprop eg_color32 BackgroundColor = eg_color32(50,50,50);
	egprop eg_bool bMoveInFrontOfCameraTargetsGrid = true;
	egprop eg_bool bSnapToGridAlsoAlignsToAxis = true;
	egprop egWorldFileEditorSpawnAtCameraData SpawnAtCamera;
	
};

egreflect struct egWorldFileGlobals
{
	egprop eg_vec3 GravityVector = eg_vec3(0.f,-9.8f,0.f);
	egprop egWorldFileEditorSettings EditorSettings;
};

egreflect struct egWorldFileEditorConfig
{
	egprop eg_transform CameraPose = CT_Default;
};

class EGWorldFile : public EGObject , protected IXmlBase
{
	EG_CLASS_BODY( EGWorldFile , EGObject )

private:

	egWorldFileGlobals      m_Globals;
	egRflEditor             m_GlobalsEd;
	egWorldFileEditorConfig m_EditorConfig;
	egRflEditor             m_EditorConfigEd;
	EGArray<EGWorldObject*> m_WorldObjects;

	EGArray<EGWorldObject*>   m_DeserializerStack;
	EGReflectionDeserializer* m_Deserializer = nullptr;
	EGReflectionDeserializer* m_PropertiesDeserializer = nullptr;

public:

	virtual void OnDestruct() override;

	void Load( const EGFileData& MemFile , eg_cpstr RefFilename , eg_bool bForEditor );
	void Save( EGFileData& MemFile );
	void Reset();
	void ClearEditor();

	void ApplyToWorldPreLoad( EGEntWorld* World ) const;
	void ApplyToWorldPostLoad( EGEntWorld* World ) const;
	eg_bool IsPreLoadComplete( const EGEntWorld* World ) const;
	void ApplyToWorldPreview( EGEntWorld* World ) const;
	void ClearAllFromPreview( EGEntWorld* World ) const;

	void GetAllWorldObjects( EGArray<const EGWorldObject*>& Out ) const;
	void GetAllWorldObjects( EGArray<EGWorldObject*>& Out );

	egRflEditor* GetGlobalsEditor() { if( !m_GlobalsEd.IsValid() ){ InitGlobalsEditor(); } assert( m_GlobalsEd.GetData() == &m_Globals ); return &m_GlobalsEd; }
	egRflEditor* GetEditorConfigEditor() { if( !m_EditorConfigEd.IsValid() ){ InitEditorConfigEditor(); } assert( m_EditorConfigEd.GetData() == &m_EditorConfig ); return &m_EditorConfigEd; }

	EGWorldObject* InsertNewWorldObject( EGClass* ObjectClass );
	void DeleteWorldObject( EGWorldObject* WorldObject );
	void MoveWorldObject( EGWorldObject* ObjToMove, EGWorldObject* ObjToMoveBefore, EGWorldObject* ObjParent );

	egWorldFileEditorSettings& GetEditorFileSettings() { return m_Globals.EditorSettings; }
	egWorldFileEditorConfig& GetMutableEditorConfig() { return m_EditorConfig; }

private:

	void InitGlobalsEditor();
	void InitEditorConfigEditor();
	eg_d_string GenerateUniqueName( EGClass* ObjClass );

protected:

	virtual void OnTag( const eg_string_base& Tag , const EGXmlAttrGetter& AttGet );
	virtual void OnTagEnd( const eg_string_base& Tag );
	virtual eg_cpstr XMLObjName() const { return "EGWorldFile"; }
};

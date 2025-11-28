// (c) 2011 Beem Media

#pragma once

#include "EGEntDefTypes.h"
#include "EGEngineSerializeTypes.h"
#include "EGXMLBase.h"
#include "EGClassName.h"
#include "EGCrcDb.h"
#include "EGEntDef.reflection.h"

extern eg_cpstr ENT_RENDER_FILTER_WORLD;
extern eg_cpstr ENT_RENDER_FILTER_PRE_FX;
extern eg_cpstr ENT_RENDER_FILTER_POST_FX;

class EGTimeline;
class EGReflectionDeserializer;
class EGComponent;

extern EGClass& EGEnt_GetClass();
extern EGClass& EGUiWidget_GetClass();

struct egEntDefEditNeeds
{
	eg_bool bRefreshEditor = false;
	eg_bool bRefreshPreivew = false;
};

egreflect class EGEntDef : protected IXmlBase
{
public:

	EGEntDef( eg_ctor_t Ct );	
	EGEntDef( eg_cpstr Filename );
	~EGEntDef(){ Unload(); }

//Public data and interface:
public:

	egprop eg_d_string      m_Id                  = "";
	egprop eg_ent_class_t   m_ClassType           = eg_ent_class_t::None;
	egprop eg_class_name    m_EntityClassName     = &EGEnt_GetClass();
	egprop eg_class_name    m_UiClassName         = &EGUiWidget_GetClass();
	egprop eg_string_crc    m_DefaultRenderFilter = EGCrcDb::StringToCrc(ENT_RENDER_FILTER_PRE_FX);
	egprop eg_bool          m_bIsSerialized       = true;
	egprop eg_bool          m_bIsLit              = true;
	egprop eg_bool          m_bIsAlwaysVisible    = false;
	egprop eg_aabb_ed       m_BaseBounds;
	egprop egEntDefEdConfig m_EditorConfig;

	egRflEditor          m_Editor;
	EGArray<EGTimeline*> m_Timelines;
	EGList<EGComponent>  m_ComponentTree = EGList<EGComponent>::DEFAULT_ID;

public:

	void LoadForDict(eg_cpstr strFile);
	void LoadForEditor( const class EGFileData& File , eg_cpstr RefName );
	void Unload( void );
	void SerializeToXml( class EGFileData& File ) const;
	void QueryEventList( EGArray<eg_d_string>& Out ) const;
	void GetComponents( EGArray<EGComponent*>& DefsOut );
	void CreateAssets() const;
	void DestroyAssets() const;
	egRflEditor* GetEditor() { return &m_Editor; }
	egRflEditor* GetConfigEditor() { return m_Editor.GetChildPtr( "m_EditorConfig" ); }
	egEntDefEdConfig& GetConfig() { return m_EditorConfig; }
	void OnEditPropertyChanged( const egRflEditor* ChangedProperty , egEntDefEditNeeds& NeedsOut );

private:

	void PostLoad();
	void FinalizeDictDef();
	void GetComponentsRecursive( EGComponent* Parent , EGArray<EGComponent*>& DefsOut );

	void RefreshVisibleProperties( egRflEditor& ThisEditor );

private:

	mutable eg_size_t                 m_AssetHolderCount = 0;
	mutable eg_byte                   m_AssetHolderMem[104];
	mutable class EGEntComponentTree* m_AssetHolder = nullptr;

//XML Loading:
protected:
	
	eg_uint                   m_NextNodeId;
	eg_string_crc             m_CurAnim;
	eg_string_crc             m_NextTransformType;
	eg_bool                   m_NextTransformIsBase64:1;
	EGComponent*              m_CurReadComponent = nullptr;
	EGReflectionDeserializer* m_ConfigDeserializer = nullptr;

	//Because client and server templates load differently create some
	//virtual functions to separate the functionality:
	void OnData(eg_cpstr strData, eg_uint nLen);
	void OnTag_component( const EGXmlAttrGetter& Getter );
	void OnTag_timeline( const eg_string_base& Tag , const EGXmlAttrGetter& Getter );
	void OnTag_keyframe( const eg_string_base& Tag , const EGXmlAttrGetter& Getter );
	virtual eg_cpstr XMLObjName()const;
	
private:
	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& atts) override final;
	virtual void OnTagEnd( const eg_string_base& Tag ) override final;
};
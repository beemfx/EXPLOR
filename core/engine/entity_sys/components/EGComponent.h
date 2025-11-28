// (c) 2018 Beem Media

#pragma once

#include "EGObject.h"
#include "EGEntTypes.h"
#include "EGTween.h"
#include "EGComponent.reflection.h"

class EGReflectionDeserializer;
class EGEntComponentTree;
class EGWorldSceneGraph;

typedef void ( * egComponentStringQuery )( const EGComponent* Component , EGArray<eg_d_string>& Out );

struct egComponentInitData
{
	const EGComponent*  Def             = nullptr;
	EGComponent*        Parent          = nullptr;
	EGObject*           Owner           = nullptr;
	EGEntComponentTree* OwnerCompTree   = nullptr;
	eg_string_crc       DefaultRenderFilter = CT_Clear;
	eg_bool             bIsServer       = false;
	eg_bool             bIsClient       = false;
};

egreflect class EGComponent : public EGObject , public IListable
{
	EG_CLASS_BODY( EGComponent , EGObject )
	EG_FRIEND_RFL( EGComponent )

public:

	static egComponentStringQuery StaticQueryForSubMeshes;

protected:

	// Editable definition:
	egprop eg_d_string   m_Name            = "";
	egprop eg_string_crc m_ParentJointId   = CT_Clear;
	egprop eg_vec3       m_ScaleVector     = eg_vec3(1.f,1.f,1.f);
	egprop eg_transform  m_BasePose        = CT_Default;
	egprop eg_bool       m_bIsLockedInTool = false;

	// Common:
	eg_bool             m_bIsDefinition = false;
	EGComponent*        m_Parent        = nullptr;
	EGList<EGComponent> m_Children      = EGList<EGComponent>::DEFAULT_ID;
	eg_d_string         m_OwnerFilename = "";

	// Live component:
	egComponentInitData   m_InitData;
	eg_string_crc         m_CachedId    = CT_Clear;
	EGTween<eg_transform> m_Pose = CT_Default;
	EGTween<eg_vec4>      m_CachedScale = eg_vec4(1.f,1.f,1.f,1.f);
	eg_vec4               m_GlobalScale = eg_vec4(1.f,1.f,1.f,1.f);
	eg_int                m_MuteCount   = 0;

protected:

	egRflEditor               m_Editor       = CT_Clear;

private:

	EGReflectionDeserializer* m_Deserializer = nullptr;

public:

	virtual void OnConstruct() override;
	virtual void OnDestruct() override;

	virtual void InitComponentFromDef( const egComponentInitData& InitData );
	virtual void OnPostInitComponents(){ }
	virtual void OnEnterWorld() { }
	virtual void OnLeaveWorld() { }
	virtual void ActiveUpdate( eg_real DeltaTime );
	virtual void RelevantUpdate( eg_real DeltaTime );
	virtual void Draw( const eg_transform& ParentPose ) const;
	virtual void DrawForTool( const eg_transform& ParentPose ) const;
	virtual void AddToSceneGraph( const eg_transform& ParentPose , EGWorldSceneGraph* SceneGraph ) const;
	virtual void ScriptExec( const struct egTimelineAction& Action );
	virtual eg_transform GetJointPose( eg_string_crc JointName ) const { unused( JointName ); return eg_transform( CT_Default ); }
	virtual eg_transform GetPose( eg_string_crc JointName ) const;
	void SetPose( const eg_transform& NewPose ) { m_Pose.SetValue( NewPose ); }
	const eg_transform ComputeLocalPose() const;
	const eg_transform GetWorldPose( const eg_transform& OwnerPose ) const;
	eg_string_crc GetId() const { return m_CachedId; }
	void SetMuteAudio( eg_bool bMute );
	void SetScaleVec( const eg_vec4& ScaleVec );
	virtual void SetBone( eg_string_crc BoneId , const eg_transform& Transform ){ unused( BoneId , Transform ); }
	virtual void ClearCustomBones(){ }
	virtual void SetText( const class eg_loc_text& NewText ){ unused( NewText ); }
	virtual void SetTexture( eg_string_crc GroupId , eg_cpstr TexturePath ){ unused( GroupId , TexturePath ); }
	virtual void SetPalette( const eg_vec4& NewPalette ){ unused( NewPalette ); }
	virtual void QueryTextNodes( EGArray<eg_d_string>& Out ) const { unused( Out ); }
	virtual void QueryBones( EGArray<eg_d_string>& Out ) const { unused( Out ); }
	virtual void RefreshFromDef();
	virtual void Reset();
	const EGComponent* GetDef() const { return m_InitData.Def; }
	eg_bool IsAudioMuted() const { return m_MuteCount > 0; }
	const eg_vec4& GetScale() const { return m_CachedScale.GetCurrentValue(); }
	const eg_vec4& GetGlobalScale() const { return m_GlobalScale; }

	void InitNewInEditor( eg_cpstr ComponentName );

	const eg_d_string& GetName() const { return m_InitData.Def ? m_InitData.Def->m_Name : m_Name; }
	const eg_d_string GetEntOwnerDefId() const;
	const eg_d_string& GetOwnerFilename() const { return m_InitData.Def ? m_InitData.Def->m_OwnerFilename : m_OwnerFilename; }
	const eg_transform& GetBasePose() const { return m_BasePose; }
	void SetLockedInTool( eg_bool bNewValue ) { m_bIsLockedInTool = bNewValue; }
	eg_bool IsLockedInTool() const { return m_bIsLockedInTool; }
	virtual eg_bool CanHaveMoreThanOne() const { return true; }

	void SetParent( EGComponent* InParent ) { m_Parent = InParent; }
	EGComponent* GetParent() { return m_Parent; }
	const EGComponent* GetParent() const { return m_Parent; }
	EGList<EGComponent>& GetChildren() { return m_Children; }
	const EGList<EGComponent>& GetChildren() const { return m_Children; }

	void InitEditor();
	void ClearEditor();
	egRflEditor* GetEditor() { assert( m_bIsDefinition ); return &m_Editor; }
	void FinalizeDictDef();

	virtual void OnPropChanged( const egRflEditor& ChangedProperty , eg_bool& bNeedsRebuildOut ) { unused( ChangedProperty ); bNeedsRebuildOut = false; }
	virtual void RefreshEditableProperties();

	void BeginXmlComponentTag( EGComponent* Parent , const EGXmlAttrGetter& AttGet , eg_cpstr XmlFilename );
	void OnXmlTag( const eg_string_base& Tag , const EGXmlAttrGetter& AttGet, eg_cpstr XmlFilename );
	void OnXmlTagEnd( const eg_string_base& Tag );
	void EndXmlComponentTag();
	virtual void PostXmlLoad(){ m_Editor.ExecutePostLoad( *m_OwnerFilename , true ); RefreshEditableProperties(); }

	eg_string_big GetToolDesc() const;
	eg_uint GetDepth() const;
	void SerializeToXml( class EGFileData& File , eg_cpstr PrefixTabs ) const;
	static eg_string GetShortNameFromClass( EGClass* Class );

	eg_ent_role GetEntRole() const;
	eg_ent_world_role GetWorldRole() const;
	eg_bool IsServer() const;
	eg_bool IsClient() const;
	template <typename T> T* GetOwner(){ return EGCast<T>( m_InitData.Owner ); }
	template <typename T> const T* GetOwner() const { return EGCast<T>( m_InitData.Owner ); }

protected:

	void SetPropEditable( eg_cpstr Name , eg_bool bEditable );
};

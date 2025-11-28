// (c) 2017 Beem Media

#pragma once

#include "EGVisualComponent.h"
#include "EGMeshState.h"
#include "EGTimelineTypes.h"
#include "EGRendererTypes.h"
#include "EGEntTypes.h"
#include "EGMesh.h"
#include "EGAssetPath.h"
#include "EGEngineConfig.h"
#include "EGComboBoxEd.h"
#include "EGMeshComponent.reflection.h"

class EGMesh;
class EGSkel;
class EGMeshObj;
class EGSkelObj;
class EGEnt;
class EGMeshMgrObj;

egreflect struct egMeshPath
{
	egprop eg_asset_path Filename = "emesh";
};

egreflect struct egMeshMaterial
{
	egprop eg_asset_path Texture0     = EXT_TEX;
	egprop eg_asset_path Texture1     = EXT_TEX;
	egprop eg_asset_path Texture2     = EXT_TEX;
	egprop eg_asset_path Texture3     = EXT_TEX;
	egprop eg_asset_path VertexShader = "evs5";
	egprop eg_asset_path PixelShader  = "eps5";
	egprop eg_color32    Diffuse      = eg_color32(eg_color(1.f,1.f,1.f,1.f));
	egprop eg_color32    Ambient      = eg_color32(eg_color(0.f,0.f,0.f,0.f));
	egprop eg_color32    Specular     = eg_color32(eg_color(0.f,0.f,0.f,0.f));
	egprop eg_color32    Emissive     = eg_color32(eg_color(0.f,0.f,0.f,0.f));
	egprop eg_real       Power        = 1.f;

	operator EGMaterialDef () const
	{
		EGMaterialDef Out( CT_Default );
		EGString_Copy( Out.m_strTex[0] , *Texture0.FullPath , countof(Out.m_strTex[0]) );
		EGString_Copy( Out.m_strTex[1] , *Texture1.FullPath , countof(Out.m_strTex[1]) );
		EGString_Copy( Out.m_strTex[2] , *Texture2.FullPath , countof(Out.m_strTex[2]) );
		EGString_Copy( Out.m_strTex[3] , *Texture3.FullPath , countof(Out.m_strTex[3]) );
		EGString_Copy( Out.m_strVS , *VertexShader.FullPath , countof(Out.m_strVS) );
		EGString_Copy( Out.m_strPS , *PixelShader.FullPath , countof(Out.m_strPS) );
		Out.m_Mtr.Diffuse  = eg_color(Diffuse);
		Out.m_Mtr.Ambient  = eg_color(Ambient);
		Out.m_Mtr.Specular = eg_color(Specular);
		Out.m_Mtr.Emissive = eg_color(Emissive);
		Out.m_Mtr.Power = Power;
		return Out;
	}
};

egreflect struct egMeshMaterialGroup
{
	egprop eg_combo_box_crc_ed MeshGroupId;
	egprop egMeshMaterial      Material;
};

egreflect class EGMeshComponent : public egprop EGVisualComponent
{
	EG_CLASS_BODY( EGMeshComponent , EGVisualComponent )
	EG_FRIEND_RFL( EGMeshComponent )

protected:

	struct egSubMeshMaterial
	{
		eg_string_crc GroupId;
		EGMaterialDef Def;
		egv_material  Material;
		eg_string_crc TxPathHash;
		eg_bool bExternallyOwned;

		egSubMeshMaterial()
		: GroupId( CT_Clear )
		, Def( CT_Default )
		, Material( EGV_MATERIAL_NULL )
		, TxPathHash( CT_Clear )
		, bExternallyOwned( false )
		{
		}
	};

protected:

	egprop egMeshPath                   m_Mesh;
	egprop EGArray<egMeshMaterialGroup> m_Materials;

protected:

	const EGMeshComponent*     m_MeshDef;
	EGEnt*                     m_EntOwner;
	EGMeshObj*                 m_MeshObj;
	EGMesh*                    m_CreatedMesh;
	EGArray<egSubMeshMaterial> m_CreatedMaterials;
	mutable EGMeshState        m_MeshState;
	eg_real                    m_Life;
	eg_bool                    m_bReady:1;
	eg_bool                    m_bHasCustomMaterials;

public:

	using EGVisualComponent::SetPalette;

	virtual void SetTexture( eg_string_crc GroupId , eg_cpstr TexturePath ) override;
	virtual void SetMaterial( eg_string_crc GroupId , egv_material Material ) override;
	virtual void Reset() override;

	virtual void QueryBones( EGArray<eg_d_string>& Out ) const override;
	void QuerySubMeshes( EGArray<eg_d_string>& Out );
	eg_aabb GetMeshBounds() const;

	void GetTextNodes( EGArray<const EGMeshBase::egTextNode*>& NodesOut ) const;
	eg_cpstr GetTextNodeBoneId( eg_uint BoneId ) const;
	virtual eg_transform GetTextNodeBonePose( eg_uint BoneId ) const { unused( BoneId ); return  CT_Default; }
	const EGMeshState& GetMeshState() const { return m_MeshState; }

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnDestruct() override;
	virtual void RelevantUpdate( eg_real DeltaTime ) override;
	virtual void Draw( const eg_transform& ParentPose ) const override;
	virtual void ScriptExec( const struct egTimelineAction& Action );

	virtual void RefreshFromDef() override;
	virtual void OnPropChanged( const egRflEditor& ChangedProperty , eg_bool& bNeedsRebuildOut ) override;
	virtual void RefreshEditableProperties() override;

protected:

	egSubMeshMaterial* GetMaterial( eg_string_crc GroupId  );
	void ResolveMaterialOverrides();
	void ResetBones();
	void ResetMaterials();

	void InitSkinMaterials();
	void InitDefaultMaterials();

	void CreateAssets();
	void DestroyAssets();

	virtual eg_bool UpdateRenderAssetLoad();

	void OnAssetLoaded( EGMeshMgrObj* Obj );
};


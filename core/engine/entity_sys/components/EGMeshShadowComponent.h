// (c) 2018 Beem Media

#pragma once

#include "EGVisualComponent.h"
#include "EGWeakPtr.h"
#include "EGAssetPath.h"
#include "EGMeshShadowComponent.reflection.h"

class EGEnt;
class EGMeshObj;
class EGMesh;
class EGMeshMgrObj;
class EGMeshComponent;
class EGSkelMeshComponent;

egreflect struct egShadowMeshPath
{
	egprop eg_asset_path Filename = "emesh";
};

egreflect class EGMeshShadowComponent : public egprop EGVisualComponent
{
	EG_CLASS_BODY( EGMeshShadowComponent , EGVisualComponent )
	EG_FRIEND_RFL( EGMeshShadowComponent )

protected:

	egprop egShadowMeshPath m_ShadowMesh;

protected:

	const EGMeshShadowComponent*   m_ShadowDef;
	EGMeshObj*                     m_ShadowObj;
	EGMesh*                        m_Shadow;
	EGWeakPtr<EGEnt>               m_OwnerEnt;
	EGWeakPtr<EGMeshComponent>     m_OwnerMesh;
	EGWeakPtr<EGSkelMeshComponent> m_OwnerSkelMesh;
	eg_bool                        m_bHasBeenMadeCompatible:1;

public:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnDestruct() override;
	virtual void RelevantUpdate( eg_real DeltaTime ) override;
	virtual void Draw( const eg_transform& ParentPose ) const override;
	virtual void AddToSceneGraph( const eg_transform& ParentPose , EGWorldSceneGraph* SceneGraph ) const override;

	eg_ent_id ForShadowDrawGetEntId() const;
	void ForShadowDrawDraw( const eg_transform& BasePose ) const;

protected:

	void OnAssetLoaded( EGMeshMgrObj* Obj );
	void MakeCompatible();
};

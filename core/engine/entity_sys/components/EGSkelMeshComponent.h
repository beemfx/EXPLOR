// (c) 2017 Beem Media

#pragma once

#include "EGMeshComponent.h"
#include "EGMeshState.h"
#include "EGTimelineTypes.h"
#include "EGRendererTypes.h"
#include "EGEntTypes.h"
#include "EGMesh.h"

class EGMesh;
class EGSkel;
class EGMeshObj;
class EGSkelObj;
class EGEnt;

egreflect struct egSkelPath
{
	egprop eg_string_crc Id = CT_Clear;
	egprop eg_asset_path Filename = "eskel";
};

egreflect class EGSkelMeshComponent : public egprop EGMeshComponent
{
	EG_CLASS_BODY( EGSkelMeshComponent , EGMeshComponent )
	EG_FRIEND_RFL( EGSkelMeshComponent )

protected:

	struct egNamedSkel
	{
		eg_string_crc Id;
		EGSkelObj*    SkelObj;
		EGSkel*       Skel;
	};

	struct egCustomBone
	{
		eg_string_crc BoneId;
		eg_transform  Transform;
	};

protected:

	egprop EGArray<egSkelPath> m_Skeletons;

protected:

	const EGSkelMeshComponent* m_SkelMeshDef = nullptr;
	EGArray<egNamedSkel>       m_CreatedSkeletons;
	EGArray<egCustomBone>      m_CustomBones;
	eg_bool                    m_bSkelsReady = false;

public:

	virtual void SetBone( eg_string_crc BoneId , const eg_transform& Transform ) override;
	virtual void ClearCustomBones() override { m_CustomBones.Clear(); }
	virtual void Reset() override;

	void SetAnimationNormalTime( eg_string_crc SkeletonId , eg_string_crc AnimationId , eg_real t );

	eg_uint GetSkelIndexByName( eg_string_crc SkelName ) const;
	eg_bool ShouldDoAnimEvent( eg_uint SkelIndex , eg_string_crc AnimId , eg_real ProgressForEvent );

	virtual eg_transform GetTextNodeBonePose( eg_uint BoneId ) const override;
	void SetupFrame( EGMesh* Mesh , EGMeshState& State ) const;
	const EGSkel* GetSkelForMeshCompatibility() const;

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual eg_transform GetJointPose( eg_string_crc JointName ) const override;
	virtual void OnDestruct() override;
	virtual void RelevantUpdate( eg_real DeltaTime ) override;
	virtual void Draw( const eg_transform& ParentPose ) const override;
	virtual void ScriptExec( const struct egTimelineAction& Action );

	virtual void OnPropChanged( const egRflEditor& ChangedProperty , eg_bool& bNeedsRebuildOut );

private:

	virtual eg_bool UpdateRenderAssetLoad() override;
	void PlayAnimation( eg_string_crc SkelName , eg_string_crc AnimationId , eg_real fSpeed , eg_real fTransitionTime );
};


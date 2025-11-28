/******************************************************************************
	EGEntObj: A class that can emulate the behavior of an entity, without it
	actually being an entity. Used for menu items and debug items.
******************************************************************************/
#pragma once
#include "EGEntDef.h"
#include "EGEntDict.h"
#include "EGRenderer.h"
#include "EGDisplayList.h"
#include "EGEntComponentTree.h"
#include "EGTimelineTypes.h"

class EGEntObj
{
public:

	EGEntObj()
	: m_EntDef(nullptr)
	, m_ComponentTree( CT_Clear )
	, m_Pose( CT_Default )
	, m_bIsLit( false )
	, m_BaseBounds( CT_Default )
	{ 

	}

	~EGEntObj()
	{
		assert( nullptr == m_EntDef ); //Should call Deinit yourself, since Init was called.
		Deinit();
	}

	const EGEntObj& operator = ( const EGEntObj& rhs )
	{
		InitClone( rhs );
		return *this;
	}

	const EGEntDef* GetDef() const { return m_EntDef; }

	eg_bool IsValid()const{ return m_EntDef != nullptr; }

	void Init( eg_string_crc EntId );
	void Init( const EGEntDef* Def );
	void InitClone( const EGEntObj& rhs );
	void Deinit();
	void SetPose( const eg_transform& Pose ) { m_Pose = Pose; }
	void SetScaleVec( const eg_vec4& ScaleVec ) { m_ComponentTree.SetScaleVec( ScaleVec ); }
	void SetScale( eg_real Scale ){ SetScaleVec( eg_vec4(Scale,Scale,Scale,1.f) ); }
	void SetIsLit( eg_bool IsLit ){ m_bIsLit = IsLit; }
	void SetDrawInfo( const eg_transform& Pose , const eg_vec4& ScaleVec , eg_bool bIsLit );
	void Update( eg_real DeltaTime );
	void RunEvent( eg_string_crc EventCrc );
	void StopAllEvents();
	void ResetEvents();
	void Draw();
	void DrawForTool();

	const eg_aabb& GetBaseBounds()const{ return m_BaseBounds; }
	eg_aabb GetMeshComponentBounds() const { return m_ComponentTree.GetMeshComponentBounds(); }
	void SetCustomBone( eg_string_crc NodeId , eg_string_crc BoneId , const eg_transform& Transform );
	void ClearCustomBones();
	void SetTexture( eg_string_crc NodeId , eg_string_crc GroupId , eg_cpstr TexturePath );
	void SetMaterial( eg_string_crc NodeId , eg_string_crc GroupId , egv_material Material );
	void SetText( eg_string_crc TextNodeCrcId , const eg_loc_text& NewText );
	void SetMuteAudio( eg_bool bMute );
	void SetPalette( const eg_vec4& Palette );
	void SetAnimationNormalTime( eg_string_crc NodeId , eg_string_crc SkeletonId , eg_string_crc AnimationId , eg_real t );

	void QueryTextNodes( EGArray<eg_d_string>& Out ) const;
	void QueryBones( EGArray<eg_d_string>& Out ) const;
	void QueryEvents( EGArray<eg_d_string>& Out ) const;

	void RefreshFromDefForTool( const class EGComponent* CompDef );

	const EGEntComponentTree& GetComponentTree() const { return m_ComponentTree; }

private:

	const EGEntDef*    m_EntDef;
	eg_aabb            m_BaseBounds;
	EGEntComponentTree m_ComponentTree;
	eg_transform       m_Pose;
	eg_bool            m_bIsLit;
};
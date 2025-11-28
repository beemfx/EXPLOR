// (c) 2017 Beem Media

#pragma once

#include "EGTimelineMgr.h"
#include "EGEntTypes.h"

class EGComponent;
class EGWorldSceneGraph;
enum egv_material : eg_uint;

class EGEntComponentTree : public IEGTimelineHandler
{
private:
	
	typedef EGArray<EGComponent*> EGCompList;

private:

	EGCompList    m_Components;
	EGTimelineMgr m_TimelineMgr;

public:

	EGEntComponentTree( const EGEntComponentTree& rhs ) = delete;
	EGEntComponentTree( eg_ctor_t Ct );

	void Init( const EGList<EGComponent>& ComponentDefTree , EGObject* OwnerObj , eg_bool bInitServer , eg_bool bInitClient , const eg_string_crc& DefaultFxFilter );
	void Deinit();

	void ActiveUpdate( eg_real DeltaTime );
	void RelevantUpdate( eg_real DeltaTime );
	void Draw( const eg_transform& WorldPose ) const;
	void DrawForTool( const eg_transform& WorldPose ) const;
	void AddToSceneGraph( const eg_transform& WorldPose , EGWorldSceneGraph* SceneGraph ) const;

	void RunTimeline( const EGTimeline* Timeline );
	void StopAllTimelines();
	void SetMuteAudio( eg_bool bMute );
	void SetScaleVec( const eg_vec4& ScaleVec );
	void SetBone( eg_string_crc NodeId , eg_string_crc BoneId , const eg_transform& Transform );
	void ClearCustomBones();
	void SetText( eg_string_crc ComponentId , const class eg_loc_text& NewText );
	void SetTexture( eg_string_crc NodeId , eg_string_crc GroupId , eg_cpstr TexturePath );
	void SetMaterial( eg_string_crc NodeId , eg_string_crc GroupId , egv_material Material );
	void SetPalette( const eg_vec4& NewPalette );
	void SetAnimationNormalTime( eg_string_crc NodeId , eg_string_crc SkeletonId , eg_string_crc AnimationId , eg_real t );
	void ResetAnimations();
	void HandleEnterWorld();
	void HandleLeaveWorld();

	void QueryTextNodes( EGArray<eg_d_string>& Out ) const;
	void QueryBones( EGArray<eg_d_string>& Out ) const;
	eg_aabb GetMeshComponentBounds() const;

	void RefreshFromDefForTool( const EGComponent* CompDef );

	EGEntComponentTree& operator = ( const EGEntComponentTree& rhs ) = delete;

	template<class T>
	T* GetComponentById( eg_string_crc Id ) const
	{
		for( EGComponent* Comp : m_Components )
		{
			if( Comp && Comp->GetId() == Id )
			{
				return EGCast<T>( Comp );
			}
		}
		return nullptr;
	}

	template<class T>
	T* FindComponentByClass() const
	{
		for( EGComponent* Comp : m_Components )
		{
			if( Comp && Comp->IsA( &T::GetStaticClass() ) )
			{
				return EGCast<T>( Comp );
			}
		}
		return nullptr;
	}

	template<class T>
	EGArray<T*> FindAllComponentsByClass() const
	{
		EGArray<T*> Out;

		for( EGComponent* Comp : m_Components )
		{
			if( Comp && Comp->IsA( &T::GetStaticClass() ) )
			{
				Out.Append( EGCast<T>( Comp ) );
			}
		}

		return std::move(Out);
	}

	template<typename T>
	void IterateComponents( T Lambda ) const
	{
		for( const EGComponent* Comp : m_Components )
		{
			Lambda( Comp );
		}
	}

	template<typename T>
	void IterateComponents( T Lambda )
	{
		for( EGComponent* Comp : m_Components )
		{
			Lambda( Comp );
		}
	}

private:

	// BEGIN IEGTimelineHandler
	virtual void OnTimelineAction( const egTimelineAction& Action ) override final;
	// END IEGTimelineHandler

	void ScriptExec( const struct egTimelineAction& Action );
	void CreateComponentRecursive( const EGComponent* CompDef , EGComponent* Parent , EGObject* OwnerObj , eg_bool bInitServer , eg_bool bInitClient , const eg_string_crc& DefaultFxFilter );
};

// (c) 2017 Beem Media

#include "EGEntComponentTree.h"
#include "EGComponent.h"
#include "EGTimeline.h"
#include "EGMeshComponent.h"
#include "EGVisualComponent.h"
#include "EGSkelMeshComponent.h"

EGEntComponentTree::EGEntComponentTree( eg_ctor_t Ct )
: m_Components()
, m_TimelineMgr( this )
{
	unused( Ct );
	assert( Ct == CT_Clear );
}

void EGEntComponentTree::Init( const EGList<EGComponent>& ComponentDefTree , EGObject* OwnerObj , eg_bool bInitServer , eg_bool bInitClient , const eg_string_crc& DefaultFxFilter )
{
	// Recursively create all components
	for( const EGComponent* CompDef : ComponentDefTree )
	{
		CreateComponentRecursive( CompDef , nullptr , OwnerObj , bInitServer , bInitClient , DefaultFxFilter );
	}

	for( EGComponent* Comp : m_Components )
	{
		Comp->OnPostInitComponents();
	}
}

void EGEntComponentTree::Deinit()
{
	for( EGComponent* Comp : m_Components )
	{
		EG_SafeRelease( Comp );
	}
	m_Components.Clear();
}

void EGEntComponentTree::ActiveUpdate( eg_real DeltaTime )
{
	for( EGComponent* Component : m_Components )
	{
		Component->ActiveUpdate( DeltaTime );
	}
}

void EGEntComponentTree::RelevantUpdate( eg_real DeltaTime )
{
	m_TimelineMgr.Update( DeltaTime );

	for( EGComponent* Component : m_Components )
	{
		Component->RelevantUpdate( DeltaTime );
	}
}

void EGEntComponentTree::Draw( const eg_transform& WorldPose ) const
{
	for( const EGComponent* Component : m_Components )
	{
		Component->Draw( Component->ComputeLocalPose() * WorldPose );
	}
}

void EGEntComponentTree::DrawForTool( const eg_transform& WorldPose ) const
{
	for( const EGComponent* Component : m_Components )
	{
		Component->DrawForTool( Component->ComputeLocalPose() * WorldPose );
	}
}

void EGEntComponentTree::AddToSceneGraph( const eg_transform& WorldPose, EGWorldSceneGraph* SceneGraph ) const
{
	for( const EGComponent* Component : m_Components )
	{
		Component->AddToSceneGraph( Component->ComputeLocalPose() * WorldPose , SceneGraph );
	}
}

void EGEntComponentTree::RunTimeline( const EGTimeline* Timeline )
{
	m_TimelineMgr.StartTimeline( Timeline );
}

void EGEntComponentTree::StopAllTimelines()
{
	m_TimelineMgr.StopAll();
}

void EGEntComponentTree::OnTimelineAction( const egTimelineAction& Action )
{
	const eg_string_crc CompId = eg_string_crc(Action.FnCall.SystemName);

	if( CompId.IsNull() )
	{
		
		ScriptExec( Action );
		return;
	}

	for( EGComponent* Comp : m_Components )
	{
		if( Comp->GetId() == CompId )
		{
			Comp->ScriptExec( Action );
			return;
		}
	}

	EGLogf( eg_log_t::Error , "Ran a script on a non existent component (%s)." , Action.FnCall.SystemName );
	// assert( false ); // No such component.
}

void EGEntComponentTree::SetMuteAudio( eg_bool bMute )
{
	for( EGComponent* Comp : m_Components )
	{
		Comp->SetMuteAudio( bMute );
	}
}

void EGEntComponentTree::SetScaleVec( const eg_vec4& ScaleVec )
{
	for( EGComponent* Comp : m_Components )
	{
		Comp->SetScaleVec( ScaleVec );
	}
}

void EGEntComponentTree::SetBone( eg_string_crc NodeId , eg_string_crc BoneId , const eg_transform& Transform )
{
	if( NodeId == CT_Clear )
	{
		if( m_Components.IsValidIndex(0) )
		{
			m_Components[0]->SetBone( BoneId , Transform );
		}
	}
	else
	{
		for( EGComponent* Comp : m_Components )
		{
			if( Comp->GetId() == NodeId )
			{
				Comp->SetBone( BoneId , Transform );
				break;
			}
		}
	}
}

void EGEntComponentTree::ClearCustomBones()
{
	for( EGComponent* Comp : m_Components )
	{
		Comp->ClearCustomBones();
	}
}

void EGEntComponentTree::SetText( eg_string_crc ComponentId , const class eg_loc_text& NewText )
{
	EGComponent* Comp = GetComponentById<EGComponent>( ComponentId );
	if( Comp )
	{
		Comp->SetText( NewText );
	}
	else
	{
		if( m_Components.Len() > 0 ) 
		{
			EGLogf( eg_log_t::Warning , "SetText: \"%s\" on a component that didn't exist." , EGString_ToMultibyte( NewText.GetString() ) );
		}
	}
}

void EGEntComponentTree::SetTexture( eg_string_crc NodeId , eg_string_crc GroupId , eg_cpstr TexturePath )
{
	if( NodeId == CT_Clear )
	{
		if( m_Components.IsValidIndex( 0 ) )
		{
			m_Components[0]->SetTexture( GroupId , TexturePath );
		}
	}
	else
	{
		for( EGComponent* Comp : m_Components )
		{
			if( Comp->GetId() == NodeId )
			{
				Comp->SetTexture( GroupId , TexturePath );
				break;
			}
		}
	}
}

void EGEntComponentTree::SetMaterial( eg_string_crc NodeId, eg_string_crc GroupId, egv_material Material )
{
	if( NodeId == CT_Clear )
	{
		if( m_Components.IsValidIndex( 0 ) )
		{
			EGVisualComponent* AsVisual = EGCast<EGVisualComponent>( m_Components[0] );
			if( AsVisual )
			{
				AsVisual->SetMaterial( GroupId , Material );
			}
		}
	}
	else
	{
		EGVisualComponent* AsVisual = GetComponentById<EGVisualComponent>( NodeId );
		if( AsVisual )
		{
			AsVisual->SetMaterial( GroupId , Material );
		}
	}
}

void EGEntComponentTree::SetPalette( const eg_vec4& NewPalette )
{
	for( EGComponent* Comp : m_Components )
	{
		Comp->SetPalette( NewPalette );
	}
}

void EGEntComponentTree::SetAnimationNormalTime( eg_string_crc NodeId , eg_string_crc SkeletonId , eg_string_crc AnimationId , eg_real t )
{
	for( EGComponent* Comp : m_Components )
	{
		if( Comp->GetId() == NodeId )
		{
			EGSkelMeshComponent* AsSkelMesh = EGCast<EGSkelMeshComponent>( Comp );
			if( AsSkelMesh )
			{
				AsSkelMesh->SetAnimationNormalTime( SkeletonId , AnimationId , t );
			}
			return;
		}
	}
}

void EGEntComponentTree::ResetAnimations()
{
	StopAllTimelines();

	for( EGComponent* Comp : m_Components )
	{
		Comp->Reset();
	}
}

void EGEntComponentTree::HandleEnterWorld()
{
	for( EGComponent* Comp : m_Components )
	{
		Comp->OnEnterWorld();
	}
}

void EGEntComponentTree::HandleLeaveWorld()
{
	for( EGComponent* Comp : m_Components )
	{
		Comp->OnLeaveWorld();
	}
}

void EGEntComponentTree::QueryTextNodes( EGArray<eg_d_string>& Out ) const
{
	for( EGComponent* Comp : m_Components )
	{
		Comp->QueryTextNodes( Out );
	}
}

void EGEntComponentTree::QueryBones( EGArray<eg_d_string>& Out ) const
{
	for( EGComponent* Comp : m_Components )
	{
		Comp->QueryBones( Out );
	}
}

eg_aabb EGEntComponentTree::GetMeshComponentBounds() const
{
	eg_bool bGotFirstBounds = false;
	eg_aabb FinalBounds( CT_Default );

	auto AddBounds = [&bGotFirstBounds,&FinalBounds]( const eg_aabb& NewBounds )
	{
		if( !bGotFirstBounds )
		{
			bGotFirstBounds = true;
			FinalBounds = NewBounds;
		}
		else
		{
			FinalBounds.AddBox( NewBounds );
		}
	};

	for( EGComponent* Comp : m_Components )
	{
		EGMeshComponent* AsMesh = EGCast<EGMeshComponent>( Comp );
		if( AsMesh )
		{
			AddBounds( AsMesh->GetMeshBounds() );
		}
	}

	return FinalBounds;
}

void EGEntComponentTree::RefreshFromDefForTool( const EGComponent* CompDef )
{
	for( EGComponent* Comp : m_Components )
	{
		if( Comp->GetDef() == CompDef )
		{
			Comp->RefreshFromDef();
		}
	}
}

void EGEntComponentTree::ScriptExec( const struct egTimelineAction& Action )
{
	const eg_string_crc ActionAsCrc = eg_string_crc(Action.FnCall.FunctionName);

	switch_crc( ActionAsCrc )
	{
		case_crc("StopAllTimelines"):
		{
			StopAllTimelines();
		} break;

		case_crc("ResetAnimations"):
		{
			ResetAnimations();
		} break;

		default:
		{
			EGLogf( eg_log_t::Error , "No component specified in timeline script (%s)." , Action.FnCall.FunctionName );
		} break;
	}
}

void EGEntComponentTree::CreateComponentRecursive( const EGComponent* CompDef , EGComponent* Parent , EGObject* OwnerObj , eg_bool bInitServer , eg_bool bInitClient  , const eg_string_crc& DefaultFxFilter )
{
	auto CreateComponent = [this,&CompDef,&Parent,&OwnerObj,&DefaultFxFilter,&bInitServer,&bInitClient]() -> EGComponent*
	{
		EGClass* ComponentClass = CompDef->GetObjectClass();
		
		if( nullptr == ComponentClass || !ComponentClass->IsA( &EGComponent::GetStaticClass() ) )
		{
			EGLogf( eg_log_t::Warning , "%s had an invalid class (%s)." , *CompDef->GetName() , ComponentClass->GetName() );
			ComponentClass = &EGComponent::GetStaticClass();
		}
		EGComponent* NewComponent = EGNewObject<EGComponent>( ComponentClass , eg_mem_pool::Entity );
		if( NewComponent )
		{
			egComponentInitData InitData;
			InitData.Def = CompDef;
			InitData.Parent = Parent;
			InitData.Owner = OwnerObj;
			InitData.OwnerCompTree = this;
			InitData.DefaultRenderFilter = DefaultFxFilter;
			InitData.bIsServer = bInitServer;
			InitData.bIsClient = bInitClient;
			NewComponent->InitComponentFromDef( InitData );
		}
		return NewComponent;
	};

	EGComponent* NewComp = CreateComponent();
	if( NewComp )
	{
		m_Components.Append( NewComp );
		for( const EGComponent* ChildCompDef : CompDef->GetChildren() )
		{
			CreateComponentRecursive( ChildCompDef , NewComp , OwnerObj , bInitServer , bInitClient , DefaultFxFilter );
		}
	}
}

// (c) 2011 Beem Software

#include "EGEnt.h"
#include "EGEntDict.h"
#include "EGGameMap.h"
#include "EGServer.h"
#include "EGClient.h"
#include "EGGame.h"
#include "EGComponent.h"
#include "EGTimeline.h"
#include "EGEntWorld.h"
#include "EGPhysBodyComponent.h"
#include "EGPhysSim2.h"

EG_CLASS_DECL( EGEnt )

EGEnt::EGEnt()
: m_Id(INVALID_ID)
, m_nCreateTime(0)
, m_EntDef( nullptr )
, m_Owner( nullptr )
, m_OwnerGame( nullptr )
, m_ComponentTree( CT_Clear )
, m_PhysBody( nullptr )
{
	// Entities are serialized, so we need to reset any data that should not be serialized
}

EGGame* EGEnt::SDK_GetGame()
{
	return m_OwnerGame.GetObject();
}

const EGGame* EGEnt::SDK_GetGame() const
{
	return m_OwnerGame.GetObject();
}

void EGEnt::Init( const eg_ent_id& EntId , EGEntWorld* OwnerWorld , const egEntCreateParms& CreateParms )
{
	m_Id = EntId;
	m_Owner = OwnerWorld;
	m_OwnerWorld = OwnerWorld;

	if( m_OwnerWorld.IsValid() )
	{
		m_OwnerGame = EGCast<EGGame>(m_OwnerWorld->GetGame());
	}

	assert( m_OwnerGame.IsValid() || (m_OwnerWorld && m_OwnerWorld->GetRole() == eg_ent_world_role::EditorPreview) ); // Ent was not owned by server or client?

	m_EntDef = EntDict_GetDef( CreateParms.EntDefId );

	assert( m_EntDef ); // Never should have been able to init without an entdef (will crash).

	m_Role = CreateParms.Role;
	m_WorldRole = m_OwnerWorld.IsValid() ? m_OwnerWorld->GetRole() : eg_ent_world_role::Unknown;
	m_bIsSerialized = m_EntDef->m_bIsSerialized;
	InitComponents();

	//So long as we have a spot to creat the new entity, let's create it.
	m_LinVel = eg_vec4(0,0,0,0);
	m_AngVel = eg_vec4(0,0,0,0);
	m_Pose = CreateParms.Pose;
	m_DefCrcId = CreateParms.EntDefId;
	ReevaluateAABB();

	if( m_WorldRole == eg_ent_world_role::Client )
	{
		m_EntDef->CreateAssets();
	}

	if( m_Role == eg_ent_role::Authority && m_OwnerWorld.IsValid() && m_OwnerWorld->GetPhysSim() )
	{
		eg_transform SpawnedPose = GetPose();
		EGPhysBodyComponent* PhysBodyComp = FindComponentByClass<EGPhysBodyComponent>();
		if( PhysBodyComp )
		{
			m_OwnerWorld->GetPhysSim()->CreatePhysBody( this , PhysBodyComp->GetPhysBodyDef() , SpawnedPose );
		}
	}

	if( eg_spawn_reason::GameLoad == CreateParms.Reason )
	{
		if( !m_bPhysBodyStateWasSaved )
		{
			m_SavedPhysBodyState = CT_Default;
		}
		SetPhysBodyState( m_SavedPhysBodyState , true );
	}

	OnCreate( CreateParms );
}

void EGEnt::Deinit()
{
	OnDestroy();

	if( m_WorldRole == eg_ent_world_role::Client && m_EntDef )
	{
		m_EntDef->DestroyAssets();
	}

	EG_SafeRelease( m_PhysBody );

	DeinitComponents();
}

void EGEnt::RefreshGame()
{
	m_OwnerGame = m_OwnerWorld.IsValid() ? EGCast<EGGame>(m_OwnerWorld->GetGame()) : nullptr;
}

void EGEnt::OnCreate( const egEntCreateParms& Parms )
{
	unused( Parms );
	assert( eg_crc("") != Parms.EntDefId ); // We don't know what the ent id is?
	RefreshFromSimulation(); // This will cause PhysBodyOnUpdateState to be called which will set the AABB and refresh the regions.
}

void EGEnt::OnEnterWorld()
{
	m_ComponentTree.HandleEnterWorld();
}

void EGEnt::OnLeaveWorld()
{
	m_ComponentTree.HandleLeaveWorld();
}

void EGEnt::PhysBodySetPhysBody( class EGPhysBody2* PhysBody )
{
	m_PhysBody = PhysBody;
}

void EGEnt::PhysBodyOnUpdateState( const egPhysBodyState& NewState )
{
	m_Pose   = NewState.Pose;
	m_LinVel = NewState.LinearVelocity;
	m_AngVel = NewState.AngularVelocity;
	m_aabb   = NewState.BoundingBox;
	assert( m_aabb.Min.w == 1.f && m_aabb.Max.w == 1.f );

	if( IsServer() && m_OwnerWorld.IsValid() )
	{
		m_OwnerWorld->Net_MarkEntDirty( m_Id );
	}
}

eg_ent_id EGEnt::PhysBodyGetEntId() const
{
	return m_Id;
}

eg_bool EGEnt::IsServer() const
{
	return m_OwnerWorld.IsValid() && m_OwnerWorld->GetRole() == eg_ent_world_role::Server;
}

eg_cpstr EGEnt::GetDefName() const
{
	return m_EntDef ? *m_EntDef->m_Id : "";
}

eg_bool EGEnt::IsClient() const
{
	return m_OwnerWorld.IsValid() && m_OwnerWorld->GetRole() == eg_ent_world_role::Client;
}

eg_bool EGEnt::IsEditorPreview() const
{
	return m_OwnerWorld.IsValid() && m_OwnerWorld->GetRole() == eg_ent_world_role::EditorPreview;
}

EGClient* EGEnt::GetOwnerClient()
{
	return m_OwnerWorld ? m_OwnerWorld->GetOwner<EGClient>() : nullptr;
}

EGServer* EGEnt::GetOwnerServer()
{
	return m_OwnerWorld.IsValid() ? EGCast<EGServer>( m_OwnerWorld->GetOwner() ) : nullptr;
}

EGEntWorld* EGEnt::GetWorld()
{
	return m_OwnerWorld.GetObject();
}

const EGEntWorld* EGEnt::GetWorld() const
{
	return m_OwnerWorld.GetObject();
}

void EGEnt::SetPose( const eg_transform& NewPose )
{
	m_Pose = NewPose;
	m_bPoseHasJumped = true;

	if( m_PhysBody ) 
	{ 
		m_PhysBody->SetPose( NewPose ); 
	}
	else
	{
		ReevaluateAABB();
	}

	MarkDirtyOnServer();
}

void EGEnt::MoveToPose( const eg_transform& NewPose )
{
	m_Pose = NewPose;

	if( m_PhysBody ) 
	{
		m_PhysBody->MoveToPose( NewPose ); 
	}
	else
	{
		ReevaluateAABB();
	}
	
	MarkDirtyOnServer();
}

void EGEnt::RunEvent( eg_string_crc Event )
{
	if( IsServer() )
	{
		if( m_OwnerWorld.IsValid() )
		{
			m_OwnerWorld->Net_RunEntEvent( GetID() , Event );
		}
	}
	else
	{
		if( m_EntDef )
		{
			for( const EGTimeline* Timeline : m_EntDef->m_Timelines )
			{
				if( Timeline && Timeline->GetId() == Event )
				{
					m_ComponentTree.RunTimeline( Timeline );
					break;
				}
			}
		}
	}
}

void EGEnt::ReplicateDataToClient( const void* p , eg_size_t Size ) const
{
	assert( IsServer() && m_bCanReplicateData );

	if( IsServer() && m_OwnerWorld.IsValid() )
	{
		eg_size_t EntObjStart = reinterpret_cast<eg_size_t>(this);
		eg_size_t ChunkDataStart = reinterpret_cast<eg_size_t>(p);

		m_OwnerWorld->Net_ReplicateEntData( GetID() , ChunkDataStart - EntObjStart , Size );
	}
}

void EGEnt::ReplicateDataToServer( const void* p , eg_size_t Size ) const
{
	assert( IsClient() && m_bCanReplicateData );

	if( IsClient() && m_OwnerWorld.IsValid() )
	{
		eg_size_t EntObjStart = reinterpret_cast<eg_size_t>(this);
		eg_size_t ChunkDataStart = reinterpret_cast<eg_size_t>(p);

		m_OwnerWorld->Net_ReplicateEntData( GetID() , ChunkDataStart - EntObjStart , Size );
	}
}

void EGEnt::SetActive( eg_bool bActive )
{
	if( m_OwnerWorld.IsValid() )
	{
		m_OwnerWorld->SetEntActive( GetID() , bActive );
	}
}

void EGEnt::SetKinematicEnabled( eg_bool bEnabled )
{
	if( m_PhysBody )
	{
		m_PhysBody->SetKinematicEnabled( bEnabled );
	}
}

void EGEnt::SetCollisionEnabled( eg_bool bEnabled )
{
	if( m_PhysBody )
	{
		m_PhysBody->SetCollisionEnabled( bEnabled );
	}
}

void EGEnt::SetRaycastHitEnabled( eg_bool bEnabled )
{
	if( m_PhysBody )
	{
		m_PhysBody->SetRaycastHitEnabled( bEnabled );
	}
}

eg_bool EGEnt::IsKinematicEnabled() const
{
	return m_PhysBody ? m_PhysBody->IsKinematicEnabled() : false;
}

eg_bool EGEnt::IsCollisionEnabled() const
{
	return m_PhysBody ? m_PhysBody->IsCollisionEnabled() : true;
}

eg_bool EGEnt::IsRaycastHitEnabled() const
{
	return m_PhysBody ? m_PhysBody->IsRaycastHitEnabled() : true;
}

void EGEnt::DeleteFromWorld()
{
	if( m_Role == eg_ent_role::Authority && m_OwnerWorld.IsValid() )
	{
		m_OwnerWorld->DestroyEnt( GetID() );
	}
}

//////////////////////
//AI compatibility ///
//////////////////////

eg_ent_id EGEnt::GetID()const
{
	return m_Id;
}

eg_d_string EGEnt::GetDefId() const
{	return m_EntDef ? m_EntDef->m_Id : "-no definition-";

}

void EGEnt::RefreshFromSimulation()
{
	if( m_PhysBody )
	{
		m_PhysBody->RefreshFromSimulation();
	}
}

eg_bool EGEnt::IsLit() const
{
	return m_EntDef->m_bIsLit;
}

eg_bool EGEnt::IsAlwaysVisible() const
{
	return m_EntDef->m_bIsAlwaysVisible;
}

egPhysBodyState EGEnt::GetPhysBodyState() const
{
	egPhysBodyState Out = CT_Default;
	if( m_PhysBody )
	{
		Out = m_PhysBody->GetState();
	}
	Out.Pose = m_Pose;
	Out.LinearVelocity = m_LinVel;
	Out.AngularVelocity = m_AngVel;
	return Out;
}

void EGEnt::SetPhysBodyState( const egPhysBodyState& State , eg_bool bIgnorePose )
{
	if( !bIgnorePose )
	{
		m_Pose = State.Pose;
	}

	if( !State.bKinematicEnabled )
	{
		m_LinVel = State.LinearVelocity;
		m_AngVel = State.AngularVelocity;
	}
	else
	{
		m_LinVel = eg_vec4( CT_Clear );
		m_AngVel = eg_vec4( CT_Clear );
	}

	if( m_PhysBody )
	{
		m_PhysBody->SetState( State , bIgnorePose );
	}
}

void EGEnt::ReevaluateAABB()
{
	assert( m_PhysBody == nullptr ); // Should only be used for entities without physics bodies

	m_aabb = m_EntDef->m_BaseBounds;
	eg_vec4 Center = eg_vec4( 0.f , 0.f , 0.f , 1.f ) * m_Pose;
	m_aabb.Min += Center;
	m_aabb.Max += Center;
	m_aabb.Min.w = 1.f;
	m_aabb.Max.w = 1.f;
}

void EGEnt::ActiveUpdate( eg_real DeltaTime )
{
	OnUpdate( DeltaTime );

	m_ComponentTree.ActiveUpdate( DeltaTime );
}

void EGEnt::RelevantUpdate( eg_real DeltaTime )
{
	m_ComponentTree.RelevantUpdate( DeltaTime );
}

void EGEnt::ClientDraw( const eg_transform& WorldPose ) const
{
	m_ComponentTree.Draw( WorldPose );
}

void EGEnt::AddToSceneGraph( EGWorldSceneGraph* SceneGraph ) const
{
	m_ComponentTree.AddToSceneGraph( m_Pose , SceneGraph );
}

void EGEnt::InitComponents()
{
	OnPreInitComponents();
	m_ComponentTree.Init( m_EntDef->m_ComponentTree , this , IsServer() , IsClient() || IsEditorPreview() , m_EntDef->m_DefaultRenderFilter );
	OnPostInitComponents();
}

void EGEnt::DeinitComponents()
{
	m_ComponentTree.Deinit();
}

egRflEditor* EGEnt::GetEditor()
{
	if( !m_Editor.IsValid() )
	{
		m_Editor = EGReflection_GetEditorForClass( GetObjectClass()->GetName() , this , "EGEnt" );
	}

	assert( m_Editor.GetData() == this );
	return &m_Editor;
}

const egRflEditor* EGEnt::GetEditor() const
{
	return const_cast<EGEnt*>(this)->GetEditor();
}

void EGEnt::MarkDirtyOnServer()
{
	if( IsServer() && m_OwnerWorld.IsValid() )
	{
		m_OwnerWorld->Net_MarkEntDirty( m_Id );
	}
}

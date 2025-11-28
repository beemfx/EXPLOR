// (c) 2018 Beem Media

#include "EGMeshShadowComponent.h"
#include "EGMesh.h"
#include "EGMeshMgr2.h"
#include "EGRenderer.h"
#include "EGMeshComponent.h"
#include "EGEnt.h"
#include "EGWorkerThreads.h"
#include "EGEngine.h"
#include "EGSkelMeshComponent.h"
#include "EGSkel.h"
#include "EGWorldSceneGraph.h"

EG_CLASS_DECL( EGMeshShadowComponent )

void EGMeshShadowComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	assert( nullptr == m_Shadow );

	m_ShadowDef = EGCast<EGMeshShadowComponent>(InitData.Def);
	m_OwnerEnt = EGCast<EGEnt>(InitData.Owner);
	m_OwnerMesh = EGCast<EGMeshComponent>(InitData.Parent);
	m_OwnerSkelMesh = EGCast<EGSkelMeshComponent>(InitData.Parent);

	if( InitData.bIsClient )
	{
		if( m_ShadowDef )
		{
			if( m_ShadowDef->m_ShadowMesh.Filename.FullPath.Len() > 0 )
			{
				m_ShadowObj = EGMeshMgr2::Get().CreateMesh( *m_ShadowDef->m_ShadowMesh.Filename.FullPath );
				m_ShadowObj->OnLoaded.Bind( this , &ThisClass::OnAssetLoaded );
				m_Shadow = m_ShadowObj->Obj;
			}
		}

		if( !m_OwnerMesh.IsValid() )
		{
			EGLogf( eg_log_t::Error , "Shadow mesh had no owner in %s" , *InitData.Def->GetName() );
		}
	}
}

void EGMeshShadowComponent::OnDestruct()
{
	EG_SafeRelease( m_ShadowObj );
	m_Shadow = nullptr;

	Super::OnDestruct();
}

void EGMeshShadowComponent::RelevantUpdate( eg_real DeltaTime )
{
	Super::RelevantUpdate( DeltaTime );

	MakeCompatible(); // Since there is no guarantee when the parent will be loaded, we'll make compatible every frame.
}

void EGMeshShadowComponent::Draw( const eg_transform& ParentPose ) const
{
	unused( ParentPose );

	// Does not get drawn in this manner, is drawn as part of scene graph.
}

void EGMeshShadowComponent::AddToSceneGraph( const eg_transform& ParentPose, EGWorldSceneGraph* SceneGraph ) const
{
	if( !m_bIsHidden && m_bHasBeenMadeCompatible && m_Shadow && m_Shadow->IsLoaded() && m_OwnerEnt )
	{
		egWorldSceneGraphShadow NewShadow;
		NewShadow.Component = this;
		NewShadow.EntOwner = EGCast<EGEnt>(m_InitData.Owner);
		NewShadow.OwnerMesh = m_OwnerMesh.GetObject();
		NewShadow.OwnerSkelMesh = m_OwnerSkelMesh.GetObject();
		NewShadow.WorldPose = ParentPose;
		SceneGraph->AddItem( NewShadow );
	}
}

eg_ent_id EGMeshShadowComponent::ForShadowDrawGetEntId() const
{
	return m_OwnerEnt ? m_OwnerEnt->GetID() : INVALID_ID;
}

void EGMeshShadowComponent::ForShadowDrawDraw( const eg_transform& BasePose ) const
{
	if( m_Shadow && m_OwnerMesh.IsValid() && m_bHasBeenMadeCompatible )
	{
		EGMeshState ShadowState = m_OwnerMesh->GetMeshState();

		MainDisplayList->SetWorldTF( eg_mat(BasePose) );
		MainDisplayList->SetVec4( eg_rv4_t::SCALE , m_OwnerMesh->GetScale() * m_OwnerMesh->GetGlobalScale() );
		if( m_OwnerSkelMesh.IsValid() )
		{
			m_OwnerSkelMesh->SetupFrame( m_Shadow , ShadowState );
		}
		m_Shadow->DrawRaw( ShadowState );
	}
}

void EGMeshShadowComponent::OnAssetLoaded( EGMeshMgrObj* Obj )
{
	unused( Obj );

	MakeCompatible();	
}

void EGMeshShadowComponent::MakeCompatible()
{
	if( !m_bHasBeenMadeCompatible && m_Shadow && m_Shadow->IsLoaded() )
	{
		if( m_OwnerSkelMesh && m_Shadow->GetNumBones() > 0 )
		{
			const EGSkel* SkelForMeshCompatibility = m_OwnerSkelMesh->GetSkelForMeshCompatibility();
			if( SkelForMeshCompatibility && SkelForMeshCompatibility->IsLoaded() )
			{
				m_Shadow->MakeCompatibleWith( SkelForMeshCompatibility );
				m_bHasBeenMadeCompatible = true;
			}
		}
		else
		{
			m_bHasBeenMadeCompatible = true;
		}
	}
}

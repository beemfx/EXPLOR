// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMapNavBase.h"
#include "ExGame.h"
#include "EGSettings2Types.h"
#include "EGMeshComponent.h"

EG_CLASS_DECL( ExMapNavBase )

static EGSettingsBool ExMapNavBase_ShowDebugHitBoxes( "ExEntity.ShowDebugHitBoxes" , eg_loc("ShowEntityHintBoxesText","DEBUG: Show Entity Hit Boxes") , false , EGS_F_DEBUG_EDITABLE|EGS_F_USER_SAVED );


void ExMapNavBase::RestablishNavGraph( const eg_transform& InPose )
{
	m_NavGraph = GetGameData()->SDK_GetNavGraph( eg_nav_graph_id( eg_crc( "nav" ) ) );
	TeleporTo( InPose );
}

void ExMapNavBase::OnCreate( const egEntCreateParms& Parms )
{
	Super::OnCreate( Parms );
}

void ExMapNavBase::Initialize()
{
	SetKinematicEnabled( true );
	SetCollisionEnabled( false );

	m_RepDataChecksum = eg_crc( "" );
	m_MoveState = ex_move_s::NONE;
	m_NavGraph = GetGameData()->SDK_GetNavGraph( eg_nav_graph_id( eg_crc( "nav" ) ) );
	// assert( m_NavGraph.IsValid() ); // Need a nav graph to play this game.
	RunEvent( eg_crc( "Init" ) );
	// We always want to make sure we on on a nav path.
	if( m_NavGraph.IsValid() )
	{
		eg_transform Pose = GetPose();
		TeleporTo( Pose );
	}
}

void ExMapNavBase::UpdateDebugHitBoxVisibility()
{
	EGMeshComponent* DebugHitBox = GetComponentById<EGMeshComponent>( eg_crc("DebugHitBoxMesh") );
	if( DebugHitBox )
	{
		DebugHitBox->SetHidden( !(EX_CHEATS_ENABLED && ExMapNavBase_ShowDebugHitBoxes.GetValue()) );
	}
}

void ExMapNavBase::TeleporTo( const eg_transform& NewPose )
{
	if( m_NavGraph.IsValid() )
	{
		assert( m_MoveState != ex_move_s::TURNING );
		eg_uint ClosestVertexIdx = m_NavGraph.GetVertexIndexClosestTo( GetGameData() , NewPose.GetPosition() );
		eg_bool bGotVertex = m_NavGraph.GetVertex( GetGameData() , ClosestVertexIdx , &m_CurentNavVertex );
		assert( bGotVertex );
		if( bGotVertex )
		{
			eg_transform FinalPos;
			FinalPos.SetRotation( NewPose.GetRotation() );
			FinalPos.SetTranslation( m_CurentNavVertex.GetPosition().ToVec3() );
			FinalPos.SetScale( 1.f );
			SetPose( FinalPos );
			m_MoveState = ex_move_s::NONE;
		}
	}
	else
	{
		SetPose( NewPose );
	}
}

void ExMapNavBase::ReplicateChangedData( const void* Data , eg_size_t DataSize )
{
	eg_string_crc NewDataChecksum = eg_string_crc::HashData( Data , DataSize );

	if( NewDataChecksum != m_RepDataChecksum )
	{
		ReplicateDataToClient( Data , DataSize );
		m_RepDataChecksum = NewDataChecksum;
	}
}

eg_ent_id ExMapNavBase::GetTargetedEnt( eg_vec4* HitPosOut , eg_real* DistToEntSqOut ) const
{
	eg_vec4 FaceDir = eg_vec4(0.f,0.f,1.f,0.f) * GetPose();
	FaceDir.NormalizeThisAsVec3();
	egRaycastInfo RayCastInfo = SDK_GetGame()->SDK_RayCastFromEnt( GetID(), FaceDir );
	if( HitPosOut )
	{
		if( RayCastInfo.nEntID != INVALID_ID )
		{
			*HitPosOut = RayCastInfo.vHit;
		}
		else
		{
			*HitPosOut = eg_vec4(0.f,0.f,0.f,1.f);
		}
	}

	if( DistToEntSqOut )
	{
		if( RayCastInfo.nEntID != INVALID_ID )
		{
			*DistToEntSqOut = (RayCastInfo.vHit - RayCastInfo.vFrom).LenSqAsVec3();
		}
		else
		{
			*DistToEntSqOut = 0.f;
		}
	}

	return RayCastInfo.nEntID;
}

EGNavGraphVertex ExMapNavBase::GetTargetedVertex( eg_real* DistToVertexSqOut ) const
{
	EGNavGraphVertex Out( CT_Clear );

	eg_vec4 FaceDir = eg_vec4(0.f,0.f,1.f,0.f) * GetPose();
	eg_bool bFoundNav = false;
	for( eg_uint i = 0; !bFoundNav && i < m_CurentNavVertex.GetNumEdges(); i++ )
	{
		// If the angle between the face direction and the edge in question
		// is zero then we found the vertex we want to go to.
		EGNavGraphVertex Edge = m_CurentNavVertex.GetEdge( GetGameData(), i );
		eg_vec4 ToEdge = Edge.GetPosition() - m_CurentNavVertex.GetPosition();
		ToEdge.y = 0.f;
		eg_real DistSq = ToEdge.LenSqAsVec3();
		ToEdge.NormalizeThisAsVec3();
		eg_real Dotp = ToEdge.Dot( FaceDir );
		const eg_real Eps = .01f;
		if( EG_IsEqualEps( Dotp, 1.f , Eps ) )
		{
			if( DistToVertexSqOut )
			{
				*DistToVertexSqOut = DistSq;
			}
			Out = m_CurentNavVertex.GetEdge( GetGameData(), i );
			bFoundNav = true;
		}
	}

	return Out;
}

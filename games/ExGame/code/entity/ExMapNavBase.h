// (c) 2016 Beem Media

#pragma once

#include "ExEnt.h"
#include "EGNavGraph.h"
#include "ExMapNavBase.reflection.h"

enum class ex_move_s
{
	NONE,
	WALKING,
	TURNING,
	WAIT_TURN,
	WAITING_ON_DOOR,
	WAITING_ON_LOCKED_DOOR,
};

egreflect class ExMapNavBase : public egprop ExEnt
{
	EG_CLASS_BODY( ExMapNavBase , ExEnt )
	EG_FRIEND_RFL( ExMapNavBase )

friend class ExMapObjectComponent;

protected:

	ex_move_s        m_MoveState;
	EGNavGraph       m_NavGraph = CT_Preserve;
	eg_string_crc    m_RepDataChecksum;
	EGNavGraphVertex m_CurentNavVertex = CT_Preserve;

public:

	EGNavGraphVertex GetMapVertex()const{ return m_CurentNavVertex; }

	eg_bool IsInTurn()const { return m_MoveState == ex_move_s::WALKING || m_MoveState  == ex_move_s::WAIT_TURN; }
	eg_bool IsIdle() const { return m_MoveState == ex_move_s::NONE; }
	void RestablishNavGraph( const eg_transform& InPose );

protected:

	virtual void OnCreate( const egEntCreateParms& Parms ) override;

	void Initialize();
	void UpdateDebugHitBoxVisibility();

	virtual void TeleporTo( const eg_transform& NewPose );

	void ReplicateChangedData( const void* Data , eg_size_t DataSize );

	eg_ent_id GetTargetedEnt( eg_vec4* HitPosOut , eg_real* DistToEntSqOut ) const;
	EGNavGraphVertex GetTargetedVertex( eg_real* DistToVertexSqOut ) const;
};
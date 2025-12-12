// (c) 2020 Beem Media. All Rights Reserved.

#include "ExCameraSplineNodeEnt.h"
#include "ExGame.h"

EG_CLASS_DECL( ExCameraSplineNodeEnt )

void ExCameraSplineNodeEnt::OnCreate( const egEntCreateParms& Parms )
{
	Super::OnCreate( Parms );

	if( Parms.Reason == eg_spawn_reason::Spawned || Parms.Reason == eg_spawn_reason::Replicated )
	{
		m_SplineId = CT_Clear;
		m_SplineNode = -1;
	}

	if( GetRole() == eg_ent_role::Authority )
	{		
		if( Parms.Reason == eg_spawn_reason::Spawned )
		{
			ParseInitString( Parms.InitString );
		}
	}
}

void ExCameraSplineNodeEnt::OnDataReplicated( const void* Offset , eg_size_t Size )
{
	Super::OnDataReplicated( Offset , Size );

	if( IsClient() )
	{
		if( Offset == &m_SplineNode && Size == sizeof(m_SplineNode) )
		{
			// m_SplineNode is replicated.
		}
	}
}

void ExCameraSplineNodeEnt::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( ExGame* Game = GetGameData() )
	{
		if( IsAuthority() )
		{
			Game->SetCameraSplineNode( this , true );
		}
	}
}

void ExCameraSplineNodeEnt::OnLeaveWorld()
{
	if( ExGame* Game = GetGameData() )
	{
		if( IsAuthority() )
		{
			Game->SetCameraSplineNode( this , false );
		}
	}

	Super::OnLeaveWorld();
}

void ExCameraSplineNodeEnt::ParseInitString( const eg_d_string& InitString )
{
	EGParse_ProcessFnCallScript( *InitString , InitString.Len() , [this]( const egParseFuncInfo& InfoStr )->void
	{
		if( EGString_EqualsI(InfoStr.FunctionName,"SetSplineId") && InfoStr.NumParms >= 1 )
		{
			m_SplineId = EGCrcDb::StringToCrc( InfoStr.Parms[0] );
		}
		else if( EGString_EqualsI(InfoStr.FunctionName,"SetSplineIndex") && InfoStr.NumParms >= 1 )
		{
			m_SplineNode = EGString_ToInt( InfoStr.Parms[0] );
		}
	} );
}

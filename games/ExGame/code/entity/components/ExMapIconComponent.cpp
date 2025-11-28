// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMapIconComponent.h"
#include "ExGame.h"
#include "EGEnt.h"
#include "EGRenderer.h"

EG_CLASS_DECL( ExMapIconComponent )

void ExMapIconComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );
	const ExMapIconComponent* DefAsNpc = EGCast<ExMapIconComponent>(InitData.Def);
	m_MapIcon = DefAsNpc->m_MapIcon;
	m_bClampIconToMinimapEdge = DefAsNpc->m_bClampIconToMinimapEdge;
}

void ExMapIconComponent::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( IsClient() && m_MapIcon.FullPath.Len() > 0 )
	{
		EGMaterialDef MatDef;
		EGString_Copy( MatDef.m_strTex[0] , *m_MapIcon.FullPath , EG_MAX_PATH );
		EGString_Copy( MatDef.m_strVS , "/game/shaders/ex_ui" , EG_MAX_PATH );
		EGString_Copy( MatDef.m_strPS , "/game/shaders/ex_ui" , EG_MAX_PATH );

		eg_string SharedId = EGString_Format( "MapIconTx(%s)" , *m_MapIcon.FullPath );

		m_MapIconMaterial = EGRenderer::Get().CreateMaterial( &MatDef , SharedId.String() );
	}

	if( EGEnt* OwnerAsEnt = GetOwner<EGEnt>() )
	{
		if( ExGame* Game = EGCast<ExGame>( OwnerAsEnt->SDK_GetGame() ) )
		{
			Game->SetTrackedMapIcon( this , true );
		}
	}
}

void ExMapIconComponent::OnLeaveWorld()
{
	if( IsClient() )
	{
		if( m_MapIconMaterial != EGV_MATERIAL_NULL )
		{
			EGRenderer::Get().DestroyMaterial( m_MapIconMaterial );
			m_MapIconMaterial = EGV_MATERIAL_NULL;
		}
	}

	if( EGEnt* OwnerAsEnt = GetOwner<EGEnt>() )
	{
		if( ExGame* Game = EGCast<ExGame>( OwnerAsEnt->SDK_GetGame() ) )
		{
			Game->SetTrackedMapIcon( this , false );
		}
	}

	Super::OnLeaveWorld();
}

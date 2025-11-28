// (c) 2018 Beem Media

#include "EGSkyboxMeshComponent.h"
#include "EGRenderer.h"
#include "EGEnt.h"

EG_CLASS_DECL( EGSkyboxMeshComponent );

void EGSkyboxMeshComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	EGEnt* OwnerEnt = EGCast<EGEnt>(InitData.Owner);
	if( OwnerEnt && !OwnerEnt->IsAlwaysVisible() )
	{
		EGLogf( eg_log_t::Warning , "%s has a Skybox Component that is not always visible." , OwnerEnt->GetDefName() );
	}

	// EGLogf( eg_log_t::General , "Spawned a skybox component." );
}

void EGSkyboxMeshComponent::Draw( const eg_transform& ParentPose ) const
{
	unused( ParentPose );

	if( m_bReady && m_CreatedMesh && !m_bIsHidden )
	{
		eg_transform FullPose = m_Pose.GetCurrentValue();
		FullPose *= ParentPose;
		FullPose.SetTranslation( eg_vec3(0.f,0.f,0.f) );

		MainDisplayList->SetWorldTF( eg_mat(FullPose) );
		MainDisplayList->SetFloat( F_TIME, m_Life );
		MainDisplayList->SetVec4( eg_rv4_t::SCALE, m_CachedScale.GetCurrentValue() * m_GlobalScale );
		MainDisplayList->SetVec4( eg_rv4_t::ENT_PALETTE_0, m_Palette.GetCurrentValue() );

		MainDisplayList->PushDefaultShader( eg_defaultshader_t::MESH_UNLIT );
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF );
		MainDisplayList->PushRasterizerState( eg_rasterizer_s::CULL_NONE );
		m_CreatedMesh->Draw( m_MeshState );
		MainDisplayList->PopRasterizerState();
		MainDisplayList->PopDefaultShader();
		MainDisplayList->PopDepthStencilState();
	}
}

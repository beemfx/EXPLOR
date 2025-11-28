// (c) 2018 Beem Media

#include "EGVisualComponent.h"
#include "EGWorldSceneGraph.h"
#include "EGEnt.h"

EG_CLASS_DECL( EGVisualComponent )

void EGVisualComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	ThisClass::RefreshFromDef();

	if( m_RenderFilter.IsNull() )
	{
		m_RenderFilter = InitData.DefaultRenderFilter;
	}
}

void EGVisualComponent::RefreshFromDef()
{
	Super::RefreshFromDef();

	const EGVisualComponent* DefAsVisual = EGCast<EGVisualComponent>(m_InitData.Def);
	m_bIsHidden = DefAsVisual->m_bIsHidden;
	m_bIsReflective =  DefAsVisual->m_bIsReflective;
	m_RenderFilter = DefAsVisual->m_RenderFilter;
	m_bIsTransparent = DefAsVisual->m_bIsTransparent;

	m_Palette.SetValue( eg_vec4(1.f,1.f,1.f,1.f) );
}

void EGVisualComponent::SetPalette( const eg_vec4& NewPalette )
{
	m_Palette.SetValue( NewPalette );
}

void EGVisualComponent::Reset()
{
	const EGVisualComponent* DefAsVisual = EGCast<EGVisualComponent>(m_InitData.Def);
	m_Palette.SetValue( eg_vec4(1.f,1.f,1.f,1.f) );
	m_Pose.SetValue( DefAsVisual->m_BasePose );
	m_bIsHidden = DefAsVisual->m_bIsHidden;
}

void EGVisualComponent::RelevantUpdate( eg_real DeltaTime )
{
	Super::RelevantUpdate( DeltaTime );

	m_Palette.Update( DeltaTime );
}

void EGVisualComponent::ScriptExec( const struct egTimelineAction& Action )
{
	const eg_string_crc ActionAsCrc = eg_string_crc(Action.FnCall.FunctionName);

	switch_crc( ActionAsCrc )
	{
		case_crc("SetPal"):
		case_crc("SetPalette"):
		{
			if( Action.FnCall.NumParms >= 4 )
			{
				m_Palette.SetValue( eg_vec4( Action.RealParm(0) , Action.RealParm(1) , Action.RealParm(2) , Action.RealParm(3) ) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for SetPalette." );
			}
		} break;

		case_crc("SetPalAlpha"):
		case_crc("SetPaletteAlpha"):
		{
			if( Action.FnCall.NumParms >= 1 )
			{
				eg_vec4 NewValue = m_Palette.GetEndValue();
				NewValue.w = Action.RealParm(0);
				m_Palette.SetValue( NewValue );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for SetPaletteAlpha." );
			}
		} break;

		case_crc("TweenPalAlphaTo"):
		case_crc("TweenPaletteAlphaTo"):
		{
			if( Action.FnCall.NumParms >= 3 )
			{
				eg_vec4 FinalValue = m_Palette.GetEndValue();
				FinalValue.w = Action.RealParm(0);
				m_Palette.MoveTo( FinalValue , Action.RealParm(1) , Action.TweenTypeParm(2) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for TweenPaletteAlphaTo." );
			}
		} break;

		case_crc("TweenPalTo"):
		case_crc("TweenPaletteTo"):
		{
			if( Action.FnCall.NumParms >= 6 )
			{
				const eg_vec4 NewValue = eg_vec4( Action.RealParm(0) , Action.RealParm(1) , Action.RealParm(2) , Action.RealParm(3) );
				m_Palette.MoveTo( NewValue , Action.RealParm(4) , Action.TweenTypeParm(5) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for TweenPaletteTo." );
			}
		} break;

		case_crc("SetVisible"):
		{
			if( Action.FnCall.NumParms >= 1 )
			{
				m_bIsHidden = !Action.BoolParm(0);
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for SetVisible." );
			}
		} break;

		case_crc("SetPos"):
		case_crc("SetPosition"):
		{
			if( Action.FnCall.NumParms >= 3 )
			{
				eg_transform FinalPose = m_Pose.GetEndValue();
				FinalPose.SetTranslation( eg_vec3(Action.RealParm(0),Action.RealParm(1),Action.RealParm(2)) );
				m_Pose.SetValue( FinalPose );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for SetPosition." );
			}
		} break;

		case_crc("TweenPosTo"):
		case_crc("TweenPositionTo"):
		{
			if( Action.FnCall.NumParms >= 5 )
			{
				eg_transform FinalPose = m_Pose.GetEndValue();
				FinalPose.SetTranslation( eg_vec3(Action.RealParm(0),Action.RealParm(1),Action.RealParm(2)) );
				m_Pose.MoveTo( FinalPose , Action.RealParm(3) , Action.TweenTypeParm(4) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for TweenPositionTo." );
			}
		} break;

		case_crc("SetRot"):
		case_crc("SetRotation"):
		{
			if( Action.FnCall.NumParms >= 3 )
			{
				eg_transform FinalPose = m_Pose.GetEndValue();
				eg_quat RotX = eg_quat::BuildRotationX( EG_Deg(Action.RealParm(0)) );
				eg_quat RotY = eg_quat::BuildRotationY( EG_Deg(Action.RealParm(1)) );
				eg_quat RotZ = eg_quat::BuildRotationZ( EG_Deg(Action.RealParm(2)) );
				eg_quat Rotation = RotX * RotY * RotZ;
				FinalPose.SetRotation( Rotation );
				m_Pose.SetValue( FinalPose );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for SetRotation." );
			}
		} break;

		case_crc("TweenRotTo"):
		case_crc("TweenRotationTo"):
		{
			if( Action.FnCall.NumParms >= 5 )
			{
				eg_transform FinalPose = m_Pose.GetEndValue();
				eg_quat RotX = eg_quat::BuildRotationX( EG_Deg(Action.RealParm(0)) );
				eg_quat RotY = eg_quat::BuildRotationY( EG_Deg(Action.RealParm(1)) );
				eg_quat RotZ = eg_quat::BuildRotationZ( EG_Deg(Action.RealParm(2)) );
				eg_quat Rotation = RotX * RotY * RotZ;
				FinalPose.SetRotation( Rotation );
				m_Pose.MoveTo( FinalPose , Action.RealParm(3) , Action.TweenTypeParm(4) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for TweenRotationTo." );
			}
		} break;
		case_crc("SetScale"):
		{
			if( Action.FnCall.NumParms >= 3 )
			{
				m_CachedScale.SetValue( eg_vec4(Action.RealParm(0),Action.RealParm(1),Action.RealParm(2),1.f) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for SetScale." );
			}
		} break;
		case_crc("TweenScaleTo"):
		{
			if( Action.FnCall.NumParms >= 5 )
			{
				m_CachedScale.MoveTo( eg_vec4(Action.RealParm(0),Action.RealParm(1),Action.RealParm(2),1.f) , Action.RealParm(3) , Action.TweenTypeParm(4) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for TweenScaleTo." );
			}
		} break;

		default:
		{
			Super::ScriptExec( Action );
		} break;
	}
}

void EGVisualComponent::AddToSceneGraph( const eg_transform& ParentPose, EGWorldSceneGraph* SceneGraph ) const
{
	if( !m_bIsHidden )
	{
		egWorldSceneGraphComponent NewComp;
		NewComp.Component = this;
		NewComp.EntOwner = EGCast<EGEnt>(m_InitData.Owner);
		NewComp.RenderFilter = m_RenderFilter;
		NewComp.WorldPose = ParentPose;
		NewComp.bIsReflective = m_bIsReflective;
		SceneGraph->AddItem( NewComp );
	}
}

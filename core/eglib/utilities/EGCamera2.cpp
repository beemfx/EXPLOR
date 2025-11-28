// EGCamera2 - Camera Class
// (c) 2016 Beem Media

#include "EGCamera2.h"

void EGCamera2::InitPerspective( eg_real AspectRatio, eg_real FovDeg, eg_real Near, eg_real Far )
{
	m_Type = eg_camera_t::PERSP;
	m_FovDeg = FovDeg;
	m_Near = Near;
	m_Far = Far;
	m_AspectRatio = AspectRatio;
	m_OrthoHeight = 0.f;
	m_Pose = eg_transform::BuildIdentity();
	m_ViewDirty = true;
	m_ProjDirty = true;
	m_ViewTransDirty = true;
}

void EGCamera2::InitOrthographic( eg_real AspectRatio , eg_real OrthoHeight, eg_real Near, eg_real Far )
{
	m_Type = eg_camera_t::ORTHO;
	m_FovDeg = 0.f;
	m_Near = Near;
	m_Far = Far;
	m_AspectRatio = AspectRatio;
	m_OrthoHeight = OrthoHeight;
	m_Pose = eg_transform::BuildIdentity();
	m_ViewDirty = true;
	m_ProjDirty = true;
	m_ViewTransDirty = true;
}

const eg_mat& EGCamera2::GetViewMat() const
{
	if( m_ViewDirty )
	{
		m_ViewDirty = false;

		#if 1
		m_CachedView = eg_mat::BuildORTInverse( eg_mat(m_Pose) );
		#else
		eg_vec4 Pos = eg_vec4(0.f,0.f,0.f,1.f) * m_Pose;
		eg_vec4 At  = eg_vec4(0.f,0.f,1.f,1.f) * m_Pose;
		eg_vec4 Up  = eg_vec4(0.f,1.f,0.f,0.f) * m_Pose;
		m_CachedView.MakeLookAtLH( Pos , At , Up );
		#endif
	}
	return m_CachedView;
}

const eg_transform& EGCamera2::GetViewTransform() const
{
	if( m_ViewTransDirty )
	{
		m_CachedViewTrans = m_Pose.GetInverse();
		m_ViewTransDirty = false;
	}

	return m_CachedViewTrans;
}

const eg_mat& EGCamera2::GetProjMat() const
{
	if( m_ProjDirty )
	{
		m_ProjDirty = false;

		switch( m_Type )
		{
			case eg_camera_t::ORTHO:
			{
				m_CachedProj = eg_mat::BuildOrthographicLH( m_OrthoHeight*m_AspectRatio , m_OrthoHeight , m_Near , m_Far );
			} break;
			case eg_camera_t::PERSP:
			{
				m_CachedProj = eg_mat::BuildPerspectiveFovLH( EG_Deg( m_FovDeg*.5f ) , m_AspectRatio , m_Near , m_Far );
			} break;
		}
	}
	return m_CachedProj;
}

eg_vec2 EGCamera2::WorldSpaceToScreenSpace( const eg_vec4& Pos, const eg_mat& View, const eg_mat& Proj )
{
	eg_vec4 ProjSpace = Pos * View * Proj;
	return eg_vec2( ProjSpace.x/ProjSpace.w , ProjSpace.y/ProjSpace.w );
}

eg_vec2 EGCamera2::WorldSpaceToMouseSpace( const eg_vec4& Pos, eg_real AspectRatio, const eg_mat& View, const eg_mat& Proj )
{
	eg_vec2 ScreenSpace = WorldSpaceToScreenSpace( Pos , View , Proj );

	return eg_vec2( ScreenSpace.x*AspectRatio , ScreenSpace.y );
}


// EGCamera2 - Camera Class
// (c) 2016 Beem Media

#pragma once

class EGCamera2
{
private:
	
	eg_transform m_Pose;
	eg_camera_t  m_Type;
	eg_real      m_FovDeg;
	eg_real      m_AspectRatio;
	eg_real      m_OrthoHeight;
	eg_real      m_Near;
	eg_real      m_Far;

	// Cached properties:
	mutable eg_mat       m_CachedView;
	mutable eg_mat       m_CachedProj;
	mutable eg_transform m_CachedViewTrans;
	mutable eg_bool      m_ViewDirty:1;
	mutable eg_bool      m_ProjDirty:1;
	mutable eg_bool      m_ViewTransDirty:1;

public:
	
	EGCamera2( eg_ctor_t Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			InitPerspective( 1.f , 90.f , .1f , 1000.f );
		}
	}

	void InitPerspective( eg_real AspectRatio , eg_real FovDeg , eg_real Near , eg_real Far );
	void InitOrthographic( eg_real AspectRatio , eg_real OrthoHeight , eg_real Near , eg_real Far ); 

	const eg_mat& GetViewMat() const;
	const eg_transform& GetViewTransform() const;
	const eg_mat& GetProjMat() const;
	const eg_transform& GetPose() const{ return m_Pose; }

	void SetPose( const eg_transform& Pose ){ m_Pose = Pose; m_Pose.NormalizeRotationOfThis(); m_ViewDirty = true; m_ViewTransDirty = true; }
	void SetAspectRatio( eg_real AspectRatio ){ m_AspectRatio = AspectRatio; m_ProjDirty = true; }

	eg_real GetFovDeg() const { return m_FovDeg; }
	eg_real GetAspectRatio() const{ return m_AspectRatio; }
	eg_real GetNear() const { return m_Near; }
	eg_real GetFar() const { return m_Far; }

	static eg_vec2 WorldSpaceToScreenSpace( const eg_vec4& Pos , const eg_mat& View , const eg_mat& Proj );
	static eg_vec2 WorldSpaceToMouseSpace( const eg_vec4& Pos , eg_real AspectRatio, const eg_mat& View , const eg_mat& Proj );
};
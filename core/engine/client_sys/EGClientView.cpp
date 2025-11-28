// (c) 2011 Beem Software

#include "EGClientView.h"
#include "EGRenderer.h"

EGClientView::EGClientView()
: m_Camera( CT_Default )
{
	m_Vp.Left   = -1.0f;
	m_Vp.Right  = 1.0f;
	m_Vp.Bottom = -1.0f;
	m_Vp.Top    = 1.0f;
	m_Vp.Near   = 0.0f;
	m_Vp.Far    = 1.0f;
	m_GameAspectRatio = 1.f;
}

void EGClientView::SetPose( const eg_transform& Pose, eg_real FovDeg, eg_real Near, eg_real Far )
{
	m_Camera.InitPerspective( m_Vp.GetAspectRatio() , FovDeg , Near , Far );
	m_Camera.SetPose( Pose );
}

const EGViewFr& EGClientView::GetViewFr()const
{
	return m_ViewFr;
}

void EGClientView::SetRenderTransforms()
{
	SetRenderTransforms( MainDisplayList , m_Camera );
}

void EGClientView::SetRenderTransforms( EGDisplayList* DisplayList, const EGCamera2& Camera )
{
	DisplayList->SetViewTF( Camera.GetViewMat() );
	DisplayList->SetProjTF( Camera.GetProjMat() );
	DisplayList->SetVec4( eg_rv4_t::CAMERA_POS , Camera.GetPose().GetPosition() );
}

void EGClientView::SetReflectiveRenderTransforms( EGDisplayList* DisplayList , const EGCamera2& Camera , const eg_vec4& pvPoint , const eg_vec4& pvNorm )
{
	assert(pvPoint.w == 1 && pvNorm.w == 0);

	const eg_plane pln(pvNorm.x, pvNorm.y, pvNorm.z, -pvPoint.Dot(pvNorm));

	eg_mat m1;
	m1 = eg_mat::BuildReflection(pln);
	eg_mat mV = m1 * Camera.GetViewMat();
	eg_mat mC = mV * Camera.GetProjMat();

	DisplayList->SetViewTF( mV );
	DisplayList->SetProjTF( Camera.GetProjMat() );
	DisplayList->SetVec4( eg_rv4_t::CAMERA_POS , Camera.GetPose().GetPosition() );
	DisplayList->SetClipPlane( pln );
}

void EGClientView::PreSceneSetup( eg_bool SetupGraphicsCam /*= true */ )
{
	//This method basically sets up all the matrices for the scene
	//depending on how the scene is to be rendered.
	m_Camera.SetAspectRatio( m_Vp.GetAspectRatio() );
	m_ViewFr.Initialize( m_Camera.GetFovDeg() , m_Camera.GetAspectRatio() , m_Camera.GetNear() , m_Camera.GetFar() );

	if( SetupGraphicsCam )
	{
		SetRenderTransforms();
	}
}

void EGClientView::WorldToScreen(eg_vec4* pOut, const eg_vec4* pIn)
{
	eg_mat mCamera = m_Camera.GetViewMat() * m_Camera.GetProjMat();
	eg_vec4 v4;
	v4 = *pIn * mCamera;
	v4 /= v4.w;
	*pOut = v4;
}

void EGClientView::WorldToView(eg_vec4* av3Out, const eg_vec4* av3In, eg_uint n)
{
	EGMatrix_TransformArray(av3Out, sizeof(eg_vec4), av3In, sizeof(eg_vec4), m_Camera.GetViewMat(), n);
	for(eg_uint i=0; i<n; i++)
	{
		av3Out[i] /= av3Out[i].w;
	}
}

void EGClientView::GetInvView(eg_mat* pInvView)const
{
	*pInvView = eg_mat::BuildInverse( m_Camera.GetViewMat() , nullptr );
}

void EGClientView::GetCameraTraceVecs(eg_real fX, eg_real fY, eg_vec4* pOrg, eg_vec4* pDir)
{
	//Here is some code to find out where the mouse is pointing:
	//See DirectX examples for the inspiration.
	//Because the projection matrix contains the aspect ratio, we divide that out
	//of fX
	fX /= m_ViewFr.GetAspect();

	eg_vec4 v(fX/m_Camera.GetProjMat().r11, fY/m_Camera.GetProjMat().r22, 1.0f, 0);
	eg_mat m;
	m = eg_mat::BuildInverse( m_Camera.GetViewMat() , nullptr );
	pOrg->x=m.tx;
	pOrg->y=m.ty;
	pOrg->z=m.tz;
	pOrg->w = 1;
	*pDir = v * m;
	assert(0 == pDir->w);
	pDir->NormalizeThisAsVec3();
}

void EGClientView::Update()
{	
	eg_vec4 Pos = m_Camera.GetPose().GetPosition();

	//Check the regions we are in.
	//We'll give the camera a volume of (1/2 meter)^3
	eg_aabb aabb;
	aabb.Min.x = Pos.x - 0.25f;
	aabb.Min.y = Pos.y - 0.25f;
	aabb.Min.z = Pos.z - 0.25f;

	aabb.Max.x = Pos.x + 0.25f;
	aabb.Max.y = Pos.y + 0.25f;
	aabb.Max.z = Pos.z + 0.25f;

	aabb.Min.w = aabb.Max.w = 1;

	PreSceneSetup( false );
}
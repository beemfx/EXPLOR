#include "EGViewFr.h"

EGViewFr::EGViewFr()
{
	//PreSceneSetup();
	Initialize( 90.0f , 1.0f , 0.5f , 1000.0f );
}

EGViewFr::EGViewFr(const EGViewFr& rhs)
{
	*this = rhs;
}


EGViewFr::~EGViewFr()
{
	//Nothing to destroy in this class.
}

void EGViewFr::Initialize(	eg_real fFOVDeg, eg_real fAspect, eg_real fNear, eg_real fFar)
{	
	unused( fNear );

	m_fFOV = EG_Deg(fFOVDeg).ToRad();
	eg_real fFOVRad = m_fFOV*0.5f;

	//We need to create the projection matrix and setup the frustrum planes:
	m_fAspect = fAspect;
	m_rcVis.left   = -1.0f;
	m_rcVis.right  = 1.0f;
	m_rcVis.top    = 1.0f/fAspect;
	m_rcVis.bottom = -1.0f/fAspect;

	//The frustrum planes will ignor the near plane, and just uses the origin.
	m_plnN.a = 0;
	m_plnN.b = 0;
	m_plnN.c = 1.0f;
	m_plnN.d = 0;

	//The far plane uses the actual far plane:
	m_plnF.a = 0;
	m_plnF.b = 0;
	m_plnF.c = -1.0f;
	m_plnF.d = fFar;

	//#define BY_ENGEL
	#ifdef BY_ENGEL
		
	eg_real fE = 1/tan(fAspect*0.5f);

	eg_real fA = 1/sqrt(fE*fE + 1);
	eg_real fB = 1/sqrt(fE*fE + 1/(fAspect*fAspect));


	//From Engel pg 93:
	//Some basic trig will get us the plane equation we want, not that these
	//planes pass through the origin, so our frustrum is basically a full pyramid.
	m_plnL.a = fE*fA;
	m_plnL.b = 0;
	m_plnL.c = fA;
	m_plnL.d = 0;
	

	//The right plane is exactly the opposite of the left plane:
	m_plnR.a = -m_plnL.a;
	m_plnR.b = 0;
	m_plnR.c = m_plnL.c;
	m_plnR.d = 0;
	
	//The top and bottom view frustrums are determined by the aspect ratio:
	
	m_plnT.a = 0;
	m_plnT.b = -fE*fB;
	m_plnT.c = 1/fAspect*fB;
	m_plnT.d = 0;
	


	m_plnB.a = 0;
	m_plnB.b = -m_plnT.b;
	m_plnB.c = m_plnT.c;
	m_plnB.d = 0;
	
	/*::EG_OLprintf(("Engel Right <%.2f, %.2f, %.2f, %.2f>"), m_plnR.a, m_plnR.b, m_plnR.c, m_plnR.d);
	::EG_OLprintf(("Engel Left <%.2f, %.2f, %.2f, %.2f>"), m_plnL.a, m_plnL.b, m_plnL.c, m_plnL.d);
	::EG_OLprintf(("Engel Top <%.2f, %.2f, %.2f, %.2f>"), m_plnT.a, m_plnT.b, m_plnT.c, m_plnT.d);
	::EG_OLprintf(("Engel Bottom <%.2f, %.2f, %.2f, %.2f>"), m_plnB.a, m_plnB.b, m_plnB.c, m_plnB.d);*/
	#else
	eg_angle fAngle = EG_Rad(EG_PI*0.5f - fFOVRad);
	eg_real fCos = EGMath_cos(fAngle);
	eg_real fSin = EGMath_sin(fAngle);

	m_plnR.a = -fSin;
	m_plnR.b = 0;
	m_plnR.c = fCos;
	m_plnR.d = 0;
	

	m_plnL.a = fSin;
	m_plnL.b = 0;
	m_plnL.c = fCos;
	m_plnL.d = 0;
	

	fAngle = EG_Rad(EG_PI*0.5f - fFOVRad/fAspect);
	fCos = EGMath_cos(fAngle);
	fSin = EGMath_sin(fAngle);

	m_plnT.a = 0;
	m_plnT.b = -fSin;
	m_plnT.c = fCos;
	m_plnT.d = 0;
	

	m_plnB.a = 0;
	m_plnB.b = fSin;
	m_plnB.c = fCos;
	m_plnB.d = 0;
	#endif

	#if 0
	::EG_OLprintf(("Right <%.2f, %.2f, %.2f, %.2f>"), m_plnR.a, m_plnR.b, m_plnR.c, m_plnR.d);
	::EG_OLprintf(("Left <%.2f, %.2f, %.2f, %.2f>"), m_plnL.a, m_plnL.b, m_plnL.c, m_plnL.d);
	::EG_OLprintf(("Top <%.2f, %.2f, %.2f, %.2f>"), m_plnT.a, m_plnT.b, m_plnT.c, m_plnT.d);
	::EG_OLprintf(("Bottom <%.2f, %.2f, %.2f, %.2f>"), m_plnB.a, m_plnB.b, m_plnB.c, m_plnB.d);
	#endif
}

void EGViewFr::Initialize(const eg_t_sphere& sph, eg_real /*fNear*/, eg_real fFar)
{
	//The frustrum planes will ignore the near plane, and just use the origin.
	m_plnN.a = 0;
	m_plnN.b = 0;
	m_plnN.c = 1.0f;
	m_plnN.d = 0;

	//The far plane uses the actual far plane:
	m_plnF.a = 0;
	m_plnF.b = 0;
	m_plnF.c = -1.0f;
	m_plnF.d = fFar;

	AdjustAboutSphere(sph);

}

void EGViewFr::AdjustAboutSphere(const eg_t_sphere& sph)
{
	//The idea is to create planes that pass through the origin, and touch the
	//edges of the sphere.

	//First of all, if origin is inside the sphere, we are done, as the whole
	//frustrum should be used.
	eg_vec4 T(sph.GetCenter());
	T.w = 0;
	if(T.LenSqAsVec3() <= sph.GetRadius()*sph.GetRadius())
		return;

	//::EG_OLprintf(("   Sph(%.2f, %.2f, %.2f, %.2f)"), sph.v3Pos.x, sph.v3Pos.y, sph.v3Pos.z, sph.fRadius);

	//To keep this as fast as posible, what we do is generate a plane through one
	//of the ends of the sphere. Technically we should calculate the precise
	//point that the plane should pass through, but for now this seems to work.

	//::EG_OLprintf(("   O Right  <%.2f, %.2f, %.2f, %.2f>"), m_plnR.a, m_plnR.b, m_plnR.c, m_plnR.d);
	//::EG_OLprintf(("   O Left   <%.2f, %.2f, %.2f, %.2f>"), m_plnL.a, m_plnL.b, m_plnL.c, m_plnL.d);
	//::EG_OLprintf(("   O Top    <%.2f, %.2f, %.2f, %.2f>"), m_plnT.a, m_plnT.b, m_plnT.c, m_plnT.d);
	//::EG_OLprintf(("   O Bottom <%.2f, %.2f, %.2f, %.2f>"), m_plnB.a, m_plnB.b, m_plnB.c, m_plnB.d);

	//Right plane:
	m_plnR.a = -sph.GetCenter().z;
	m_plnR.b = 0;
	m_plnR.c = (sph.GetCenter().x+sph.GetRadius());
	m_plnR.d = 0;
	m_plnR.Normalize();

	//Left plane:
	m_plnL.a = sph.GetCenter().z;
	m_plnL.b = 0;
	m_plnL.c = -(sph.GetCenter().x-sph.GetRadius());
	m_plnL.d = 0;
	m_plnL.Normalize();
	
	//Top plane:
	//m_plnT.a = 0;
	//m_plnT.b = sph.v3Pos.z;
	//m_plnT.c = -sph.v3Pos.y;
	//m_plnT.d = sph.fRadius;

	m_plnT.a = 0;
	m_plnT.b = -sph.GetCenter().z;
	m_plnT.c = (sph.GetCenter().y + sph.GetRadius());
	m_plnT.d = 0;
	m_plnT.Normalize();
	
	//Bottom plane:
	//m_plnB.a = 0;
	//m_plnB.b = -sph.v3Pos.z;
	//m_plnB.c = sph.v3Pos.y;
	//m_plnB.d = sph.fRadius;

	m_plnB.a = 0;
	m_plnB.b = sph.GetCenter().z;
	m_plnB.c = -(sph.GetCenter().y - sph.GetRadius());
	m_plnB.d = 0;
	m_plnB.Normalize();
	
	//::EG_OLprintf(("   M Right  <%.2f, %.2f, %.2f, %.2f>"), m_plnR.a, m_plnR.b, m_plnR.c, m_plnR.d);
	//::EG_OLprintf(("   M Left   <%.2f, %.2f, %.2f, %.2f>"), m_plnL.a, m_plnL.b, m_plnL.c, m_plnL.d);
	//::EG_OLprintf(("   M Top    <%.2f, %.2f, %.2f, %.2f>"), m_plnT.a, m_plnT.b, m_plnT.c, m_plnT.d);
	//::EG_OLprintf(("   M Bottom <%.2f, %.2f, %.2f, %.2f>"), m_plnB.a, m_plnB.b, m_plnB.c, m_plnB.d);
}

eg_bool EGViewFr::IsSphereVisible(const eg_t_sphere* pSphere)const
{
	//First, if the camera is inside the sphere, we can see it.
	eg_vec4 T(pSphere->GetCenter());
	T.w = 0;
	if(T.LenSqAsVec3() <= pSphere->GetRadius()*pSphere->GetRadius())
		return true;

	//We start with the near and far planes, because they are the most likely
	//to be clipping.
	eg_real fDist = 0;

	eg_bool bVis = true;


	//Near plane:
	fDist = m_plnN.Dot(pSphere->GetCenter()) + pSphere->GetRadius();
	//::EG_OLprintf(("Near: %.2f"), fDist);
	bVis = bVis && !(fDist < 0);


	
	//Far plane:
	fDist = m_plnF.Dot(pSphere->GetCenter()) + pSphere->GetRadius();
	//::EG_OLprintf(("Far: %.2f"), fDist);
	bVis = bVis && !(fDist < 0);

	//Left and right planes come next because they are 2nd most likely cull planes

	//Left:
	fDist = m_plnL.Dot(pSphere->GetCenter()) + pSphere->GetRadius();
	//::EG_OLprintf(("Left: %.2f"), fDist);
	bVis = bVis && !(fDist < 0);


	//Right:
	fDist = m_plnR.Dot(pSphere->GetCenter()) + pSphere->GetRadius();
	//::EG_OLprintf(("Right: %.2f"), fDist);
	bVis = bVis && !(fDist < 0);


	//Top:
	fDist = m_plnT.Dot(pSphere->GetCenter()) + pSphere->GetRadius();
	//::EG_OLprintf(("Top: %.2f"), fDist);
	bVis = bVis && !(fDist < 0);


	//Bottom:
	fDist = m_plnB.Dot(pSphere->GetCenter()) + pSphere->GetRadius();
	//::EG_OLprintf(("Bottom: %.2f"), fDist);
	bVis = bVis && !(fDist < 0);


	return bVis;
}

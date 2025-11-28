// (c) 2011 Beem Software

#pragma once

#include "EGEnt.h"
#include "EGWorldMapBase.h"
#include "EGViewFr.h"
#include "EGCamera2.h"

class EGDisplayList;

class EGClientView
{
public:

	EGClientView();

	void SetPose( const eg_transform& Pose , eg_real FovDeg , eg_real Near , eg_real Far );
	void SetViewport( const egScreenViewport& NewVp , eg_real GameAspectRatio ){ m_Vp = NewVp; m_GameAspectRatio = GameAspectRatio; }
	void Update();
	void PreSceneSetup(eg_bool SetupGraphicsCam = true );
		
	void WorldToScreen(eg_vec4* pOut, const eg_vec4* pIn);
	void WorldToView(eg_vec4* av3Out, const eg_vec4* av3In, eg_uint n);
	void GetInvView(eg_mat* pInvView)const;
	void GetCameraTraceVecs(eg_real fX, eg_real fY, eg_vec4* pOrg, eg_vec4* pDir);

	const EGCamera2& GetCamera() const{ return m_Camera; }

	const EGViewFr& GetViewFr()const;
	const egScreenViewport& GetViewport()const{ return m_Vp; }
	eg_real GetGameAspectRatio() const { return m_GameAspectRatio; }

	static void SetRenderTransforms( EGDisplayList* DisplayList , const EGCamera2& Camera );
	static void SetReflectiveRenderTransforms( EGDisplayList* DisplayList , const EGCamera2& Camera , const eg_vec4& pvPoint , const eg_vec4& pvNorm );

private:
	void SetRenderTransforms();

private:
	EGCamera2        m_Camera;
	EGViewFr         m_ViewFr;
	egScreenViewport m_Vp;
	eg_real          m_GameAspectRatio;
};
// (c) 2017 Beem Media

#pragma once

#include "EGEntObj.h"
#include "EGCamera2.h"
#include "EGCameraController.h"

class EGDebugSphere;

class EGDefEdPreview
{
private:

	static EGDefEdPreview s_Instance;

private:

	EGCamera2          m_Camera;
	EGCameraController m_CameraController;
	EGDebugSphere*     m_DbLightObj = nullptr;
	eg_real            m_BigDim = 1.f;

public:

	static EGDefEdPreview& Get() { return s_Instance; }

	EGDefEdPreview(): m_Camera( CT_Default ){ }

	void Init();
	void Deinit();
	void Draw();
	void ResetCamera();

	eg_real GetBigDim() const { return m_BigDim; }
	EGCameraController& GetCameraController() { return m_CameraController; }
	eg_bool CanMoveCamera() const;

private:
	
	void Draw_Grid( const eg_real GridSpacing );
	void Draw_ApplyEditorConfig( const eg_vec4& ObjCenter );
};

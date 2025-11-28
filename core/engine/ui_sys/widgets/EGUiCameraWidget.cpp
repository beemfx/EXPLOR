#include "EGUiCameraWidget.h"
#include "EGUiLayout.h"
#include "EGMenu.h"
#include "EGRenderer.h"

EG_CLASS_DECL( EGUiCameraWidget )

void EGUiCameraWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner, InInfo );
	assert( InInfo->Type == egUiWidgetInfo::eg_obj_t::CAMERA );

	if( InInfo->CameraInfo.Type == eg_camera_t_ed::ORTHOGRAPHIC )
	{
		m_Camera.InitOrthographic( 1.f , InInfo->CameraInfo.OrthoRadius*2.f , InInfo->CameraInfo.Near , InInfo->CameraInfo.Far );
	}
	else
	{
		m_Camera.InitPerspective( 1.f , InInfo->CameraInfo.FovDeg , InInfo->CameraInfo.Near , InInfo->CameraInfo.Far );
	}
}


void EGUiCameraWidget::Draw( eg_real AspectRatio )
{
	m_Camera.SetAspectRatio( AspectRatio );
	eg_mat mP = m_Camera.GetProjMat();
	eg_mat mV = m_Camera.GetViewMat();
	MainDisplayList->SetProjTF( mP );
	MainDisplayList->SetViewTF( mV );
	m_Owner->SetCameraPos( m_Camera.GetPose().GetPosition() );
}

void EGUiCameraWidget::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	unused( DeltaTime );
	m_Camera.SetAspectRatio( AspectRatio );
	m_Camera.SetPose( GetFullPose( AspectRatio ) );
}

eg_mat EGUiCameraWidget::GetProjMatrix() const
{
	return m_Camera.GetProjMat();
}

eg_mat EGUiCameraWidget::GetViewMatrix() const
{
	return m_Camera.GetViewMat();
}

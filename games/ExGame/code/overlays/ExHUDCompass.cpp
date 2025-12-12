// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMenu.h"
#include "EGParse.h"
#include "ExEnt.h"
#include "ExMapInfos.h"
#include "ExClient.h"
#include "ExMapping.h"
#include "ExPlayerController.h"
#include "ExHudTextWidget.h"
#include "ExMapIconComponent.h"


class ExHUDCompass: public ExMenu
{
	EG_CLASS_BODY( ExHUDCompass , ExMenu )

private:

	EGUiWidget* m_Compass;
	eg_bool m_bCurrentlyVisible = true;

public:
	
	virtual void OnInit() override final
	{ 
		Super::OnInit();
		m_Compass = GetWidget( eg_crc("Compass") );
		Refresh();
	}

	virtual void OnDeinit() override final
	{
		GetGame()->OnTrackedMapIconsChanged.RemoveAll( this );
		GetGame()->GetMappingModule()->MapMaterialChangedDelegate.RemoveAll( this );

		Super::OnDeinit();
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final
	{
		Super::OnUpdate( DeltaTime , AspectRatio );
		eg_angle Rot = EG_Rad(0.f);
		eg_ent_id EntId = GetGame()->GetPrimaryPlayer();
		const ExEnt* Ent = FindEntData( EntId );
		if( Ent )
		{
			eg_transform EntPose = Ent->GetPose();
			eg_vec4 FaceDir( 0 , 0 , 1.f , 0.f );
			eg_transform EntPoseM( EntPose );
			FaceDir *= EntPoseM;
			const eg_real& y = FaceDir.x;
			const eg_real& x = FaceDir.z;

			Rot = EGMath_atan2( y , x );
		}

		if( m_Compass )
		{
			const eg_real ELEMENT_SCALE = 2.f; // Must match how much the compass is scaled in the layout.
			const eg_real DIST_BETWEEN_TICKS = 5.f; // Must match the distance between ticks in the mesh
			const eg_real NUM_TICKS = 2.f*4.f; // 4 ticks, but the mesh is 2 wide.
			const eg_real MAX_OFFSET = ELEMENT_SCALE*DIST_BETWEEN_TICKS*NUM_TICKS;
			eg_real CompassOffset = EGMath_GetMappedRangeValue( Rot.ToRad() , eg_vec2(0.f , EG_PI*2.f) , eg_vec2(0.f,-MAX_OFFSET) );
			eg_transform CompassTrans;
			CompassTrans = eg_transform::BuildTranslation( eg_vec3(CompassOffset , 0.f , 0.f) );
			m_Compass->SetOffset( EGUiWidget::eg_offset_t::PRE , CompassTrans );
		}
	}

	virtual void OnHudVisibleChanged(  eg_bool bVisible , eg_bool bPlayTransition )
	{
		m_bCurrentlyVisible = bVisible;

		auto HandleMeshWidget = [bVisible,bPlayTransition]( EGUiWidget* Widget ) -> void
		{
			if( Widget )
			{
				if( bVisible )
				{
					Widget->RunEvent( bPlayTransition ? eg_crc("Show") : eg_crc("ShowNow") );
				}
				else
				{
					Widget->RunEvent( bPlayTransition ? eg_crc("Hide") : eg_crc("HideNow") );
				}
			}
		};

		auto HandleTextWidget = [bVisible,bPlayTransition]( ExHudTextWidget* TextWidget ) -> void
		{
			if( TextWidget )
			{
				if( bVisible )
				{
					if( bPlayTransition )
					{
						TextWidget->HudPlayShow();
					}
					else
					{
						TextWidget->HudShowNow();
					}
				}
				else
				{
					if( bPlayTransition )
					{
						TextWidget->HudPlayHide();
					}
					else
					{
						TextWidget->HudHideNow();
					}
				}
			}
		};

		HandleMeshWidget( m_Compass );
		HandleMeshWidget( GetWidget( eg_crc("Bg") ) );
	}
};

EG_CLASS_DECL( ExHUDCompass )

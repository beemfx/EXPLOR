// (c) 2021 Beem Media. All Rights Reserved.

#include "ExMenu.h"
#include "ExClient.h"

class ExProgressSavedOverlay : public ExMenu
{
	EG_CLASS_BODY( ExProgressSavedOverlay , ExMenu )

protected:
	
	enum class ex_show_s
	{
		Hidden ,
		Showing ,
		Hiding ,
	};

protected:
	
	EGUiMeshWidget* m_ProgressSavedWidget = nullptr;
	EGTween<eg_transform> m_WidgetPose;
	eg_real m_ShowTimer = 0.f;
	eg_real m_DurationToShow = 5.f;
	eg_real m_SlideAnimationTime = .5f; // Should match Show/Hide duration in content.
	eg_real m_HideOffset = 45.f;
	ex_show_s m_ShowState = ex_show_s::Hidden;
	eg_transform m_HidePose = CT_Default;

protected:

	virtual void OnInit() override
	{
		Super::OnInit();

		m_HidePose = eg_transform::BuildTranslation( m_HideOffset , 0.f , 0.f );
		m_WidgetPose.SetValue( m_HidePose );
		m_ProgressSavedWidget = GetWidget<EGUiMeshWidget>( eg_crc("ProgressSaved") );
		if( m_ProgressSavedWidget )
		{
			m_ProgressSavedWidget->SetOffset( EGUiWidget::eg_offset_t::PRE , m_HidePose );
			m_ProgressSavedWidget->RunEvent( eg_crc("HideNow") );
		}
	}
	
	virtual void OnActivate() override
	{
		Super::OnActivate();

		if( ExClient* Client = EGCast<ExClient>(GetOwnerClient()) )
		{
			Client->ProgressSavedDelegate.AddUnique( this , &ThisClass::OnProgressSaved );
		}
	}

	virtual void OnDeactivate() override
	{
		if( ExClient* Client = EGCast<ExClient>(GetOwnerClient()) )
		{
			Client->ProgressSavedDelegate.RemoveAll( this );
		}

		Super::OnDeactivate();
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override
	{
		Super::OnUpdate( DeltaTime , AspectRatio );

		m_ShowTimer += DeltaTime;
		m_WidgetPose.Update( DeltaTime );

		switch( m_ShowState )
		{
		case ex_show_s::Hidden:
			break;
		case ex_show_s::Showing:
		{
			if( m_ProgressSavedWidget )
			{
				m_ProgressSavedWidget->SetOffset( EGUiWidget::eg_offset_t::PRE , m_WidgetPose.GetCurrentValue() );
			}

			if( m_ShowTimer >= m_DurationToShow )
			{
				m_ShowState = ex_show_s::Hiding;
				m_ShowTimer = 0.f;
				m_WidgetPose.MoveTo( m_HidePose , m_SlideAnimationTime , eg_tween_animation_t::Cubic );
				if( m_ProgressSavedWidget )
				{
					m_ProgressSavedWidget->RunEvent( eg_crc("Hide") );
				}
			}
		} break;
		case ex_show_s::Hiding:
		{
			if( m_ProgressSavedWidget )
			{
				m_ProgressSavedWidget->SetOffset( EGUiWidget::eg_offset_t::PRE , m_WidgetPose.GetCurrentValue() );
			}

			if( m_ShowTimer >= m_SlideAnimationTime )
			{
				m_ShowState = ex_show_s::Hidden;
				m_ShowTimer = 0.f;
				if( m_ProgressSavedWidget )
				{
					m_ProgressSavedWidget->RunEvent( eg_crc("HideNow") );
				}
			}
		} break;
		}
	}

	void OnProgressSaved()
	{
		m_ShowState = ex_show_s::Showing;
		m_ShowTimer = 0.f;
		m_WidgetPose.MoveTo( CT_Default , m_SlideAnimationTime , eg_tween_animation_t::Cubic );
		if( m_ProgressSavedWidget )
		{
			m_ProgressSavedWidget->RunEvent( eg_crc("Show") );
		}
	}
};

EG_CLASS_DECL( ExProgressSavedOverlay )

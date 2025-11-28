#include "EGMenu.h"
#include "EGClient.h"
#include "EGUiWidget.h"

static const eg_real LOAD_FADEOUT_TIME = 2.f;
static const eg_real SAVE_FADEOUT_TIME = 3.f;

class EGHUDLoading: public EGMenu
{
	EG_CLASS_BODY( EGHUDLoading , EGMenu )

private:
	eg_real m_LoadingCountdown;
	eg_real m_SaveTimer;
	eg_real m_LoadTimer;
	eg_bool m_bHasGoneToWorld:1;

public:

	virtual void OnInit() override final
	{
		Super::OnInit();

		m_LoadingCountdown = 0.f;
		m_SaveTimer = SAVE_FADEOUT_TIME;
		m_bHasGoneToWorld = false;
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final
	{
		Super::OnUpdate( DeltaTime , AspectRatio );

		EGClient* Client = GetOwnerClient();
		if( nullptr == Client )
		{
			assert( false ); // This should be an overlay owned by a client.
			return;
		}

		eg_bool CanNavigateWorld = Client->SDK_AskStateQuestion( eg_client_state_q::IS_CONNECTED) && Client->SDK_AskStateQuestion( eg_client_state_q::HAS_MAP );
		eg_bool IsLoading = Client->SDK_AskStateQuestion( eg_client_state_q::IS_LOADING_THREAD_ACTIVE );
		eg_bool IsSaving = Client->SDK_AskStateQuestion( eg_client_state_q::IS_SAVING );

		// If at any point we can't navigate the world then we haven't gone to the world yet.
		if( !CanNavigateWorld )
		{
			m_bHasGoneToWorld = false;
			m_LoadingCountdown = 0.f;
		}

		// If at any point we can navigate the world and we're not loading we are probably fully loaded so flag as such.
		if( !m_bHasGoneToWorld && CanNavigateWorld && !IsLoading )
		{
			m_bHasGoneToWorld = true;
			m_LoadingCountdown = 0.f;
		}

		if( IsSaving )
		{
			m_SaveTimer = 0.f;
		}

		if( IsLoading )
		{
			m_LoadTimer = 0.f;
		}

		if( !m_bHasGoneToWorld || !CanNavigateWorld )
		{
			m_LoadingCountdown = 0.f;
		}

		eg_real BgAlpha = EG_Clamp( EGMath_GetMappedRangeValue( m_LoadingCountdown , eg_vec2(.2f,LOAD_FADEOUT_TIME) , eg_vec2(1.f,0.f) ) , 0.f , 1.f);

		eg_bool IsFullScreenLoading = !m_bHasGoneToWorld || BgAlpha > 0.f;

		eg_bool ShowActivity = m_SaveTimer < SAVE_FADEOUT_TIME || IsLoading || m_LoadTimer < SAVE_FADEOUT_TIME;
		if( IsFullScreenLoading )
		{
			ShowActivity = false;
		}

		EGUiWidget* BackgroundObj = GetWidget( eg_crc("Background") );
		EGUiWidget* ActivityTextObj = GetWidget( eg_crc("InGameText") );
		EGUiWidget* MessageTextObj = GetWidget( eg_crc("TopText") );
		EGUiWidget* FullScreenNeedleObj = GetWidget( eg_crc("FullScreenNeedle") );
		EGUiWidget* ActivityNeedleObj = GetWidget( eg_crc("InGameNeedle") );

		if( BackgroundObj && ActivityTextObj && MessageTextObj && FullScreenNeedleObj && ActivityNeedleObj )
		{
			MessageTextObj->SetText( eg_crc("") , IsFullScreenLoading ? eg_loc_text(eg_loc("EGTextLoadingText","Loading...")) : eg_loc_text(eg_loc("EGTextSavingText","Saving...")) );
			ActivityTextObj->SetText( eg_crc("") , (IsSaving || m_SaveTimer < SAVE_FADEOUT_TIME) ? eg_loc_text(eg_loc("EGTextSavingText","Saving...")) : eg_loc_text(eg_loc("EGTextLoadingText","Loading...")) );

			MessageTextObj->SetVisible( !m_bHasGoneToWorld );
			BackgroundObj->SetVisible( IsFullScreenLoading );
			BackgroundObj->SetPalette( 0 , eg_color(1.f,1.f,1.f,BgAlpha).ToVec4() );
			FullScreenNeedleObj->SetVisible( !m_bHasGoneToWorld );

			ActivityNeedleObj->SetVisible( ShowActivity );
			ActivityTextObj->SetVisible( ShowActivity );
		}

		m_LoadingCountdown += DeltaTime;
		m_SaveTimer += DeltaTime;
		m_LoadTimer += DeltaTime;
	}
};

EG_CLASS_DECL( EGHUDLoading )

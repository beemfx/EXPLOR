#include "ExMenu.h"
#include "ExClient.h"

class ExLoadingActivityOverlay: public ExMenu
{
	EG_CLASS_BODY( ExLoadingActivityOverlay , ExMenu )

private:
	EGUiWidget*  m_Needle;
	EGUiWidget*  m_LoadingText;
	eg_real      m_Countdown;
	eg_bool      m_bShowSaving:1;

public:

		virtual void OnInit() override final
		{ 
			Super::OnInit();
			assert( m_Countdown == 0.f ); // Data should be wiped before INIT

			m_Needle = GetWidget( eg_crc("Needle") );
			m_LoadingText = GetWidget( eg_crc("LoadingText") );

			assert( m_Needle );
		}

		virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final
		{
			Super::OnUpdate( DeltaTime , AspectRatio );
			m_Countdown += DeltaTime;

			EGClient* Client = GetOwnerClient();
			const eg_bool bHasPlayerEntity = GetGame()->GetPlayerEnt() != nullptr;
			const eg_bool bIsClientConnected = Client->SDK_AskStateQuestion( eg_client_state_q::IS_CONNECTED);
			const eg_bool bHasMap = Client->SDK_AskStateQuestion( eg_client_state_q::HAS_MAP );
			eg_bool CanNavigateWorld = bHasPlayerEntity && bIsClientConnected && bHasMap;
			eg_bool IsLoading = false; //Client->SDK_AskStateQuestion( eg_client_state_q::IS_LOADING_THREAD_ACTIVE );
			eg_bool IsSaving = Client->SDK_AskStateQuestion( eg_client_state_q::IS_SAVING );

			if( !CanNavigateWorld || IsLoading || IsSaving )
			{
				m_Countdown = 0.f;
			}

			// We always prioritize showing the saving message over the loading
			// message. If we have a loading message without a saving one then
			// we switch to the loading message.
			if( IsSaving )
			{
				m_bShowSaving = true;
			}
			else if( IsLoading )
			{
				m_bShowSaving = false;
			}
			else
			{
				// Preserve the message.
			}

			static const eg_real MIN_TIME = 3.f;

			if( m_Needle )
			{
				m_Needle->SetVisible( m_Countdown < MIN_TIME );
			}
			
			if( m_LoadingText )
			{
				m_LoadingText->SetVisible( m_Countdown < MIN_TIME );
				m_LoadingText->SetText( eg_crc("") , m_bShowSaving ? eg_loc_text(eg_loc("LoadingActivitySavingText","Saving...")) : eg_loc_text(eg_loc("LoadingActivityLoadingText","Loading...")) );

			}
		}
};

EG_CLASS_DECL( ExLoadingActivityOverlay )

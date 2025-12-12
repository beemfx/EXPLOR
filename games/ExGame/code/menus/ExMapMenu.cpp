// (c) 2017 Beem Media

#include "ExMenu.h"
#include "ExMapInfos.h"
#include "ExMapping.h"
#include "EGUiImageWidget.h"
#include "ExPlayerController.h"
#include "ExMapIconComponent.h"

class ExMapMenu : public ExMenu
{
	EG_CLASS_BODY( ExMapMenu , ExMenu )

private:

	struct exTrackedIcon
	{
		EGUiImageWidget* Widget = nullptr;
		ExEnt* Ent = nullptr;
	};

private:

	exMapInfoData        m_MapInfo;
	EGWeakPtr<ExMapping> m_Mapping;
	EGUiImageWidget*     m_MapImage;
	EGUiMeshWidget*      m_PlayerPose;
	EGUiTextWidget*      m_PlayerPosText;
	EGUiImageWidget*     m_MapIconTemplate;
	eg_vec2              m_OriginalMapSize;
	eg_vec2              m_FinalScale = eg_vec2(1.f,1.f);
	eg_ivec2             m_MapSize = CT_Clear;
	eg_ivec2             m_LowerLeftMapOrigin = CT_Clear;
	eg_bool              m_bIsRevealed = false;
	EGArray<EGUiImageWidget*> m_AvailableMapIcons;
	EGArray<exTrackedIcon> m_TrackedIcons;


public:

	virtual void OnInit() override
	{
		Super::OnInit();

		HideHUD();
	
		m_Mapping = GetGame()->GetMappingModule();
		m_MapImage = GetWidget<EGUiImageWidget>( eg_crc("MapImage") );
		m_PlayerPose = GetWidget<EGUiMeshWidget>( eg_crc("PlayerPose") );
		m_PlayerPosText = GetWidget<EGUiTextWidget>( eg_crc("PlayerPosText") );
		m_MapIconTemplate = GetWidget<EGUiImageWidget>( eg_crc("MapIconTemplate") );

		if( m_MapImage )
		{
			m_MapImage->SetOverrideSamplerState( eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT );
			
			m_OriginalMapSize.x = m_MapImage->GetInfo()->ScaleVec.x;
			m_OriginalMapSize.y = m_MapImage->GetInfo()->ScaleVec.y;

			m_MapImage->SetMaterial( eg_crc("") , eg_crc("") , m_Mapping->GetMapRenderMaterial() );

			// Decide which way to scale the map.
			m_MapSize = m_Mapping->GetMapSize();
			m_LowerLeftMapOrigin = m_Mapping->GetMapLowerLeftOrigin();
			eg_int MaxMapSize = EG_Max( m_MapSize.x , m_MapSize.y );

			m_FinalScale = eg_vec2(1.f,1.f);

			if( m_MapSize.x > m_MapSize.y )
			{
				m_FinalScale.y = EG_To<eg_real>(m_MapSize.y)/EG_To<eg_real>(m_MapSize.x);
			}
			else
			{
				m_FinalScale.x = EG_To<eg_real>(m_MapSize.x)/EG_To<eg_real>(m_MapSize.y);
			}

			// If the map isn't very big we don't want it taking up the whole screen
			const eg_real MinDisplaySize = 16.f;
			if( MaxMapSize < MinDisplaySize )
			{
				m_FinalScale.x *= MaxMapSize/MinDisplaySize;
				m_FinalScale.y *= MaxMapSize/MinDisplaySize;
			}

			m_FinalScale.x *= m_OriginalMapSize.x;
			m_FinalScale.y *= m_OriginalMapSize.y;

			m_MapImage->GetEntObj().SetScaleVec(eg_vec4( m_FinalScale.x , m_FinalScale.y , 1.f , 1.f) );
		}

		EGUiTextWidget* TitleText = GetWidget<EGUiTextWidget>( eg_crc("TitleText") );
		ExGame* Game = GetGame();
		if( TitleText && Game )
		{
			TitleText->SetText( CT_Clear , eg_loc_text(Game->GetCurrentMapInfo().NameCrc) );
		}

		if( Game )
		{
			Game->OnTrackedMapIconsChanged.AddUnique( this , &ThisClass::OnTrackedMapIconsChanged );
		}

		ClearHints();
		AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("MapMenuBackOutHint","Back")) );

		DoReveal();

		if( m_MapIconTemplate )
		{
			m_MapIconTemplate->SetVisible( false );
		}
	}

	virtual void OnRevealComplete() override
	{
		Super::OnRevealComplete();

		m_bIsRevealed = true;

		if( m_MapIconTemplate )
		{
			m_MapIconTemplate->SetVisible( false );
		}

		OnTrackedMapIconsChanged();
	}

	virtual void OnDeinit() override
	{
		ShowHUD( false );

		if( ExGame* Game = GetGame() )
		{
			Game->OnTrackedMapIconsChanged.RemoveAll( this );
		}

		Super::OnDeinit();
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override
	{
		Super::OnUpdate( DeltaTime , AspectRatio );

		if( m_bIsRevealed )
		{
			SetWorldWidgetPose( m_PlayerPose , GetGame()->GetPlayerEnt() , true );

			for( const exTrackedIcon& TrackedIcon : m_TrackedIcons )
			{
				SetWorldWidgetPose( TrackedIcon.Widget , TrackedIcon.Ent , false );
			}
		}
	}

	virtual void OnInputCmds( const struct egLockstepCmds& Cmds ) override
	{
		if( HandlePossibleMenuToggle( ExMenu::ex_toggle_menu_t::Map , Cmds ) )
		{
			return;
		}

		Super::OnInputCmds( Cmds );
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override
	{
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			MenuStack_Pop();
			return true;
		}

		return Super::OnInput( InputType );
	}

	virtual void OnPreDraw( eg_real AspectRatio ) override
	{
		Super::OnPreDraw( AspectRatio );

		if( m_Mapping.IsValid() )
		{
			m_Mapping->SetGlobalSamplerToMap();
		}
	}

	void SetWorldWidgetPose( EGUiMeshWidget* Widget , const ExEnt* Ent , eg_bool bIsPartyPose )
	{
		if( Widget && m_Mapping )
		{
			const eg_real SquareSize = m_FinalScale.x / m_MapSize.x;

			const exGridPose GridPose = m_Mapping->WorldPoseToGridPos( Ent ? Ent->GetPose() : CT_Default );

			const eg_aabb& PoseBounds = Widget->GetEntObj().GetBaseBounds();
			const eg_vec2 OriginalWidgetSize( PoseBounds.GetWidth() , PoseBounds.GetHeight() );


			eg_ivec2 OffsetFromMapOrigin = GridPose.GridPos - m_LowerLeftMapOrigin;

			eg_transform IconTr( CT_Default );
			if( bIsPartyPose )
			{
				IconTr.RotateZThis( -GetGame()->GetPlayerRotation() );

				if( m_PlayerPosText )
				{
					m_PlayerPosText->SetText( eg_crc("") , EGFormat( eg_loc("MapMenuPlayerPosText","Party Position: {0}, {1}") , GridPose.GridPos.x , GridPose.GridPos.y ) );
				}
			}

			const eg_real AdditionalScale = bIsPartyPose ? 1.9f : 2.f;

			eg_vec2 FinalOffset(0.f,0.f);
			FinalOffset.x = (1.f - m_MapSize.x) * SquareSize*.5f;
			FinalOffset.y = (1.f - m_MapSize.y) * SquareSize*.5f;

			FinalOffset.x += OffsetFromMapOrigin.x * SquareSize;
			FinalOffset.y += OffsetFromMapOrigin.y * SquareSize;

			IconTr.TranslateThis( FinalOffset.x , FinalOffset.y , 0.f );
			Widget->GetEntObj().SetScale( (SquareSize/(OriginalWidgetSize.x+1.f))*AdditionalScale );
			Widget->SetOffset( EGUiWidget::eg_offset_t::PRE , IconTr );
			Widget->SetVisible( m_Mapping->IsPoseRevealed( GridPose ) );
		}
	}

	EGUiImageWidget* CreateMapIcon( egv_material InMat )
	{
		EGUiImageWidget* OutImage = nullptr;
		if( m_AvailableMapIcons.HasItems() )
		{
			OutImage = m_AvailableMapIcons.Last();
			m_AvailableMapIcons.DeleteByItem( OutImage );
		}

		if( nullptr == OutImage )
		{
			OutImage = EGCast<EGUiImageWidget>(DuplicateWidget( m_MapIconTemplate ));
		}

		if( OutImage )
		{
			OutImage->SetVisible( true );
			OutImage->SetMaterial( CT_Clear , CT_Clear , InMat );
		}

		return OutImage;
	}

	void DestroyMapIcon( EGUiImageWidget* InIcon )
	{
		if( InIcon )
		{
			InIcon->SetVisible( false );
			m_AvailableMapIcons.Append( InIcon );
		}
	}

	void ClearTrackedIcons()
	{
		for( exTrackedIcon& TrackedIcon : m_TrackedIcons )
		{
			DestroyMapIcon( TrackedIcon.Widget );
		}

		m_TrackedIcons.Clear( false );
	}

	void OnTrackedMapIconsChanged()
	{
		ClearTrackedIcons();

		EGArray<ExMapIconComponent*> TrackedMapIcons;
		if( ExGame* Game = GetGame() )
		{
			TrackedMapIcons = Game->GetTrackedMapIcons();
		}

		for( ExMapIconComponent* TrackedMapIcon : TrackedMapIcons )
		{
			ExEnt* Ent = TrackedMapIcon ? TrackedMapIcon->GetOwner<ExEnt>() : nullptr;
			if( Ent )
			{
				exTrackedIcon NewIcon;
				NewIcon.Ent = Ent;
				NewIcon.Widget = CreateMapIcon( TrackedMapIcon->GetIconMaterial() );
				m_TrackedIcons.Append( NewIcon );
			}
		}
	}
};

EG_CLASS_DECL( ExMapMenu )
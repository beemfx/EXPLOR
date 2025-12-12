// (c) 2016 Beem Media. All Rights Reserved.

#include "ExHUD.h"
#include "ExClient.h"
#include "ExMenu.h"

ExHUD::ExHUD( ExClient* Owner ) 
: m_Owner( Owner )
{
	assert( nullptr != Owner );
}

ExHUD::~ExHUD()
{

}

void ExHUD::InitClientComponents()
{
	assert( nullptr == m_ProgressSavedOverlay );
	m_ProgressSavedOverlay = EGCast<ExMenu>(m_Owner->SDK_AddOverlay( eg_crc("ProgressSavedOverlay") , 0.f , true ));
	assert( nullptr != m_ProgressSavedOverlay );
}

void ExHUD::DeinitClientComponents()
{
	assert( m_ProgressSavedOverlay );
	m_Owner->SDK_RemoveOverlay( m_ProgressSavedOverlay );
	m_ProgressSavedOverlay =  nullptr;
}

void ExHUD::InitGameComponents()
{
	assert( nullptr == m_CharacterPortraits && nullptr == m_Automap && nullptr == m_BaseHud );
	m_CharacterPortraits = EGCast<ExMenu>(m_Owner->SDK_AddOverlay( eg_crc( "CharacterPortraits" ), 0.f ));
	m_Automap = EGCast<ExMenu>(m_Owner->SDK_AddOverlay( eg_crc( "HUDAutomap" ), 0.f ));
	m_BaseHud = EGCast<ExMenu>(m_Owner->SDK_AddOverlay( eg_crc("HUDBase") , 0.f ));
	m_Compass = EGCast<ExMenu>(m_Owner->SDK_AddOverlay( eg_crc("HUDCompass") , -1.f ));

	if( m_HideCount > 0 )
	{
		SetWidgetVisible( m_Automap , false , false );
		SetWidgetVisible( m_CharacterPortraits , false , false );
		SetWidgetVisible( m_BaseHud , false , false );
		SetWidgetVisible( m_Compass , false , false );
	}

	assert( nullptr != m_CharacterPortraits && nullptr != m_Automap && nullptr != m_BaseHud && nullptr != m_Compass );
}

void ExHUD::DeinitGameComponents()
{
	assert( m_CharacterPortraits && m_Automap && m_BaseHud && m_Compass );
	m_Owner->SDK_RemoveOverlay( m_CharacterPortraits );
	m_Owner->SDK_RemoveOverlay( m_Automap );
	m_Owner->SDK_RemoveOverlay( m_BaseHud );
	m_Owner->SDK_RemoveOverlay( m_Compass );
	m_CharacterPortraits = nullptr;
	m_Automap = nullptr;
	m_BaseHud = nullptr;
	m_Compass = nullptr;
}

void ExHUD::Update( eg_real DeltaTime )
{
	unused( DeltaTime );
}

void ExHUD::Refresh()
{
	auto RefreshWidget = [this]( EGMenu* Widget ) -> void
	{
		ExMenu* ExWidget = EGCast<ExMenu>( Widget );
		if( ExWidget )
		{
			ExWidget->Refresh();
		}
	};

	RefreshWidget( m_Automap );
	RefreshWidget( m_CharacterPortraits );
	RefreshWidget( m_BaseHud );
	RefreshWidget( m_Compass );
}

void ExHUD::Hide()
{
	if( 0 == m_HideCount )
	{
		SetWidgetVisible( m_Automap , false , true );
		SetWidgetVisible( m_CharacterPortraits , false , true );
		SetWidgetVisible( m_BaseHud , false , true );
		SetWidgetVisible( m_Compass , false , true );
	}
	m_HideCount++;
}

void ExHUD::Show( eg_bool bImmediatePortraits )
{
	m_HideCount--;
	if( 0 == m_HideCount )
	{
		SetWidgetVisible( m_Automap , true , true );
		SetWidgetVisible( m_CharacterPortraits , true , !bImmediatePortraits );
		SetWidgetVisible( m_BaseHud , true , true );
		SetWidgetVisible( m_Compass , true , true );
	}
}

void ExHUD::SetWidgetVisible( ExMenu* Widget , eg_bool bVisible , eg_bool bAnimate )
{
	if( Widget )
	{
		Widget->HudSetVisible( bVisible , bAnimate );
	}
}

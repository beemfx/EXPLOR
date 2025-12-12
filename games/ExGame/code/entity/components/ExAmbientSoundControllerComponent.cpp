// (c) 2020 Beem Media. All Rights Reserved.

#include "ExAmbientSoundControllerComponent.h"
#include "ExClient.h"
#include "EGMenu.h"
#include "EGMenuStack.h"

EG_CLASS_DECL( ExAmbientSoundControllerComponent )

void ExAmbientSoundControllerComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	const ExAmbientSoundControllerComponent* DefAsAC = EGCast<ExAmbientSoundControllerComponent>( InitData.Def );
	m_bSetVolumeByLoadingOpacity = DefAsAC->m_bSetVolumeByLoadingOpacity;
	m_bDisableForMenus = DefAsAC->m_bDisableForMenus;
}

void ExAmbientSoundControllerComponent::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( IsClient() && m_bSetVolumeByLoadingOpacity )
	{
		if( EGEnt* EntOwner = GetOwner<EGEnt>() )
		{
			if( ExClient* Client = EGCast<ExClient>(EntOwner->GetOwnerClient()) )
			{
				OnLoadingOpacityChanged( Client->GetLoadingOpacity() );
				Client->OnLoadingOpacityChangedDelegate.AddUnique( this , &ThisClass::OnLoadingOpacityChanged );
			}
		}
	}

	if( IsClient() && m_bDisableForMenus )
	{
		if( IsClient() )
		{
			if( EGEnt* OwnerEnt = GetOwner<EGEnt>() )
			{
				if( EGClient* Client = OwnerEnt->GetOwnerClient() )
				{
					EGMenuStack* MenuStack = Client ? Client->SDK_GetMenuStack() : nullptr;
					if( MenuStack )
					{
						m_MenuCount = GetAudioDisablingMenus( MenuStack );
						OnMenuStackChanged( MenuStack );
						MenuStack->MenuStackChangedDelegate.AddUnique( this , &ThisClass::OnMenuStackChanged );
					}
				}
			}
		}
	}
}

void ExAmbientSoundControllerComponent::OnLeaveWorld()
{
	if( IsClient() && m_bSetVolumeByLoadingOpacity )
	{
		if( EGEnt* EntOwner = GetOwner<EGEnt>() )
		{
			if( ExClient* Client = EGCast<ExClient>(EntOwner->GetOwnerClient()) )
			{
				Client->OnLoadingOpacityChangedDelegate.RemoveAll( this );
			}
		}
	}

	if( IsClient() && m_bDisableForMenus )
	{
		if( IsClient() )
		{
			if( EGEnt* OwnerEnt = GetOwner<EGEnt>() )
			{
				if( EGClient* Client = OwnerEnt->GetOwnerClient() )
				{
					EGMenuStack* MenuStack = Client ? Client->SDK_GetMenuStack() : nullptr;
					if( MenuStack )
					{
						MenuStack->MenuStackChangedDelegate.RemoveAll( this );
					}
				}
			}
		}
	}

	Super::OnLeaveWorld();
}

void ExAmbientSoundControllerComponent::OnLoadingOpacityChanged( eg_real LoadingOpacity )
{
	assert( IsClient() && m_bSetVolumeByLoadingOpacity ); // Shouldn't be called if property not set.
	SetVolume( 1.f - LoadingOpacity );
}


void ExAmbientSoundControllerComponent::OnMenuStackChanged( EGMenuStack* MenuStack )
{
	const eg_int OldMenuCount = m_MenuCount;
	m_MenuCount = GetAudioDisablingMenus( MenuStack );

	if( m_MenuCount != 0 && OldMenuCount == 0 )
	{
		if( m_bDisableForMenus )
		{
			SetIsPlaying( false );
		}
	}

	if( m_MenuCount == 0 && OldMenuCount != 0 )
	{
		if( m_bDisableForMenus )
		{
			SetIsPlaying( true );
		}
	}
}

eg_int ExAmbientSoundControllerComponent::GetAudioDisablingMenus( EGMenuStack* MenuStack )
{
	eg_int Count = 0;

	static eg_cpstr MenusToIgnore[] =
	{
		"ExConversationMenu" ,
		"ExDialog" ,
		"ExCastSpellInWorldMenu" ,
		"ExContextMenu" ,
		"ExChoiceWidgetMenu" ,
	};

	auto ShouldIgnore = []( EGMenu* Menu ) -> eg_bool
	{
		if( Menu )
		{
			for( eg_cpstr MenuId : MenusToIgnore )
			{
				if( EGString_EqualsI( Menu->GetObjectClass()->GetName() , MenuId ) )
				{
					return true;
				}
			}
		}

		return false;
	};

	if( MenuStack )
	{
		for( eg_size_t i=0; i<MenuStack->Len(); i++ )
		{
			if( EGMenu* Menu = MenuStack->GetMenuByIndex( i ) )
			{
				if( !ShouldIgnore( Menu ) )
				{
					Count++;
				}
			}

		}
	}

	return Count;
}

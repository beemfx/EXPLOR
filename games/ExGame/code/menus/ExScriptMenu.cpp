// (c) 2017 Beem Media

#include "ExScriptMenu.h"

EG_CLASS_DECL( ExScriptMenu )

void ExScriptMenu::OnUpdate( eg_real DeltaTime, eg_real AspectRatio )
{
	Super::OnUpdate( DeltaTime , AspectRatio );

	if( m_bWasDismissed )
	{
		m_DismissTime -= DeltaTime;
		if( m_DismissTime <= 0.f && IsActive() )
		{
			MenuStack_Pop();
		}
	}
}

void ExScriptMenu::Refresh()
{
	Super::Refresh();
}

void ExScriptMenu::DismissMenu()
{
	m_bWasDismissed = true;

}

void ExScriptMenu::Continue()
{
	m_bWaitingForServer = false;
}

void ExScriptMenu::SendChoiceToServer( eg_uint ChoiceIndex )
{
	m_bWaitingForServer = true;
	GetGame()->SDK_RunServerEvent( eg_crc("OnScriptMenuChoice") , ChoiceIndex );
}

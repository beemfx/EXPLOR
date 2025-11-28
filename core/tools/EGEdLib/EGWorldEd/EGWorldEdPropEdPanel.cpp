// (c) 2018 Beem Media

#include "EGWorldEdPropEdPanel.h"
#include "EGWorldEd.h"

EGWorldEdPropEdPanel::EGWorldEdPropEdPanel( EGWndPanel* Parent )
: EGWndPanelPropEditor( Parent )
{

}

void EGWorldEdPropEdPanel::HandlePropChanged( const egRflEditor* Editor )
{
	Super::HandlePropChanged( Editor );

	if( m_CurEditor && Editor )
	{
		EGWorldEd_OnPropChanged( *m_CurEditor , *Editor );
	}
}

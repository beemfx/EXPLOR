// (c) 2018 Beem Media

#pragma once

#include "EGWorldEdPropEdPanel.h"
#include "EGWndPanelPropEditor.h"

class EGWorldEdPropEdPanel : public EGWndPanelPropEditor
{
	EG_DECL_SUPER( EGWndPanelPropEditor )

public:

	EGWorldEdPropEdPanel( EGWndPanel* Parent );

	virtual void HandlePropChanged( const egRflEditor* Editor ) override;
};

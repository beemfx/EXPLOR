// (c) 2017 Beem Media

#include "EGLytEdSettingsPanel.h"
#include "EGLytEd.h"

EGLytEdSettingsPanel* EGLytEdSettingsPanel::GlobalPanel = nullptr;

EGLytEdSettingsPanel::EGLytEdSettingsPanel( EGWndPanel* Parent )
: Super( Parent )
{
	assert( GlobalPanel == nullptr ); // Can have only one properties panel
	GlobalPanel = this;
}

EGLytEdSettingsPanel::~EGLytEdSettingsPanel()
{
	GlobalPanel = nullptr;
}

void EGLytEdSettingsPanel::HandlePropChanged( const egRflEditor* Property )
{
	Super::HandlePropChanged( Property );

	EGLytEd_SetDirty();
}


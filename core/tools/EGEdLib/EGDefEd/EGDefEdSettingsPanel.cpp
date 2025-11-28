// (c) 2017 Beem Media

#include "EGDefEdSettingsPanel.h"
#include "EGDefEd.h"
#include "EGDefEdFile.h"

EGDefEdSettingsPanel::EGDefEdSettingsPanel( EGWndPanel* Parent )
: Super( Parent )
{

}

void EGDefEdSettingsPanel::HandlePropChanged( const egRflEditor* Property )
{
	Super::HandlePropChanged( Property );
	egEntDefEditNeeds Needs;
	EGDefEdFile::Get().ApplySettingChanges( Property , Needs );
	if( Needs.bRefreshEditor )
	{
		PropEditorRebuild();
	}
	EGDefEd_SetDirty();
}

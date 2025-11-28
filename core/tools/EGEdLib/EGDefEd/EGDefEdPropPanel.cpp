// (c) 2017 Beem Media

#include "EGDefEdPropPanel.h"
#include "EGDefEdFile.h"
#include "EGWndPropControl.h"

EGDefEdPropPanel* EGDefEdPropPanel::s_PropPanel = nullptr;

EGDefEdPropPanel::EGDefEdPropPanel( EGWndPanel* Parent )
: Super( Parent )
{
	assert( s_PropPanel == nullptr ); // Can have only one properties panel
	s_PropPanel = this;

	SetNoPropsMessage( GetNoPropsMessage( true , false ) );
}

EGDefEdPropPanel::~EGDefEdPropPanel()
{
	s_PropPanel = nullptr;
}

void EGDefEdPropPanel::HandlePropChanged( const egRflEditor* Editor )
{
	EGDefEdFile::Get().ApplyComponentChanges( m_CurEditor , Editor );
}

void EGDefEdPropPanel::CommitChangesFromPreviewPanel()
{
	EGDefEdFile::Get().ApplyComponentChanges( m_CurEditor , nullptr );

	for( egProp& Prop : m_Props )
	{
		if( Prop.Control )
		{
			if( Prop.Info.PropEditor && EGString_Equals( Prop.Info.PropEditor->GetVarName() , "m_BasePose") )
			{
				Prop.Control->RefreshFromSourceData();
			}
		}
	}
}

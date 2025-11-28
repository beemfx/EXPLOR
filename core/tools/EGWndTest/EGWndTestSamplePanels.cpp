// (c) 2017 Beem Media

#include "EGWndTestSamplePanels.h"

void EGWndTestSampleDragableEditor::OnDrawItem( HDC hdc , eg_int ItemIndex , const RECT& rc , eg_bool bIsBeingDroppedOn , eg_bool bIsHovered , eg_bool bIsSelected ) const
{
	// Super draws the background
	Super::OnDrawItem( hdc , ItemIndex , rc , bIsBeingDroppedOn , bIsHovered , bIsSelected );
	const eg_string_small& Item = m_Items[ItemIndex];
	RECT rcLineSize = rc;
	InflateRect( &rcLineSize , -5 , -1 );
	DrawTextA( hdc , Item , -1 , &rcLineSize , DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX );
}

#include "EGUiClearZWidget.h"
#include "EGUiLayout.h"
#include "EGRenderer.h"

EG_CLASS_DECL( EGUiClearZWidget )

void EGUiClearZWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner, InInfo );
	assert( InInfo->Type == egUiWidgetInfo::eg_obj_t::CLEARZ );
}

void EGUiClearZWidget::Draw( eg_real AspectRatio )
{
	unused( AspectRatio );
	MainDisplayList->ClearDS( 1.f , 0 );
}

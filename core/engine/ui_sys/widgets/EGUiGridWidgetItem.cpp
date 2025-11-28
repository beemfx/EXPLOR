#include "EGUiGridWidgetItem.h"
#include "EGUiGridWidget.h"

EG_CLASS_DECL( EGUiGridWidgetItem )

void EGUiGridWidgetItem::DrawAsGridItem( const eg_transform& Pose , const eg_vec4& ScaleVec , eg_bool bIsLit )
{
	m_EntObj.SetDrawInfo( Pose , ScaleVec , bIsLit );
	if( m_OverrideSamplerState != eg_sampler_s::COUNT )
	{
		MainDisplayList->PushSamplerState( m_OverrideSamplerState );
	}
	m_EntObj.Draw();
	if( m_OverrideSamplerState != eg_sampler_s::COUNT )
	{
		MainDisplayList->PopSamplerState();
	}
}

void EGUiGridWidgetItem::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	Super::Update( DeltaTime , AspectRatio );
}

void EGUiGridWidgetItem::SetVisible( eg_bool Visible )
{
	unused( Visible );
	assert( false ); // Visibility of grids is managed by owner object.
}

void EGUiGridWidgetItem::SetEnabled( eg_bool InFocus )
{
	unused( InFocus );
	assert( false ); // Enabled state is managed by owner object (but maybe we want to disable particular itmes to be headers?
}

eg_uint EGUiGridWidgetItem::GetSelectedIndex()
{
	return Owner->GetSelectedIndex();
}

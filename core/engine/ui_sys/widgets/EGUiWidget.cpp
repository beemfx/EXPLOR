#include "EGUiWidget.h"
#include "EGUiLayout.h"
#include "EGMenu.h"
#include "EGUiClearZWidget.h"
#include "EGUiCameraWidget.h"
#include "EGUiMeshWidget.h"
#include "EGUiButtonWidget.h"
#include "EGUiGridWidget.h"
#include "EGUiTextWidget.h"
#include "EGUiLightWidget.h"
#include "EGUiScrollbarWidget.h"
#include "EGUiImageWidget.h"
#include "EGUiGridWidget2.h"

EG_CLASS_DECL( EGUiWidget )

EGUiWidget* EGUiWidget::CreateUiObject( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	assert( InOwner && InInfo );

	eg_bool bForceDefaultClass = false;
	eg_bool bSearchEntDictForClass = false;
	EGClass* DefaultClass = nullptr;
	EGClass* SecondDefaultClass = nullptr;

	if( InOwner && InInfo )
	{
		switch( GetTypeFromLayoutObjInfo( InInfo ) )
		{
			case eg_t::UNK:
				assert( false );
				break;
			case eg_t::CLEARZ:
				DefaultClass = &EGUiClearZWidget::GetStaticClass();
				break;
			case eg_t::TEXT:
				DefaultClass = &EGUiTextWidget::GetStaticClass();
				break;
			case eg_t::CAMERA:
				DefaultClass = &EGUiCameraWidget::GetStaticClass();;
				break;
			case eg_t::ENTOBJ:
				DefaultClass = &EGUiMeshWidget::GetStaticClass();
				bSearchEntDictForClass = true;
				break;
			case eg_t::ENTBUTTON:
				DefaultClass = &EGUiButtonWidget::GetStaticClass();
				bSearchEntDictForClass = true;
				break;
			case eg_t::ENTGRID:
				DefaultClass = &EGUiGridWidget::GetStaticClass();
				SecondDefaultClass = &EGUiGridWidget2::GetStaticClass();
				break;
			case eg_t::SCROLLBAR:
				DefaultClass = &EGUiScrollbarWidget::GetStaticClass();
				bSearchEntDictForClass = true;
				break;
			case eg_t::LIGHT:
				DefaultClass = &EGUiLightWidget::GetStaticClass();
				break;
			case eg_t::IMAGE:
				DefaultClass = &EGUiImageWidget::GetStaticClass();
				bSearchEntDictForClass = true;
				break;
		}
	}

	EGClass* ObjClass = FindUiObjectClass( DefaultClass , SecondDefaultClass , InInfo->ClassName.Class ? InInfo->ClassName.Class->GetName() : "" , InInfo->EntDefCrc , bForceDefaultClass , bSearchEntDictForClass );
	
	EGUiWidget* Out = EGNewObject<EGUiWidget>( ObjClass , eg_mem_pool::DefaultHi );
	if( Out )
	{
		Out->Init( InOwner , InInfo );
	}

	return Out;
}

void EGUiWidget::DestroyUiObject( EGUiWidget* UiObj )
{
	EGDeleteObject( UiObj );
}

EGUiWidget::eg_t EGUiWidget::GetTypeFromLayoutObjInfo( const egUiWidgetInfo* Info )
{
	eg_t Out = eg_t::UNK;

	if( Info )
	{
		switch( Info->Type )
		{
		case egUiWidgetInfo::eg_obj_t::CLEARZ:
			Out = eg_t::CLEARZ;
			break;
		case egUiWidgetInfo::eg_obj_t::CAMERA:
			Out = eg_t::CAMERA;
			break;
		case egUiWidgetInfo::eg_obj_t::TEXT_NODE:
			Out = eg_t::TEXT;
			break;
		case egUiWidgetInfo::eg_obj_t::LIGHT:
			Out = eg_t::LIGHT;
			break;
		case egUiWidgetInfo::eg_obj_t::IMAGE:
			Out = eg_t::IMAGE;
			break;
		case egUiWidgetInfo::eg_obj_t::MESH:
		{
			switch( Info->WidgetType )
			{
			case eg_widget_t::NONE:
				Out = eg_t::ENTOBJ;
				break;
			case eg_widget_t::BUTTON:
				Out = eg_t::ENTBUTTON;
				break;
			case eg_widget_t::GRID:
				Out = eg_t::ENTGRID;
				break;
			case eg_widget_t::SCROLLBAR:
				Out = eg_t::SCROLLBAR;
				break;
			}
		} break;
		}
	}

	return Out;
}

void EGUiWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	m_Owner = InOwner;
	m_Info = InInfo;
	if( m_Info )
	{
		m_Adjacency = m_Info->Adjacency;
	}
	assert( InOwner && InInfo ); // This is going to crash later if these don't exist.
}

EGUiWidget::eg_t EGUiWidget::GetType() const
{
	return GetTypeFromLayoutObjInfo( m_Info );
}

eg_transform EGUiWidget::GetFullPose( eg_real AspectRatio ) const
{
	eg_transform AnchorPoint( CT_Default );

	if( m_Info->Anchor.X == eg_anchor_t::RIGHT )
	{
		AnchorPoint.TranslateThis( eg_vec3( AspectRatio*MENU_ORTHO_SIZE , 0.f , 0.f ) );
	}
	else if( m_Info->Anchor.X == eg_anchor_t::LEFT )
	{
		AnchorPoint.TranslateThis( eg_vec3( -AspectRatio*MENU_ORTHO_SIZE , 0.f , 0.f ) );;
	}

	if( m_Info->Anchor.Y == eg_anchor_t::TOP )
	{
		AnchorPoint.TranslateThis( eg_vec3( 0.f , MENU_ORTHO_SIZE , 0.f ) );
	}
	else if( m_Info->Anchor.Y == eg_anchor_t::BOTTOM )
	{
		AnchorPoint.TranslateThis( eg_vec3( 0.f , -MENU_ORTHO_SIZE , 0.f ) );
	}

	eg_transform Out = m_PreOffset * m_Info->BasePose * AnchorPoint * m_PostOffset;

	return Out;
}

eg_bool EGUiWidget::IsVisibleWidget( eg_bool bIncludeUnfocusble /*= true */ ) const
{
	eg_bool Out = false;
	if( IsEnabled() && IsVisible() && IsWidget() )
	{
		Out = IsFocusable() || bIncludeUnfocusble;
	}
	return Out;
}

eg_aabb EGUiWidget::GetBoundsInSearchSpace()
{
	// Identity matrices will be fine.
	return GetBoundsInMouseSpace( 1.f , eg_mat::I , eg_mat::I );
}

eg_bool EGUiWidget::IsToolPreview() const
{
	return m_Owner && m_Owner->GetLayout() && m_Owner->GetLayout()->m_IsInTool;
}

eg_string_crc EGUiWidget::GetId() const
{
	return m_Info->Id;
}

eg_string_crc EGUiWidget::GetAdjacentWidgetId( eg_widget_adjacency Dir ) const
{
	switch( Dir )
	{
		case eg_widget_adjacency::UP: return m_Adjacency.Up;
		case eg_widget_adjacency::DOWN: return m_Adjacency.Down;
		case eg_widget_adjacency::LEFT: return m_Adjacency.Left;
		case eg_widget_adjacency::RIGHT: return m_Adjacency.Right;
	}

	return CT_Clear;
}

EGClass* EGUiWidget::FindUiObjectClass( EGClass* DefaultClass, EGClass* SecondDefaultClass , eg_cpstr ClassName, eg_string_crc EntDefCrc, eg_bool bForceDefaultClass, eg_bool bSearchEntDict )
{
	assert( DefaultClass != nullptr );

	EGClass* Out = nullptr;

	if( bForceDefaultClass )
	{
		Out = DefaultClass;
	}
	else
	{
		if( ClassName[0] != '\0' )
		{
			Out = EGClass::FindClass( ClassName );
		}
		else if( bSearchEntDict )
		{
			const class EGEntDef* EntDef = EntDict_GetDef( EntDefCrc );
			Out = EntDef ? EntDef->m_UiClassName.Class : nullptr;
		}
	}

	if( Out && Out->IsA( DefaultClass ) )
	{

	}
	else if( Out && SecondDefaultClass && Out->IsA( SecondDefaultClass ) )
	{

	}
	else
	{
		if( Out )
		{
			EGLogf( eg_log_t::Warning, "Widget Class Failure: %s was not an appropriate subclass of %s.", Out->GetName(), DefaultClass ? DefaultClass->GetName() : "UNK" );
		}
		Out = DefaultClass;
	}

	return Out;
}

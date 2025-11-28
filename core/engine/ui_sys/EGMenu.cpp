#include "EGMenu.h"
#include "EGEngine.h"
#include "EGEngineConfig.h"
#include "EGEnt.h"
#include "EGEntDict.h"
#include "EGEnt.h"
#include "EGLocalize.h"
#include "EGAudio.h"
#include "EGMenuStack.h"
#include "EGRenderer.h"
#include "EGEntObj.h"
#include "EGOverlayMgr.h"
#include "EGSaveMgr.h"
#include "EGTextFormat.h"
#include "EGQueue.h"
#include "EGUiWidget.h"
#include "EGUiGridWidget.h"
#include "EGUiGridWidget2.h"
#include "EGUiDragAndDropWidget.h"
#include "EGGlobalConfig.h"

EG_CLASS_DECL( EGMenu )

static const egUiWidgetInfo EGMenu_DefaultCamera( eg_camera_t::ORTHO );

EGUiWidget* EGMenu::__Construct_InsertObj( const egUiWidgetInfo* Info, eg_bool bInsertFirst )
{
	// Children go first...
	EGUiWidget* New = EGUiWidget::CreateUiObject( this , Info );
	if( nullptr != New )
	{
		if( bInsertFirst )
		{
			m_Objects.InsertFirst( New );
		}
		else
		{
			m_Objects.InsertLast( New );
		}
	}
	return New;
}

void EGMenu::__Construct_InitObjs()
{
	//Create all objects:
	eg_bool bHasCameraAtBeginning = false;
	eg_bool bFoundDisplayObj = false;
	for( const egUiWidgetInfo* Info : m_Layout->m_Objects )
	{
		if( Info->IsGuide && !m_bIsTool )
		{
			continue;
		}
		if( Info->Type == egUiWidgetInfo::eg_obj_t::CAMERA && !bFoundDisplayObj )
		{
			bHasCameraAtBeginning = true;
		}
		if( Info->Type != egUiWidgetInfo::eg_obj_t::CLEARZ )
		{
			bFoundDisplayObj = true;
		}

		__Construct_InsertObj( Info, false );
	}

	if( !bHasCameraAtBeginning )
	{
		__Construct_InsertObj( &EGMenu_DefaultCamera , true );
	}
}

EGMenu::EGMenu()
: m_Layout( nullptr )
, m_Font( nullptr )
, m_CaptureWidget( nullptr )
, m_FocusedWidget( nullptr )
, m_LastMouseOverWidget( nullptr )
, m_Objects( OBJ_LIST_ID )
, m_AspectRatio( 0 )
, m_bWasLastInputByMouse( false )
, m_bVisible( true )
, m_LightCount( 0 )
, m_bIgnoreAltNav( false )
{

}

void EGMenu::Init( eg_string_crc MenuCrc, EGMenuStack* MenuStackOwner, EGOverlayMgr* OverlayOwner, EGClient* OwnerClient )
{
	m_bIsTool = false;
	m_OwnerMenuStack = MenuStackOwner;
	m_OwnerOverlayMgr = OverlayOwner;
	m_OwnerClient = OwnerClient;

	m_LastMousePos = eg_vec2( 0.f, 0.f );
	m_Layout = EGUiLayoutMgr::Get()->GetLayout( MenuCrc );
	m_Font = EGFontMgr::Get()->GetFont( CON_FONT_ID );

	__Construct_InitObjs();
}

void EGMenu::InitForTool( EGUiLayout* MenuLayout )
{
	m_bIsTool = true;
	m_Layout = MenuLayout;
	m_Font = EGFontMgr::Get()->GetFont( CON_FONT_ID );

	__Construct_InitObjs();

	for( EGUiWidget* Object : m_Objects )
	{
		{
			EGUiGridWidget* AsGrid = EGCast<EGUiGridWidget>(Object);
			if( AsGrid )
			{
				AsGrid->RefreshGridWidget( AsGrid->GetNumVisibleColumns() * AsGrid->GetNumVisibleRows() * 3 );
			}
		}

		{
			EGUiGridWidget2* AsGrid2 = EGCast<EGUiGridWidget2>(Object);
			if( AsGrid2 )
			{
				AsGrid2->RefreshGridWidget( AsGrid2->GetNumVisibleColumns() * AsGrid2->GetNumVisibleRows() * 3 );
			}
		}
	}

}

EGMenu::~EGMenu()
{
	if( m_FocusedWidget )
	{
		m_FocusedWidget->OnFocusLost();
		m_FocusedWidget = nullptr;
	}
	while( m_Objects.Len() > 0 )
	{
		EGUiWidget* Info = m_Objects.GetLast();
		m_Objects.Remove( Info );
		EGUiWidget::DestroyUiObject( Info );
	}

	m_Font = nullptr;
}

void EGMenu::FormatText( eg_cpstr Flags, EGTextParmFormatter* Formatter ) const
{
	unused( Flags );
	Formatter->SetText( "NOT_HANDLED" );
}

eg_bool EGMenu::IsActive() const
{
	return m_OwnerMenuStack && m_OwnerMenuStack->GetActiveMenu() == this;
}

EGUiWidget* EGMenu::FindObjectFromLayout( const egUiWidgetInfo* Info )
{
	EGUiWidget* Out = nullptr;

	for( EGUiWidget* Obj : m_Objects )
	{
		if( Obj->GetInfo() == Info )
		{
			Out = Obj;
			break;
		}
	}

	return Out;
}

eg_real EGMenu::GetAspect()const
{
	assert( m_AspectRatio > 0.f );
	return m_AspectRatio;
}

void EGMenu::ProcessEvent( eg_menuevent_t Event , eg_parm Parm1 /*= 0*/ , eg_parm Parm2 /*= 0*/ )
{
	if( eg_menuevent_t::DRAW == Event )
	{
		Draw();
	}

	if( Event == eg_menuevent_t::INIT || Event == eg_menuevent_t::DEINIT || Event == eg_menuevent_t::ACTIVATE || Event == eg_menuevent_t::DEACTIVATE )
	{
		m_bWasLastInputByMouse = false;
		m_LastMouseOverWidget = nullptr;
	}

	if( eg_menuevent_t::ACTIVATE == Event && m_OwnerMenuStack && !m_Layout->m_NoInput )
	{
		m_OwnerMenuStack->AcquireInput();
	}

	if( eg_menuevent_t::INIT == Event )
	{
		m_CaptureWidget = nullptr;
		m_FocusedWidget = nullptr;
		m_LastMouseOverWidget = nullptr;
		m_bWasLastInputByMouse = false;

		for( EGUiWidget* ListItem : m_Objects )
		{
			EGUiWidget* Obj = ListItem;

			if( Obj->GetInfo()->StartupEvent.Crc.IsNotNull() )
			{
				Obj->RunEvent( Obj->GetInfo()->StartupEvent.Crc );
			}
		}
	}

	if( eg_menuevent_t::UPDATE == Event )
	{
		Update( Parm1.as_real(), Parm2.as_real() );
	}

	if( eg_menuevent_t::INIT == Event )
	{
		for( EGUiWidget* ListItem : m_Objects )
		{
			EGUiWidget* Obj = ListItem;

			if( Obj != m_FocusedWidget )
			{
				if( Obj->IsFocusable() )
				{
					Obj->OnFocusLost();
				}
			}
		}
	}

	switch( Event )
	{
	case eg_menuevent_t::NONE:
		assert( false );
		break;
	case eg_menuevent_t::INIT:
		OnInit();
		break;
	case eg_menuevent_t::DEINIT:
		OnDeinit();
		break;
	case eg_menuevent_t::ACTIVATE:
		OnActivate();
		break;
	case eg_menuevent_t::DEACTIVATE:
		OnDeactivate();
		break;
	case eg_menuevent_t::UPDATE:
		OnUpdate( Parm1.as_real() , Parm2.as_real() );
		break;
	case eg_menuevent_t::PRE_DRAW:
		OnPreDraw( Parm1.as_real() );
		break;
	case eg_menuevent_t::DRAW:
		OnDraw( Parm1.as_real() );
		break;
	case eg_menuevent_t::POST_DRAW:
		OnPostDraw( Parm1.as_real() );
		break;
	}

	if( eg_menuevent_t::DEINIT == Event )
	{
		for( EGUiWidget* ListItem : m_Objects )
		{
			EGUiWidget* Obj = ListItem;

			if( Obj != m_FocusedWidget )
			{
				if( Obj->IsFocusable() )
				{
					Obj->OnFocusLost();
				}
			}
		}
	}
}

eg_bool EGMenu::HandleInput( eg_menuinput_t InputType )
{
	if( m_bInputDisabled )
	{
		return true;
	}

	// All input is consumed if there is a mouse capture object.
	if( m_CaptureWidget )
	{
		m_CaptureWidget->HandleInput( InputType , eg_vec2(0.f,0.f) , false );
		return true;
	}

	// Keep track if the last event was by mouse, that way we can avoid
	// having the mouse interfere with widgets if the user isn't actually
	// doing anything with it.
	static const eg_menuinput_t EventByMouseList[] =
	{
		eg_menuinput_t::SCROLL_UP, //Mouse only
		eg_menuinput_t::SCROLL_DOWN, //Mouse only
	};

	static const eg_menuinput_t EventNotByMouseList[] =
	{
		eg_menuinput_t::BUTTON_PRIMARY, //A
		eg_menuinput_t::BUTTON_BACK,    //B
		eg_menuinput_t::BUTTON_UP,
		eg_menuinput_t::BUTTON_DOWN,
		eg_menuinput_t::BUTTON_LEFT,
		eg_menuinput_t::BUTTON_RIGHT,
		eg_menuinput_t::NEXT_PAGE,
		eg_menuinput_t::PREV_PAGE,
		eg_menuinput_t::NEXT_SUBPAGE,
		eg_menuinput_t::PREV_SUBPAGE,
	};

	for( const eg_menuinput_t CheckEvent : EventByMouseList )
	{
		if( InputType == CheckEvent )
		{
			m_bWasLastInputByMouse = true;
		}
	}

	for( eg_menuinput_t CheckEvent : EventNotByMouseList )
	{
		if( InputType == CheckEvent )
		{
			m_bWasLastInputByMouse = false;
		}
	}

	eg_bool WidgetConsumed = false;

	// Pass the event to the proc
	if( !WidgetConsumed )
	{
		WidgetConsumed = OnInput( InputType );
	}

	if( !WidgetConsumed && nullptr != m_FocusedWidget && m_FocusedWidget->IsVisible() && m_FocusedWidget->IsEnabled() )
	{
		WidgetConsumed = m_FocusedWidget->HandleInput( InputType , eg_vec2( 0, 0 ) , false );
	}

	if( !WidgetConsumed )
	{
		if( eg_menuinput_t::BUTTON_LEFT == InputType )
		{
			MoveSelection( eg_widget_adjacency::LEFT );
		}

		if( eg_menuinput_t::BUTTON_RIGHT == InputType )
		{
			MoveSelection( eg_widget_adjacency::RIGHT );
		}

		if( eg_menuinput_t::BUTTON_UP == InputType )
		{
			MoveSelection( eg_widget_adjacency::UP );
		}

		if( eg_menuinput_t::BUTTON_DOWN == InputType )
		{
			MoveSelection( eg_widget_adjacency::DOWN );
		}
	}

	return WidgetConsumed;
}

eg_bool EGMenu::HandleMouseEvent( eg_menumouse_e MouseEvent, eg_real xPos, eg_real yPos )
{
	if( m_bInputDisabled )
	{
		return true;
	}

	// Keep track if the last event was by mouse, that way we can avoid
	// having the mouse interfere with widgets if the user isn't actually
	// doing anything with it.
	static const eg_menumouse_e EventByMouseList[] =
	{
		eg_menumouse_e::MOVE,
		eg_menumouse_e::PRESSED,
		eg_menumouse_e::RELEASED,
	};

	for( const eg_menumouse_e CheckEvent : EventByMouseList )
	{
		if( MouseEvent == CheckEvent )
		{
			m_bWasLastInputByMouse = true;
		}
	}

	eg_bool WidgetConsumed = false;

	if( eg_menumouse_e::MOVE == MouseEvent )
	{
		m_LastMousePos.x = xPos;
		m_LastMousePos.y = yPos;

		eg_vec2 WidgetHitPoint(0.f,0.f);
		EGUiWidget* HoverWidget = FindWidgetAt( xPos , yPos , &WidgetHitPoint , m_CaptureWidget );
		if( m_LastMouseOverWidget != HoverWidget )
		{
			if( m_LastMouseOverWidget )
			{
				m_LastMouseOverWidget->OnMouseMovedOff();
			}

			if( HoverWidget )
			{
				WidgetConsumed = HoverWidget->OnMouseMovedOn( WidgetHitPoint );
			}
		}
		m_LastMouseOverWidget = HoverWidget;
		if( HoverWidget )
		{
			WidgetConsumed = HoverWidget->OnMouseMovedOver( WidgetHitPoint , m_CaptureWidget != nullptr ) || WidgetConsumed;
		}
	}

	if( eg_menumouse_e::PRESSED == MouseEvent )
	{
		// Force a selection change by generating a MOVE event.
		WidgetConsumed = HandleMouseEvent( eg_menumouse_e::MOVE , xPos , yPos );

		eg_vec2 WidgetHitPoint;
		EGUiWidget* ClickObj = FindWidgetAt( xPos , yPos , &WidgetHitPoint, nullptr );

		if( ClickObj )
		{
			WidgetConsumed = ClickObj->OnMousePressed( WidgetHitPoint );
		}
	}

	if( eg_menumouse_e::RELEASED == MouseEvent )
	{
		m_LastMousePos.x = xPos;
		m_LastMousePos.y = yPos;

		eg_vec2 WidgetHitPoint;
		EGUiWidget* ClickObj = FindWidgetAt( xPos , yPos , &WidgetHitPoint, nullptr );
		if( nullptr != m_CaptureWidget )
		{
			WidgetConsumed = m_CaptureWidget->OnMouseReleased( WidgetHitPoint , ClickObj );
		}

		// Force a selection change by generating a MOVE event.
		WidgetConsumed = HandleMouseEvent( eg_menumouse_e::MOVE , xPos , yPos ) || WidgetConsumed;

		// If nothing consumed the mouse click, treat it as BUTTON_PRIMARY.
		if( !WidgetConsumed )
		{
			WidgetConsumed = OnInput( eg_menuinput_t::BUTTON_PRIMARY );
		}
	}

	return WidgetConsumed;
}

void EGMenu::ProcessInputCmds( const struct egLockstepCmds& Cmds )
{
	if( m_bInputDisabled )
	{
		return;
	}

	OnInputCmds( Cmds );
}

void EGMenu::MoveSelection( eg_widget_adjacency Direction )
{
	auto DoLinearSearch = [this]( eg_bool bForward )
	{
		EGUiWidget* FoundObj = nullptr;

		if( bForward )
		{
			EGUiWidget* SearchStart = nullptr == m_FocusedWidget ? m_Objects.GetFirst() : m_FocusedWidget;
			for( EGUiWidget* Obj = SearchStart; nullptr != Obj; Obj = m_Objects.GetNext(Obj) )
			{
				if( Obj != m_FocusedWidget && Obj->IsVisibleWidget( false ) )
				{
					FoundObj = Obj;
					break;
				}
			}
		}
		else
		{
			EGUiWidget* SearchStart = nullptr == m_FocusedWidget ? m_Objects.GetLast() : m_FocusedWidget;
			for( EGUiWidget* Obj = SearchStart; nullptr != Obj; Obj = m_Objects.GetPrev(Obj) )
			{
				if( Obj != m_FocusedWidget && Obj->IsVisibleWidget( false ) )
				{
					FoundObj = Obj;
					break;
				}
			}
		}

		if( nullptr != FoundObj )
		{
			ChangeFocusInternal( FoundObj , false , eg_vec2(0,0));
		}
	};

	auto DoSearchByBounds = [this,Direction]()
	{
		eg_aabb ObjBounds = m_FocusedWidget->GetBoundsInSearchSpace();
		eg_vec4 ObjCenter = ObjBounds.GetCenter();
		eg_aabb SearchBox = ObjBounds;

		static const eg_real INF = 1e32f;
		static const eg_real ADJ = EG_REALLY_SMALL_NUMBER; // The adj is so we don't overlap.


		switch( Direction )
		{
		case eg_widget_adjacency::UP: 
			SearchBox.Min.y = ObjBounds.Max.y + ADJ;
			SearchBox.Max.y = INF;
			break;
		case eg_widget_adjacency::DOWN: 
			SearchBox.Max.y = ObjBounds.Min.y - ADJ;
			SearchBox.Min.y = -INF;
			break;
		case eg_widget_adjacency::RIGHT: 
			SearchBox.Min.x = ObjBounds.Max.x + ADJ;
			SearchBox.Max.x = INF; 
			break;
		case eg_widget_adjacency::LEFT: 
			SearchBox.Max.x = ObjBounds.Min.x - ADJ;
			SearchBox.Min.x = -INF; 
			break;
		}

		// Find all objects in the search box and save off the closest one.
		EGUiWidget* FoundObj = nullptr;
		eg_real FoundObjDistSq = INF;
		for( EGUiWidget* Obj : m_Objects )
		{
			if( Obj != m_FocusedWidget && Obj->IsVisibleWidget( false ) && Obj->FocusOnHover() )
			{
				eg_aabb CmpBounds = Obj->GetBoundsInSearchSpace();
				if( SearchBox.Intersect( CmpBounds , nullptr ) )
				{
					eg_real DistSq = (CmpBounds.GetCenter() - ObjCenter).LenSqAsVec3();
					if( FoundObj )
					{
						if( DistSq < FoundObjDistSq )
						{
							FoundObj = Obj;
							FoundObjDistSq = DistSq;
						}
					}
					else
					{
						FoundObj = Obj;
						FoundObjDistSq = DistSq;
					}
				}
			}
		}

		if( nullptr != FoundObj )
		{
			ChangeFocusInternal( FoundObj , false , eg_vec2(0,0) );
		}
	};

	auto DoSearchByDirection = [this,&Direction]() -> void
	{
		EGUiWidget* SearchWidget = m_FocusedWidget;
		// We'll search till we find an active widget, but no more than the total number of widgets (in case there is an infinte loop)
		const eg_size_t MaxSearch = m_Objects.Len();
		for( eg_size_t i=0; SearchWidget && i<MaxSearch; i++ )
		{
			eg_string_crc AdjId = SearchWidget->GetAdjacentWidgetId( Direction );
			if( AdjId.IsNotNull() )
			{
				SearchWidget = GetWidget( AdjId );
				if( SearchWidget && SearchWidget->IsEnabled() )
				{
					break;
				}
			}
			else
			{
				SearchWidget = nullptr;
				break;
			}
		}

		if( SearchWidget )
		{
			ChangeFocusInternal( SearchWidget , false , eg_vec2(0,0) );
		}
	};

	// If nothing is selected just find the first object (menu code should
	// choose a first object if this isn't desired).
	if( nullptr == m_FocusedWidget )
	{
		DoLinearSearch( true );
	}
	else
	{
		eg_string_crc AdjacencyWidgetId = m_FocusedWidget->GetAdjacentWidgetId( Direction );
		if( AdjacencyWidgetId.IsNotNull() )
		{
			DoSearchByDirection();
		}
		else
		{
			DoSearchByBounds();
		}
	}
	
}

void EGMenu::ChangeFocusInternal( EGUiWidget* NewSelection , eg_bool FromMouse , const eg_vec2& WidgetHitPoint )
{
	EGUiWidget* OldSelection = m_FocusedWidget;

	eg_bool AllowChange = true;

	if( AllowChange )
	{
		m_FocusedWidget = NewSelection;
	}

	if( OldSelection != m_FocusedWidget )
	{
		if( nullptr != OldSelection )OldSelection->OnFocusLost();
		if( nullptr != NewSelection )NewSelection->OnFocusGained( FromMouse , WidgetHitPoint );

		// It's possible that focusing a widget focused another widget so only
		// call the event if the focused widget is still the one set to.
		if( m_FocusedWidget == NewSelection )
		{
			OnFocusedWidgetChanged( NewSelection , OldSelection );
		}
	}
}

void EGMenu::Draw()
{
	if( 0.f == m_AspectRatio )
	{
		//This is kind of a stupid hack, but if the aspect ratio hasn't been set it means Update
		//hasn't been called yet, so we shouldn't be drawing (this usually happens when an 
		//INIT/ACTIVATE/UPDATE/DEACTIVATE/DEINIT pushes a menu.)
		EGLogf( eg_log_t::Warning , __FUNCTION__ ": Menu \"%s\" had not been updated yet." , m_Layout->GetFilename() );
		return;
	}

	if( !m_bVisible )
	{
		return;
	}

	//MainDisplayList->ClearDS( 1.0f , 0 );
	MainDisplayList->SetProjTF( eg_mat::I );
	MainDisplayList->SetViewTF( eg_mat::I );

	m_LightCount = 0;

	//Draw Objects:
	for( EGUiWidget* Object : m_Objects )
	{
		if( Object->IsVisible() )
		{
			Object->Draw( m_AspectRatio );
		}
	}

	MainDisplayList->EnableLight( 0 , false );
	MainDisplayList->SetVec4( eg_rv4_t::AMBIENT_LIGHT , eg_vec4(1.f,1.f,1.f,1.f) );

	//Draw Button Bounds:
	if( DebugConfig_DrawMenuButtons.GetValue() || GlobalConfig_IsUiLayoutTool.GetValue() )
	{
		MainDisplayList->PushDefaultShader( eg_defaultshader_t::TEXTURE );
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
		MainDisplayList->SetMaterial( EGV_MATERIAL_NULL );

		eg_mat MatProj( CT_Default );
		eg_mat MatView( CT_Default );

		MainDisplayList->SetWorldTF( eg_mat::I );
		MainDisplayList->SetViewTF( eg_mat::I );
		MainDisplayList->SetProjTF( eg_mat::I );

		for( EGUiWidget* Object : m_Objects )
		{
			switch( Object->GetType() )
			{
			case EGUiWidget::eg_t::CAMERA:
			{
				MatProj = Object->GetProjMatrix();
				MatView = Object->GetViewMatrix();
			} break;
			case EGUiWidget::eg_t::LIGHT:
			case EGUiWidget::eg_t::TEXT:
			case EGUiWidget::eg_t::ENTOBJ:
			case EGUiWidget::eg_t::ENTBUTTON:
			case EGUiWidget::eg_t::ENTGRID:
			case EGUiWidget::eg_t::SCROLLBAR:
			case EGUiWidget::eg_t::IMAGE:
			{
				if( Object->IsVisibleWidget() || GlobalConfig_IsUiLayoutTool.GetValue() )
				{
					if( GlobalConfig_IsUiLayoutTool.GetValue() && Object->GetInfo()->bIsLocked )
					{
					}
					else
					{
						Draw_ObjBoundsDebug( Object, MatView, MatProj );
					}
				}
			} break;
			case EGUiWidget::eg_t::CLEARZ:
			case EGUiWidget::eg_t::UNK:
			{
			} break;
			}
		}
		MainDisplayList->PopDepthStencilState();
		MainDisplayList->PopDefaultShader();
	}
}

void EGMenu::Draw_ObjBoundsDebug( EGUiWidget* Object, const eg_mat& MatView, const eg_mat& MatProj )
{
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::TEXTURE );
	MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );

	MainDisplayList->SetMaterial( EGV_MATERIAL_NULL );

	eg_aabb bb = Object->GetBoundsInMouseSpace( GetAspect() , MatView , MatProj );
	bb.Min.x /= GetAspect();
	bb.Max.x /= GetAspect();
	bb.Min.z = bb.Max.z = 0;

	eg_color32 Color(0,200,200);
	if( Object->IsVisibleWidget() )
	{
		Color = eg_color32(0,255,0);
	}
	else if( Object->GetType() == EGUiWidget::eg_t::TEXT )
	{
		Color = eg_color32(75,0,130);
	}
	MainDisplayList->DrawAABB( bb , eg_color(Color) );


	MainDisplayList->PopDefaultShader();
	MainDisplayList->PopDepthStencilState();
}

void EGMenu::Update( eg_real DeltaTime , eg_real AspectRatio )
{
	// A DeltaTime of 0.f means the menu was just created.
	m_AspectRatio = AspectRatio;

	for( EGUiWidget* Object : m_Objects )
	{
		Object->Update( DeltaTime , AspectRatio );
	}
}

EGUiWidget* EGMenu::FindObjectAt( eg_real x , eg_real y , eg_vec2* WidgetHitPointOut , const EGUiWidget* ForceFindObj , eg_bool bWidgetsOnly )
{
	eg_vec4 HitPoint( x , y , 0 , 1.f );
	EGUiWidget* OutObj = nullptr;
	eg_mat ProjMat( CT_Default );
	eg_mat ViewMat( CT_Default );

	for( EGUiWidget* _Info : m_Objects )
	{
		EGUiWidget* Obj = _Info;

		switch( Obj->GetType() )
		{
		case EGUiWidget::eg_t::UNK:
		case EGUiWidget::eg_t::CLEARZ:
		{

		} break;
		case EGUiWidget::eg_t::CAMERA:
		{
			ProjMat = Obj->GetProjMatrix();
			ViewMat = Obj->GetViewMatrix();
		} break;
		case EGUiWidget::eg_t::LIGHT:
		case EGUiWidget::eg_t::TEXT:
		case EGUiWidget::eg_t::ENTOBJ:
		case EGUiWidget::eg_t::ENTBUTTON:
		case EGUiWidget::eg_t::ENTGRID:
		case EGUiWidget::eg_t::SCROLLBAR:
		case EGUiWidget::eg_t::IMAGE:
		{
			if( m_Layout->m_IsInTool && Obj->GetInfo()->bIsLocked )
			{
				continue;
			}

			if( (bWidgetsOnly && Obj->IsVisibleWidget()) || (!bWidgetsOnly) )
			{
				const eg_aabb bb = Obj->GetBoundsInMouseSpace( GetAspect() , ViewMat , ProjMat );

				if( (nullptr == ForceFindObj && bb.ContainsPoint( HitPoint )) || (nullptr != ForceFindObj && Obj == ForceFindObj) )
				{
					OutObj = Obj;

					if( WidgetHitPointOut )
					{
						*WidgetHitPointOut = GetWidgetHitPoint( Obj, eg_vec2(HitPoint.x,HitPoint.y) , ViewMat , ProjMat );
						//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( "Widget Hit %g,%g" , HitPointOut->x , HitPointOut->y ) );
						if(ForceFindObj == nullptr && !OutObj->IsHitPointValid( *WidgetHitPointOut ) )
						{
							OutObj = nullptr;
						}
					}
				}
			}
		} break;
		}
	}

	return OutObj;
}

void EGMenu::AddLight( const EGLight& Lt, const eg_color& AmbientLight )
{
	m_AmbientLight = AmbientLight;
	
	if( 0 <= m_LightCount && m_LightCount < countof(m_Lights) )
	{
		m_Lights[m_LightCount] = Lt;
		m_LightCount++;
	}
	else
	{
		m_Lights[0] = Lt;
	}
}

void EGMenu::ClearLights()
{
	m_LightCount = 0;
}

eg_uint EGMenu::GetLights( EGLight* OutArray, eg_uint MaxOut, eg_vec4* CameraPoseOut, eg_color* AmbientColorOut )
{
	eg_uint NumOut = EG_Min(MaxOut,m_LightCount);

	for( eg_uint i=0; i<NumOut; i++ )
	{
		OutArray[i] = m_Lights[i];
	}

	*AmbientColorOut = m_AmbientLight;
	*CameraPoseOut = m_CameraPos;

	return NumOut;
}

EGMenu* EGMenu::GetParentMenu() const
{
	return m_OwnerMenuStack ? m_OwnerMenuStack->GetParentMenu( this ) : nullptr;
}

EGUiWidget* EGMenu::FindWidgetAt( eg_real x, eg_real y, eg_vec2* HitPointOut, const EGUiWidget* ForceFindObj )
{
	return FindObjectAt( x , y , HitPointOut , ForceFindObj , true );
}

eg_vec2 EGMenu::GetWidgetHitPoint( const EGUiWidget* Obj, const eg_vec2& HitPoint , const eg_mat& ViewMat , const eg_mat& ProjMat ) const
{
	eg_vec2 Out(0,0);
	const eg_aabb bb = Obj->GetBoundsInMouseSpace( GetAspect() , ViewMat , ProjMat );
	// We also want to computer where on the surface of the widget the point hit (x and y are in the range of [0,1] not screen space.
	Out.x = EGMath_GetMappedRangeValue( HitPoint.x , eg_vec2(bb.Min.x,bb.Max.x) , eg_vec2(0,1.f) );
	Out.y = EGMath_GetMappedRangeValue( HitPoint.y , eg_vec2(bb.Min.y,bb.Max.y) , eg_vec2(0,1.f) );

	return Out;
}

EGMenu* EGMenu::MenuStack_SwitchTo( eg_string_crc MenuId )
{ 
	if( m_OwnerMenuStack )
	{
		return m_OwnerMenuStack->SwitchTo( MenuId );
	}

	EGLogf( eg_log_t::Error , __FUNCTION__ ": %s can't be called on overlays." , m_Layout && m_Layout->m_ClassName.Class ? m_Layout->m_ClassName.Class->GetName() : "" );
	return nullptr;
}

EGMenu* EGMenu::MenuStack_PushTo( eg_string_crc MenuId )
{ 
	if( m_OwnerMenuStack )
	{
		return m_OwnerMenuStack->Push( MenuId );
	}

	EGLogf( eg_log_t::Error , __FUNCTION__ ": %s can't be called on overlays." , m_Layout && m_Layout->m_ClassName.Class ? m_Layout->m_ClassName.Class->GetName() : "" );
	return nullptr;
}

void EGMenu::MenuStack_Pop()
{
	if( m_OwnerMenuStack )
	{
		m_OwnerMenuStack->Pop();
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": %s can't be called on overlays." , m_Layout && m_Layout->m_ClassName.Class ? m_Layout->m_ClassName.Class->GetName() : "" );
	}
}

void EGMenu::MenuStack_PopTo( EGMenu* Menu )
{
	if( m_OwnerMenuStack )
	{
		m_OwnerMenuStack->PopTo( Menu );
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": %s can't be called on overlays." , m_Layout && m_Layout->m_ClassName.Class ? m_Layout->m_ClassName.Class->GetName() : "" );
	}
}

void EGMenu::MenuStack_Clear()
{
	if( m_OwnerMenuStack )
	{
		m_OwnerMenuStack->Clear();
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": %s can't be called on overlays." , m_Layout && m_Layout->m_ClassName.Class ? m_Layout->m_ClassName.Class->GetName() : "" );
	}
}

void EGMenu::SetVisible( eg_bool bVisible )
{
	m_bVisible = bVisible;
}

EGUiWidget* EGMenu::GetWidget( eg_string_crc CrcId )
{
	if( CrcId == eg_string_crc( CT_Clear ) )
	{
		return nullptr;
	}

	EGUiWidget* Out = nullptr;
	for( EGUiWidget* Obj : m_Objects )
	{
		if( Obj->GetInfo()->Id == CrcId )
		{
			Out = Obj;
			break;
		}
	}
	return Out;
}

EGUiWidget* EGMenu::GetFocusedWidget()
{
	return m_FocusedWidget;
}

EGUiWidget* EGMenu::DuplicateWidget( const EGUiWidget* ObjToDuplicate )
{
	EGUiWidget* NewObj = nullptr;

	const EGUiWidget* ObjToDuplicateAsUiObject = ObjToDuplicate;

	if( ObjToDuplicateAsUiObject )
	{
		NewObj = EGUiWidget::CreateUiObject( this , ObjToDuplicateAsUiObject->GetInfo() );
		m_Objects.InsertAfter( const_cast<EGUiWidget*>(ObjToDuplicateAsUiObject) , NewObj );
	}

	return NewObj;
}

void EGMenu::MoveWidgetAfter( EGUiWidget* WidgetToMove , EGUiWidget* WidgetToMoveAfter )
{
	if( WidgetToMove )
	{
		if( nullptr != WidgetToMoveAfter )
		{
			if( m_Objects.GetNext( WidgetToMoveAfter ) != WidgetToMove )
			{
				m_Objects.Remove( WidgetToMove );
				m_Objects.InsertAfter( WidgetToMoveAfter , WidgetToMove );
			}
		}
		else
		{
			if( WidgetToMove != m_Objects.GetFirst() )
			{
				m_Objects.Remove( WidgetToMove );
				m_Objects.InsertFirst( WidgetToMove );
			}
		}
	}
}

void EGMenu::SetFocusedWidget( EGUiWidget* NewFocusWidget, eg_uint NewSelectedIndex, eg_bool bAllowAudio )
{
	if( NewFocusWidget )
	{
		if( !bAllowAudio )
		{
			NewFocusWidget->SetMuteAudio( true );
		}
		ChangeFocusInternal( NewFocusWidget , false , eg_vec2(0,0) );
		NewFocusWidget->OnSetSelectedIndex( NewSelectedIndex );

		if( !bAllowAudio )
		{
			NewFocusWidget->SetMuteAudio( false );
		}
	}
	else
	{
		ChangeFocusInternal( nullptr , false , eg_vec2(0,0) );
	}
}

void EGMenu::SetFocusedWidget( eg_string_crc CrcId, eg_uint Index, eg_bool bAllowAudio )
{
	if( CrcId == eg_string_crc( CT_Clear ) )
	{
		ChangeFocusInternal( nullptr , false , eg_vec2(0,0) );
		return;
	}

	EGUiWidget* Widget = GetWidget( CrcId );
	if( Widget )
	{
		SetFocusedWidget( Widget , Index , bAllowAudio );
	}
	else
	{
		EGLogf( eg_log_t::Warning , __FUNCTION__ ": Tried to focus %08X, no such widget." , CrcId.ToUint32() );
	}
}

void EGMenu::BeginMouseCapture( EGUiWidget* Widget )
{
	assert( Widget && nullptr == m_CaptureWidget );
	m_CaptureWidget = Widget;
}

void EGMenu::EndMouseCapture()
{
	assert( m_CaptureWidget );
	m_CaptureWidget = nullptr;
}

EGClient* EGMenu::GetOwnerClient()const
{
	EGClient* Out = m_OwnerClient;
	#if defined( __DEBUG__ )
	if( m_OwnerOverlayMgr )
	{
		assert( m_OwnerOverlayMgr->GetClient() == m_OwnerClient );
	}
	#endif
	return Out;
}

EGClient* EGMenu::GetPrimaryClient()const
{
	return Engine_GetClientByIndex( 0 );
}

void EGMenu::SetAllObjectsVisibility( eg_bool bVisible )
{
	for( EGUiWidget* Object : m_Objects )
	{
		if( !Object->IgnoreDuringSetAllVisibilty() )
		{
			Object->SetVisible( bVisible );
		}
	}
}

void EGMenu::SetDragAndDropWidget( EGUiDragAndDropWidget* NewDragAndDropWidget )
{
	assert( nullptr == m_CaptureWidget ); // Should not change this while dragging and dropping is happening.
	m_DragAndDropWidget = NewDragAndDropWidget;
	if( m_DragAndDropWidget )
	{
		m_DragAndDropWidget->OnInitDragAndDrop();
	}
}

EGUiDragAndDropWidget* EGMenu::BeginDragAndDrop( EGUiWidget* SourceWidget )
{
	unused( SourceWidget );
	assert( m_DragAndDropWidget ); // Cannot drag and drop unless there is a widget to represent it
	if( m_DragAndDropWidget )
	{
		BeginMouseCapture( m_DragAndDropWidget );
		m_DragAndDropWidget->OnBeginDrag();
		OnDragAndDropStarted( SourceWidget , m_DragAndDropWidget );
	}
	return m_DragAndDropWidget;
}

void EGMenu::EndDragAndDrop( EGUiWidget* DroppedOntoWidget , const eg_vec2& WidgetHitPoint  )
{
	assert( m_CaptureWidget == m_DragAndDropWidget );
	if( m_DragAndDropWidget )
	{
		EndMouseCapture();
		m_DragAndDropWidget->OnDrop();
		OnDragAndDropEnded( DroppedOntoWidget , WidgetHitPoint );
	}
}

eg_bool EGMenu::IsIdValidMenu( eg_string_crc MenuId )
{
	eg_bool bIsValid = true;
	const EGUiLayout* Layout = EGUiLayoutMgr::Get()->GetLayout( MenuId );
	EGClass* Class = Layout ? Layout->m_ClassName.Class : nullptr;
	bIsValid = Layout && Class && Class->IsA( &EGMenu::GetStaticClass() );
	return bIsValid;
}

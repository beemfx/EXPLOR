// (c) 2017 Beem Media

#pragma once

#include "EGDelegate.h"
#include "EGRendererTypes.h"
#include "EGUiTypes.reflection.h"

class EGUiWidget;
class EGUiGridWidget;
class EGUiGridWidget2;

egreflect enum class eg_grid_wrap_t
{
	NO_WRAP,
	WRAP,
};

egreflect enum class eg_widget_adjacency
{
	UP ,
	DOWN ,
	LEFT,
	RIGHT,
};

egreflect enum class eg_camera_t_ed
{
	ORTHOGRAPHIC,
	PERSPECTIVE,
};

egreflect enum class eg_widget_t
{
	NONE,
	BUTTON,
	GRID,
	SCROLLBAR,
};

egreflect enum class eg_text_context
{
	None,
	ThisClass,
	Game,
};

egreflect struct egUiWidgetAdjacency
{
	egprop eg_string_crc Up    = CT_Clear;
	egprop eg_string_crc Down  = CT_Clear;
	egprop eg_string_crc Left  = CT_Clear;
	egprop eg_string_crc Right = CT_Clear;
};

egreflect struct egPropMaterial
{
	egprop eg_color32 Diffuse  = eg_color32::White;  
	egprop eg_color32 Ambient  = eg_color32::Black;  
	egprop eg_color32 Specular = eg_color32::Black; 
	egprop eg_color32 Emissive = eg_color32::Black; 
	egprop eg_real    Power    = 1.f; 

public:

	void operator = ( const EGMaterial& In )
	{
		Diffuse  = eg_color32(In.Diffuse);
		Ambient  = eg_color32(In.Ambient);
		Specular = eg_color32(In.Specular);
		Emissive = eg_color32(In.Emissive);
		Power = In.Power;
	}

	operator EGMaterial() const
	{
		EGMaterial Out;
		Out.Diffuse  = eg_color(Diffuse);
		Out.Ambient  = eg_color(Ambient);
		Out.Specular = eg_color(Specular);
		Out.Emissive = eg_color(Emissive);
		Out.Power = Power;
		return Out;
	}
};

enum class eg_widget_event_t
{
	Unknown,
	UpdateGridItem,
	ItemClicked,
	FocusGained,
	FocusLost,
};

struct egUIWidgetEventInfo
{
	eg_widget_event_t Type;
	EGUiWidget*       Widget;
	EGUiGridWidget*   GridWidgetOwner;
	EGUiGridWidget2*  GridWidget2Owner;
	eg_string_crc     WidgetId;
	eg_uint           GridIndex;
	eg_vec2           HitPoint; // Normalized across the widget with <0,0> being the lower left and <1,1> being the upper right.
	eg_bool           bSelected:1;
	eg_bool           bNewSelection:1;
	eg_bool           bIsGrid:1;
	eg_bool           bIsButton:1;
	eg_bool           bFromMouse:1;

	egUIWidgetEventInfo( eg_widget_event_t InType )
	: Type( InType )
	, WidgetId( CT_Clear )
	, Widget( nullptr )
	, GridWidgetOwner( nullptr )
	, GridIndex( 0 )
	, bSelected( false )
	, bNewSelection( false )
	, bIsGrid( false )
	, bIsButton( false )
	, HitPoint( CT_Clear )
	, bFromMouse(false)
	{

	}

	eg_bool IsNewlySelected() const { assert( Type == eg_widget_event_t::UpdateGridItem ); return bSelected && bNewSelection; }
	eg_bool IsNewlyDeselected() const { assert( Type == eg_widget_event_t::UpdateGridItem ); return !bSelected && bNewSelection; }
};

typedef EGDelegate<void,const egUIWidgetEventInfo&> EGUIWidgetEventDelegate;

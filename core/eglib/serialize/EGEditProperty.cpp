// (c) 2017 Beem Media

#include "EGEditProperty.h"

#include "EGWndPropControl.h"
#include "EGWndPropControl_Text.h"
#include "EGWndPropControl_Vector.h"
#include "EGWndPropControl_Rot.h"
#include "EGWndPropControl_Anchor.h"
#include "EGWndPropControl_Bool.h"
#include "EGWndPropControl_Color.h"
#include "EGWndPropControl_Transform.h"
#include "EGWndPropControl_ComboBox.h"
#include "EGWndPropControl_Containers.h"
#include "EGWndPropControlTypes.h"

class EGWndPropControl* EGEditProperty_CreateControlForType( class IWndPropControlOwner* Owner , eg_edit_ctrl_t Type , const struct egPropControlInfo& ControlInfo )
{
	switch( Type )
	{
		case eg_edit_ctrl_t::Unk: return new EGWndPropControl_Unk( Owner , ControlInfo );
		case eg_edit_ctrl_t::BigText: return new EGWndPropControl_Text( Owner , ControlInfo , EGWndPropControl_Text::eg_t::TEXT_BLOCK );
		case eg_edit_ctrl_t::StaticText: return new EGWndPropControl_Text( Owner , ControlInfo , EGWndPropControl_Text::eg_t::STATIC_TEXT );
		case eg_edit_ctrl_t::Text: return new EGWndPropControl_Text( Owner , ControlInfo , EGWndPropControl_Text::eg_t::TEXT );
		case eg_edit_ctrl_t::Vec2: return new EGWndPropControl_Vector( Owner , ControlInfo , 2 , egwnd_prop_control_vector_t::Real );
		case eg_edit_ctrl_t::Vec3: return new EGWndPropControl_Vector( Owner , ControlInfo , 3 , egwnd_prop_control_vector_t::Real );
		case eg_edit_ctrl_t::Vec4: return new EGWndPropControl_Vector( Owner , ControlInfo , 4 , egwnd_prop_control_vector_t::Real );
		case eg_edit_ctrl_t::IntVec2: return new EGWndPropControl_Vector( Owner , ControlInfo , 2 , egwnd_prop_control_vector_t::Int );
		case eg_edit_ctrl_t::IntVec3: return new EGWndPropControl_Vector( Owner , ControlInfo , 3 , egwnd_prop_control_vector_t::Int );
		case eg_edit_ctrl_t::IntVec4: return new EGWndPropControl_Vector( Owner , ControlInfo , 4 , egwnd_prop_control_vector_t::Int );
		case eg_edit_ctrl_t::Rot: return new EGWndPropControl_Rot( Owner , ControlInfo );
		case eg_edit_ctrl_t::Anchor: return new EGWndPropControl_Anchor( Owner , ControlInfo );
		case eg_edit_ctrl_t::Bool: return new EGWndPropControl_Bool( Owner , ControlInfo );
		case eg_edit_ctrl_t::Int: return new EGWndPropControl_Text( Owner , ControlInfo , EGWndPropControl_Text::eg_t::INT );
		case eg_edit_ctrl_t::Real: return new EGWndPropControl_Text( Owner , ControlInfo , EGWndPropControl_Text::eg_t::REAL );
		case eg_edit_ctrl_t::Angle: return new EGWndPropControl_Text( Owner , ControlInfo , EGWndPropControl_Text::eg_t::ANGLE );
		case eg_edit_ctrl_t::Color: return new EGWndPropControl_Color( Owner , ControlInfo );
		case eg_edit_ctrl_t::Transform: return new EGWndPropControl_Transform( Owner , ControlInfo );
		case eg_edit_ctrl_t::Label: return new EGWndPropControl_Label( Owner , ControlInfo );
		case eg_edit_ctrl_t::Enum: return new EGWndPropControl_ComboBox( Owner , ControlInfo );
		case eg_edit_ctrl_t::Combo: return new EGWndPropControl_ComboBox( Owner , ControlInfo );
		case eg_edit_ctrl_t::Array: return new EGWndPropControl_Array( Owner , ControlInfo );
		case eg_edit_ctrl_t::Struct: return new EGWndPropControl_Struct( Owner , ControlInfo );
		case eg_edit_ctrl_t::BrowseSave: return new EGWndPropControl_Filename( Owner , ControlInfo , EGWndPropControl_Filename::eg_t::Save , "" );
		case eg_edit_ctrl_t::BrowseOpen: return new EGWndPropControl_Filename( Owner , ControlInfo , EGWndPropControl_Filename::eg_t::Open , "" );
		case eg_edit_ctrl_t::BrowseDir: return new EGWndPropControl_Filename( Owner , ControlInfo , EGWndPropControl_Filename::eg_t::BrowseDir , "" );
		case eg_edit_ctrl_t::Button: return new EGWndPropControl_Button( Owner , ControlInfo );
	}

	return new EGWndPropControl_Label( Owner , ControlInfo );
}

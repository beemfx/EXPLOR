// (c) 2020 Beem Media. All Rights Reserved.

#include "EGTimelineTypes.h"

eg_tween_animation_t egTimelineAction::TweenTypeParm( eg_size_t Index ) const
{
	return EGString_EqualsI( FnCall.Parms[Index] , "CUBIC" ) ? eg_tween_animation_t::Cubic : eg_tween_animation_t::Linear;
}

// (c) Beem Media - Generated Code Definition for "EGEngineSerializeTypes.h"

#if defined( EGRFL_INCLUDE_SOURCE_HEADER )
#include "EGEngineSerializeTypes.h"
#include "EGEngineSerializeTypes.reflection.h"
#endif // defined( EGRFL_INCLUDE_SOURCE_HEADER )

EGRFL_IMPL_ENUM_BEGIN( eg_light_t )
	EGRFL_IMPL_ENUM_ITEM( eg_light_t , Point )
	EGRFL_IMPL_ENUM_ITEM( eg_light_t , Direction )
EGRFL_IMPL_ENUM_END( eg_light_t )

EGRFL_IMPL_STRUCT_BEGIN( eg_light_ed )
	EGRFL_IMPL_STRUCT_ITEM( eg_light_ed , eg_light_t , Type )
	EGRFL_IMPL_STRUCT_ITEM( eg_light_ed , eg_vec3 , Direction )
	EGRFL_IMPL_STRUCT_ITEM( eg_light_ed , eg_vec3 , Position )
	EGRFL_IMPL_STRUCT_ITEM( eg_light_ed , eg_color32 , Color )
	EGRFL_IMPL_STRUCT_ITEM( eg_light_ed , eg_real , Range )
	EGRFL_IMPL_STRUCT_ITEM( eg_light_ed , eg_real , FallofRadius )
	EGRFL_IMPL_STRUCT_ITEM( eg_light_ed , eg_real , Intensity )
EGRFL_IMPL_STRUCT_END( eg_light_ed )

EGRFL_IMPL_STRUCT_BEGIN( eg_aabb_ed )
	EGRFL_IMPL_STRUCT_ITEM( eg_aabb_ed , eg_vec3 , Min )
	EGRFL_IMPL_STRUCT_ITEM( eg_aabb_ed , eg_vec3 , Max )
EGRFL_IMPL_STRUCT_END( eg_aabb_ed )


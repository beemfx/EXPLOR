// (c) 2017 Beem Media

#include "EGEntDefTypes.h"

void egEntDefEdConfig::RefreshVisibleProperties( egRflEditor& ThisEditor )
{
	assert( ThisEditor.GetData() == this );

	auto SetPropEditable = [this, &ThisEditor]( eg_cpstr Name, eg_bool bValue ) -> void
	{
		egRflEditor* VarEd = ThisEditor.GetChildPtr( Name );
		if( VarEd )
		{
			VarEd->SetEditable( bValue );
		}
	};

	SetPropEditable( "CameraPos", false );
	SetPropEditable( "CameraYaw", false );
	SetPropEditable( "CameraPitch", false );
	SetPropEditable( "HasCameraPose", false );

	egRflEditor* LightsEd = ThisEditor.GetChildPtr( "Lights" );
	if( LightsEd && LightsEd->GetNumChildren() == Lights.Len() )
	{
		for( eg_size_t i = 0; i < Lights.Len(); i++ )
		{
			egRflEditor* LightEd = LightsEd->GetArrayChildPtr( i );
			if( LightEd )
			{
				Lights[i].RefreshVisibleProperties( *LightEd );
			}
		}
	}
}

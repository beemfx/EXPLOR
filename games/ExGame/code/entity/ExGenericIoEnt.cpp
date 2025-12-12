// (c) 2020 Beem Media. All Rights Reserved.

#include "ExGenericIoEnt.h"
#include "EGComponent.h"

EG_CLASS_DECL( ExGenericIoEnt )

void ExGenericIoEnt::ParseInitString( const eg_d_string& InitString )
{
	EGParse_ProcessFnCallScript( *InitString , InitString.Len() , [this]( const egParseFuncInfo& InfoStr )->void
	{
		if( EGString_StrLen(InfoStr.SystemName) == 0 )
		{
			if( EGString_EqualsI( InfoStr.FunctionName , "SetScript" ) && InfoStr.NumParms >= 1 )
			{
				EGArray<eg_d_string> Script = EGString_Split<eg_d_string>( InfoStr.Parms[0] , '.' , 2 );

				m_ScriptId = Script.IsValidIndex(0) ? eg_string_crc(*Script[0]) : CT_Clear;
				m_ScriptEntry = Script.IsValidIndex(1) ? eg_string_crc(*Script[1]) : CT_Clear;
			}
			else if( EGString_EqualsI( InfoStr.FunctionName , "SetControlVar" ) && InfoStr.NumParms >= 1 )
			{
				m_ControlVar = eg_string_crc(InfoStr.Parms[0]);
			}
		}
		else
		{
			EGComponent* Comp = GetComponentById<EGComponent>( eg_string_crc(InfoStr.SystemName) );
			if( Comp )
			{
				egTimelineAction Action;
				Action.FnCall = InfoStr;
				Comp->ScriptExec( Action );
			}
		}
	} );
}

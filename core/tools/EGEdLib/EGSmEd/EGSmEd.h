// (c) 2017 Beem Media

#pragma once

#include "EGSmFile2.h"

class EGSmEdPropPanel;
class EGSmEdScriptPanel;

extern EGSmFile2 EGSmEd_SmFile;
static inline EGSmFile2& EGSmEd_GetFile(){ return EGSmEd_SmFile; }

class ISmEdApp
{
public:

	static const eg_size_t NO_FOCUS_NODE = static_cast<eg_size_t>(-1);
	static const eg_uint NO_FOCUS_CHOICE = static_cast<eg_uint>(-1);
	
	struct egFocus
	{
		eg_size_t StateIndex;
		eg_uint   ChoiceIndex;

		egFocus( eg_ctor_t Ct )
		{
			if( Ct == CT_Default || Ct == CT_Clear )
			{
				StateIndex = NO_FOCUS_NODE;
				ChoiceIndex = NO_FOCUS_CHOICE;
			}
		}

		eg_bool operator==(const egFocus& rhs ) const
		{
			return StateIndex == rhs.StateIndex && ChoiceIndex == rhs.ChoiceIndex;
		}

		eg_bool operator!=(const egFocus& rhs ) const
		{
			return !(*this == rhs);
		}
	};

public:

	virtual void UpdateHeaderText() = 0;
	virtual void SetDirty() = 0;
	virtual EGSmEdPropPanel* GetPropPanel() = 0;
	virtual EGSmEdPropPanel* GetSettingsPanel() = 0;
	virtual EGSmEdScriptPanel* GetScriptPanel() = 0;
	virtual void SetFocusedNode( const egFocus& NewFocus ) = 0;
	virtual egFocus GetFocusedNode() const = 0;
	virtual void RefreshProperties() = 0;
};

int EGSmEd_Run( eg_cpstr16 InitialFilename , const struct egSdkEngineInitParms& EngineParms );
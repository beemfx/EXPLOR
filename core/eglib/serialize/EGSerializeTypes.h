// (c) 2017 Beem Media

#pragma once

#include "EGReflection.h"
#include "EGEditControlType.h"

struct egEditProperty
{
	egRflEditor*     PropEditor = nullptr;
	egRflEditor*     ParentEditor = nullptr;
	eg_string_small  EditId = CT_Clear; // Should be full path to variable (i.e. "Base.Vallue.Index0").
	eg_string_small  DisplayName = CT_Clear;
	eg_int           Depth = 0;
	eg_int           ParentIndex = 0;

	egEditProperty() = default;

	egEditProperty( egRflEditor* InEditor , egRflEditor* InParentEditor , eg_int InDepth )
	: PropEditor( InEditor )
	, ParentEditor( InParentEditor )
	, DisplayName( InEditor ? InEditor->GetDisplayName() : "-null-" )
	, Depth( InDepth )
	{

	}
};

typedef EGArray<egEditProperty> EGEditPropArray;
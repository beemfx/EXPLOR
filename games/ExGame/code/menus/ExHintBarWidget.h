// (c) 2016 Beem Media
#pragma once

#include "EGUiTextWidget.h"
#include "EGInputTypes.h"

class ExHintBarWidget : public EGUiTextWidget
{
	EG_CLASS_BODY( ExHintBarWidget , EGUiTextWidget )

private:

	struct exHintInfo
	{
		eg_cmd_t Cmd;
		eg_loc_text Text;
	};

	EGArray<exHintInfo> m_Hints;

public:

	void ClearHints();
	void AddHint( eg_cmd_t Cmd , const eg_loc_text& Text );
	eg_bool HasHints() const { return m_Hints.Len() > 0; }

private:

	void UpdateText();

};
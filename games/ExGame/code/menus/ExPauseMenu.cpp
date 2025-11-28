// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMainAndPauseMenuBase.h"

class ExPauseMenu: public ExMainAndPauseMenuBase
{
	EG_CLASS_BODY( ExPauseMenu , ExMainAndPauseMenuBase )

public:

	ExPauseMenu()
	{
		m_IsPause = true;
	}
};

EG_CLASS_DECL( ExPauseMenu )

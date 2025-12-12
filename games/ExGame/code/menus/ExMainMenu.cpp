// (c) 2016 Beem Media

#include "ExMainAndPauseMenuBase.h"

class ExMainMenu: public ExMainAndPauseMenuBase
{
	EG_CLASS_BODY( ExMainMenu , ExMainAndPauseMenuBase )

public:

	ExMainMenu()
	{
		m_IsPause = false;
	}
};

EG_CLASS_DECL( ExMainMenu )

// (c) 2017 Beem Media

#include "ExEnt.h"

class ExTorch : public ExEnt
{
	EG_CLASS_BODY( ExTorch , ExEnt )

private:

public:

	virtual void OnCreate( const egEntCreateParms& Parms ) override final
	{
		Super::OnCreate( Parms );

		SetActive( false );
	}
};

EG_CLASS_DECL( ExTorch )
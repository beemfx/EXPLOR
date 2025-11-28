// (c) 2018 Beem Media

#include "EGClassName.h"

void eg_class_name::GetComboChoices( EGArray<eg_d_string>& Out ) const
{
	Out.Append( "" );
	EGArray<EGClass*> Classes;
	EGClass::FindAllClassesOfType( BaseClass , Classes );
	for( const EGClass* Class : Classes )
	{
		Out.Append( Class->GetName() );
	}
}

// (c) 2020 Beem Media. All Rights Reserved.

#include "ExCharacterClass.h"

const exCharacterClass& exCharacterClassGlobals::GetClass( const eg_string_crc& ClassId ) const
{
	if( ClassId.IsNotNull() )
	{
		for( const exCharacterClass& CompareClass : Classes )
		{
			if( CompareClass.ClassCrcId == ClassId )
			{
				return CompareClass;
			}
		}
	}

	return DefaultClass;
}

void exCharacterClassGlobals::PostLoad()
{
	for( exCharacterClass& Class : Classes )
	{
		Class.SortBalanceData();
	}
}

void exCharacterClass::SortBalanceData()
{
	BalanceData.Sort( []( const exCharacterClassBalanceData& Left , const exCharacterClassBalanceData& Right ) -> eg_bool
	{
		return Left.Level < Right.Level;
	} );
}

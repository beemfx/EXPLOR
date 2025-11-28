
#pragma once

#include "EGList.h"

void AlwaysLoaded_Init();
void AlwaysLoaded_Deinit();

void* AlwaysLoaded_GetFile( eg_cpstr Filename , eg_size_t* SizeOut );

void AlwaysLoaded_GetFileList( class EGAlwaysLoadedFilenameList* List , eg_cpstr Ext );

struct egAlwaysLoadedFilename: public IListable
{
	eg_string Filename;
};

class EGAlwaysLoadedFilenameList: public EGList<egAlwaysLoadedFilename>
{
public:
	EGAlwaysLoadedFilenameList(): EGList( EGAlwaysLoadedFilenameList::DEFAULT_ID )
	{ 
	}

	~EGAlwaysLoadedFilenameList()
	{
		ClearFilenames();
	}

	void InsertFilename( eg_cpstr Filename )
	{
		egAlwaysLoadedFilename* NewItem = new egAlwaysLoadedFilename;
		if( NewItem )
		{
			NewItem->Filename = Filename;
			InsertLast( NewItem );
		}
		else
		{
			assert( false ); //Out of memory?
		}
	}

	void ClearFilenames()
	{
		while( HasItems() )
		{
			egAlwaysLoadedFilename* Item = GetFirst();
			Remove( Item );
			delete Item;
		}
	}
};
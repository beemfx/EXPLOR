// (c) 2018 Beem Media

#pragma once

#include "EGTerrain2.h"
#include "EGRendererTypes.h"
#include "EGLoader_Loadable.h"
#include "EGDelegate.h"

class EGGameTerrain2 : public EGTerrain2 , public ILoadable
{
public:

	EGMCDelegate<EGTerrain2*> TerrainLoadedDelegate;

public:

	virtual void DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size) override;
	virtual void OnLoadComplete(eg_cpstr strFile) override;

	void Load( eg_cpstr sFile , eg_bool bServer );
	void LoadOnThisThread(eg_cpstr strFile);

	void Unload();
};

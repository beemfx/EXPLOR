// (c) 2011 Beem Media

#pragma once

#include "EGSkelBase.h"
#include "EGDelegate.h"
#include "EGLoader_Loadable.h"

class EGSkel : public EGSkelBase , private ILoadable
{
public:

	EGSimpleMCDelegate OnLoaded;

public:

	EGSkel( eg_cpstr Filename );
	~EGSkel();

private:

	LOAD_S m_LoadState;
	
private:

	void LoadFromBinaryOnThread(eg_cpstr strFile);
	void Unload();
	void DeallocateMemory();

	//ILOADABLE INTERFACE:
	virtual void DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size) override;
	virtual void OnLoadComplete(eg_cpstr strFile) override;

};

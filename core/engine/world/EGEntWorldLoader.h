// (c) 2018 Beem Media

#pragma once

#include "EGWeakPtr.h"
#include "EGEntTypes.h"
#include "EGLoader_Loadable.h"

class EGEntWorld;
class EGWorldFile;

class EGEntWorldLoader : public EGObject , public ILoadable
{
	EG_CLASS_BODY( EGEntWorldLoader , EGObject )

private:

	EGWorldFile*          m_WorldFile = nullptr;
	EGWeakPtr<EGEntWorld> m_OwnerWorld = nullptr;
	eg_string_small       m_Filename;
	eg_bool               m_bIsLoading = false;
	eg_bool               m_bCanceledLoad = false;
	eg_bool               m_bIsLoaded = false;

public:

	void InitWorldLoader( eg_cpstr Filename , EGEntWorld* Owner );
	virtual void OnDestruct() override;
	eg_bool IsLoaded() const { return m_bIsLoaded && nullptr != m_WorldFile; }
	eg_bool IsLoading() const { return m_bIsLoading; }
	eg_cpstr GetFilename() const { return m_Filename; }
	const EGWorldFile* GetWorldFile() { return m_WorldFile; }

private:

	virtual void DoLoad( eg_cpstr strFile , const eg_byte*const  pMem , const eg_size_t Size ) override;
	virtual void OnLoadComplete( eg_cpstr strFile ) override;
};

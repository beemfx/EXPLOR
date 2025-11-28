// (c) 2014 Beem Media

#pragma once

#include "EGLoader_Loadable.h"

class EGMap_PhysLoader: public ILoadable
{
public:
	EGMap_PhysLoader(): m_Chunk(nullptr), m_AlignedChunk(nullptr), m_Size(0), m_State( STATUS_NOT_LOADED ){}
	~EGMap_PhysLoader(){ Clear(); }

	void BeginLoad( eg_cpstr strFile );
	void LoadNow( eg_cpstr strFile );
	eg_bool IsLoaded()const{ return 0 != m_Size; }
	const eg_byte* GetChunk()const{ return m_AlignedChunk; }
	eg_size_t GetSize()const{ return m_Size; }
	void Clear();
private:
	eg_byte*  m_Chunk;
	eg_byte*  m_AlignedChunk;
	eg_size_t m_Size;
	enum STATUS
	{
		STATUS_NOT_LOADED,
		STATUS_LOADING,
		STATUS_LOADED,
	} m_State;
private:
	//LOADING THREAD FUNCTIONALITY:
	virtual void DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size) override;
	virtual void OnLoadComplete(eg_cpstr strFile) override;
};
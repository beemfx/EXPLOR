#pragma once

#include "EGLoader_Loadable.h"
#include "EGDirectXAPI.h"

class EGD11R_Texture: public ILoadable
{
public:
	eg_string                  Filename;
	ID3D11Resource*            Texture;
	ID3D11ShaderResourceView*  View;
	eg_uint                    RefCount;

	EGD11R_Texture(): Filename(""), Texture(nullptr), View(nullptr), RefCount(0), TempLoadData(nullptr), TempLoadDataSize(0), m_LoadState(LOAD_NOT_LOADED){ }
	~EGD11R_Texture(){ assert( 0 == RefCount ); assert( nullptr == TempLoadData ); assert( 0 == TempLoadDataSize); }

	void AddRef();
	void Release();

	static void InitDevice( ID3D11Device* Device ){ s_Device = Device; }
private:
	LOAD_S    m_LoadState;
	void*     TempLoadData;
	eg_size_t TempLoadDataSize;
	static ID3D11Device* s_Device;
private:
	void CreateTextureFromData( ID3D11Device* Device , const void* Data , eg_size_t Size );
	virtual void DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size) override;
	virtual void OnLoadComplete(eg_cpstr strFile) override;
};
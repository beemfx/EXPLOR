#pragma once
#include "EGD11R_Texture.h"
#include "EGItemMap.h"
#include "EGList.h"
#include "EGEngineTemplates.h"

class EGD11R_ShaderMgr
{
public:
	//Init and deinit should only be called from the render thread.
	void Init( ID3D11Device* Device );
	void Deinit( void );
	//The getters are called from other threads (mainly during 
	//EGRenderer::Get().CreateMaterial), but the pointers will only actually be passed
	// to the video card on the render thread.
	ID3D11VertexShader* GetVertexShader( eg_cpstr Filename )const;
	ID3D11PixelShader*  GetPixelShader( eg_cpstr Filename )const;
	EGD11R_Texture*     GetTexture( eg_cpstr Filename )const;

private:
	EGSysMemItemMap<ID3D11VertexShader*> m_VsMap;
	EGSysMemItemMap<ID3D11PixelShader*>  m_PsMap;
	EGSysMemItemMap<EGD11R_Texture*>     m_TxMap;
	ID3D11Device*     m_Device;
	eg_size_t         m_VsBytes;
	eg_size_t         m_PsBytes;
	eg_size_t         m_TxBytes;
	EGD11R_Texture*   m_TxMasterList;
	eg_size_t         m_TxMasterListSize;
	eg_uint           m_TxCount;
	eg_bool           m_TxLoadAllAlways;
private:
	void InsertVsFile( eg_cpstr Filename );
	void InsertPsFile( eg_cpstr Filename );
	void InsertTxFile( eg_cpstr Filename );

	static eg_string_crc FilenameToCrcId( eg_cpstr Filename , eg_uint ClampSize );
};
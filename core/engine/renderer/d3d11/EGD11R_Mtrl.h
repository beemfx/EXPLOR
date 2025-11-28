/******************************************************************************
File: Material.h
Class: EGD3D9Material
Purpose: The material class as used by the video class, all it really is
is references to a texture and a shader, the actual structure used by the
game is an egv_material object (which is a dword).

(c) 2011 Beem Software
******************************************************************************/
#pragma once

#include "EGRendererTypes.h"
#include "EGD11R_Texture.h"

class EGD11R_Mtrl
{
public:

	EGD11R_Mtrl();
	~EGD11R_Mtrl();

public:

	//We keep a reference to both the resource index, and the actual
	//resource. The index is so we can delete the resource. The actual
	//resource is so we can use it.
	static const eg_uint MAX_TEX  = EGMaterialDef::MAX_TEX;

	EGD11R_Texture*     m_Tx[MAX_TEX];
	ID3D11VertexShader* m_Vs;
	ID3D11PixelShader*  m_Ps;
	EGMaterial          m_D3DMtrl;

	eg_bool HasShaders() const { return m_Vs && m_Ps; }
	void Begin( ID3D11DeviceContext* Context ) const;
	void End( ID3D11DeviceContext* Context ) const;
};

/******************************************************************************
File: Material.cpp
Class: EGD11R_Mtrl
Purpose: See header.

(c) 2011 Beem Software
******************************************************************************/

#include "EGD11R_Mtrl.h"

EGD11R_Mtrl::EGD11R_Mtrl()
{
	for(eg_uint i=0; i<MAX_TEX; i++)
	{
		m_Tx[i] = nullptr;
	}

	m_Ps = nullptr;
	m_Vs = nullptr;
	
	//Start out by setting the material to just all white.
	m_D3DMtrl.Diffuse.r=1.0f;
	m_D3DMtrl.Diffuse.g=1.0f;
	m_D3DMtrl.Diffuse.b=1.0f;
	m_D3DMtrl.Diffuse.a=1.0f;
	
	m_D3DMtrl.Ambient.r=1.0f;
	m_D3DMtrl.Ambient.g=1.0f;
	m_D3DMtrl.Ambient.b=1.0f;
	m_D3DMtrl.Ambient.a=1.0f;
	
	m_D3DMtrl.Specular.r=0.0f;
	m_D3DMtrl.Specular.g=0.0f;
	m_D3DMtrl.Specular.b=0.0f;
	m_D3DMtrl.Specular.a=0.0f;
	
	m_D3DMtrl.Emissive.r=0.0f;
	m_D3DMtrl.Emissive.g=0.0f;
	m_D3DMtrl.Emissive.b=0.0f;
	m_D3DMtrl.Emissive.a=0.0f;
	
	m_D3DMtrl.Power=0.0f;
}

EGD11R_Mtrl::~EGD11R_Mtrl()
{
	//The video manager handles deleting resources.
}

void EGD11R_Mtrl::Begin( ID3D11DeviceContext* Context ) const
{
	Context->VSSetShader( m_Vs , nullptr , 0 );
	Context->PSSetShader( m_Ps , nullptr , 0 );
}

void EGD11R_Mtrl::End( ID3D11DeviceContext* Context ) const
{
	Context->VSSetShader( nullptr , nullptr , 0 );
	Context->PSSetShader( nullptr , nullptr , 0 );
}

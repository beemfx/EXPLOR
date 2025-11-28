// (c) 2017 Beem Media

#pragma once

struct ID3D11Device;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
enum DXGI_FORMAT;

class EGD11R_RenderTarget : public EGObject
{
	EG_CLASS_BODY( EGD11R_RenderTarget , EGObject )

private:

	ID3D11Texture2D*          m_Texture = nullptr;
	ID3D11RenderTargetView*   m_ColorView = nullptr;
	ID3D11DepthStencilView*   m_DepthView = nullptr;
	ID3D11ShaderResourceView* m_ResourceView = nullptr;
	eg_uint                   m_Width = 0;
	eg_uint                   m_Height = 0;

public:

	void InitRenderTarget( ID3D11Device* Device , DXGI_FORMAT BufferFormat , eg_uint Width , eg_uint Height );
	void InitRenderTargetWithTexture( ID3D11Device* Device , ID3D11Texture2D* Texture , eg_uint Width , eg_uint Height );
	void InitDepthStencil( ID3D11Device* Device , eg_uint Width , eg_uint Height );
	virtual void OnDestruct() override final;

	eg_uint GetWidth() const { return m_Width; }
	eg_uint GetHeight() const { return m_Height; }
	ID3D11RenderTargetView* GetRenderTargetView() const { return m_ColorView; }
	ID3D11DepthStencilView* GetDepthStencilView() const { return m_DepthView; }
	ID3D11ShaderResourceView* GetResourceView() const { return m_ResourceView; }
};

// (c) 2017 Beem Media

#include "EGD11R_RenderTarget.h"
#include "EGD11R_Base.h"

EG_CLASS_DECL( EGD11R_RenderTarget )

void EGD11R_RenderTarget::InitRenderTarget( struct ID3D11Device* Device , enum DXGI_FORMAT BufferFormat , eg_uint Width, eg_uint Height )
{
	HRESULT Res = S_OK;

	m_Width = Width;
	m_Height = Height;

	D3D11_TEXTURE2D_DESC TxDesc;
	zero( &TxDesc );
	TxDesc.ArraySize = 1;
	TxDesc.BindFlags = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
	TxDesc.CPUAccessFlags = 0; // No CPU access required.
	TxDesc.Format = BufferFormat;
	TxDesc.Width = m_Width;
	TxDesc.Height = m_Height;
	TxDesc.MipLevels = 1;
	TxDesc.SampleDesc.Count = 1;
	TxDesc.SampleDesc.Quality = 0;
	TxDesc.Usage = D3D11_USAGE_DEFAULT;

	Res = Device->CreateTexture2D( &TxDesc , nullptr , &m_Texture );
	assert(SUCCEEDED(Res));
	Res = Device->CreateRenderTargetView( m_Texture , nullptr , &m_ColorView );
	assert(SUCCEEDED(Res));
	Res = Device->CreateShaderResourceView( m_Texture , nullptr , &m_ResourceView );
	assert(SUCCEEDED(Res));
}

void EGD11R_RenderTarget::InitRenderTargetWithTexture( ID3D11Device* Device, ID3D11Texture2D* Texture , eg_uint Width, eg_uint Height )
{
	HRESULT Res = S_OK;

	m_Width = Width;
	m_Height = Height;

	m_Texture = Texture;
	m_Texture->AddRef();
	Res = Device->CreateRenderTargetView( m_Texture , nullptr , &m_ColorView );
	assert(SUCCEEDED(Res));
}

void EGD11R_RenderTarget::InitDepthStencil( ID3D11Device* Device , eg_uint Width, eg_uint Height )
{
	m_Width = Width;
	m_Height = Height;

	D3D11_TEXTURE2D_DESC DsDesc;
	zero( &DsDesc );
	DsDesc.Width = Width;
	DsDesc.Height = Height;
	DsDesc.MipLevels = 1;
	DsDesc.ArraySize = 1;
	DsDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	DsDesc.SampleDesc.Count = 1;
	DsDesc.SampleDesc.Quality = 0;
	DsDesc.Usage = D3D11_USAGE_DEFAULT;
	DsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE;
	DsDesc.CPUAccessFlags = 0;
	DsDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC DsvDesc;
	zero( &DsvDesc );
	DsvDesc.Flags = 0;
	DsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT ;
	DsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DsvDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc;
	zero( &SrvDesc );
	SrvDesc.Format                    = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	SrvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	SrvDesc.Texture2D.MostDetailedMip = 0;
	SrvDesc.Texture2D.MipLevels       = 1;//-1;


	HRESULT Res = S_OK;

	Res = Device->CreateTexture2D(&DsDesc, 0, &m_Texture);
	assert( SUCCEEDED(Res) );
	Res = Device->CreateDepthStencilView(m_Texture, &DsvDesc, &m_DepthView);
	assert( SUCCEEDED(Res) );
	Res = Device->CreateShaderResourceView(m_Texture, &SrvDesc, &m_ResourceView);
	assert( SUCCEEDED(Res) );
}

void EGD11R_RenderTarget::OnDestruct()
{
	EG_SafeRelease( m_ResourceView );
	EG_SafeRelease( m_ColorView );
	EG_SafeRelease( m_DepthView );
	EG_SafeRelease( m_Texture );
}

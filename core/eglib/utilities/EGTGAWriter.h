///////////////////////////////////////////////////////////////////////////////
// EGTgaWriter
// (c) 2015 Beem Media
///////////////////////////////////////////////////////////////////////////////
#pragma once

class EGTgaWriter
{
public:
/////////////////////
// Construct/Destruct
/////////////////////
	EGTgaWriter(eg_uint Width, eg_uint Height);

	~EGTgaWriter();
///////////////////
// Public Interface
///////////////////
public:
	#pragma pack(push, 1)
	struct egPixel
	{
		eg_byte b;
		eg_byte g;
		eg_byte r;
		eg_byte a;

		egPixel() = default;

		egPixel( const eg_color& rhs )
		{
			*this = rhs;
		}

		egPixel( const eg_color32& rhs )
		{
			*this = rhs;
		}

		egPixel& operator = ( const eg_color& rhs )
		{
			eg_color32 As32( rhs );
			b = As32.B;
			g = As32.G;
			r = As32.R;
			a = As32.A;
			return *this;
		}

		egPixel& operator = ( const eg_color32& rhs )
		{
			b = rhs.B;
			g = rhs.G;
			r = rhs.R;
			a = rhs.A;
			return *this;
		}
	};
	#pragma pack(pop)
public:
	void WritePixel(eg_uint x, eg_uint y, const egPixel& Color);
	void Clear( const egPixel& Color )
	{
		for(eg_uint x = 0; x<GetWidth(); x++)
		{
			for(eg_uint y=0; y<GetHeight(); y++)
			{
				WritePixel(x, y, Color);
			}
		}
	}
	const eg_byte* GetTGA()const{	return m_pChunk; }
	eg_uint GetSize()const{	return m_FileSize; }
	eg_uint GetWidth()const{ return m_Width; }
	eg_uint GetHeight()const{ return m_Height; }

private:
	static const eg_uint HEADER_SIZE = 18;

	const eg_uint m_Width;
	const eg_uint m_Height;
	const eg_uint m_FileSize;
	eg_byte*      m_pChunk;
	eg_byte*      m_pHeader;
	egPixel*       m_pData;
};
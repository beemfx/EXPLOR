// (c) 2019 Beem Media

#include "EGBase64.h"
#include "EGFileData.h"

static eg_char8 EGBase64_6BitToChar( eg_byte b );
static void EGBase64_Byte3ToChar4( const eg_byte in[] , eg_size_t nCount , eg_char8 out[] );
static eg_byte EGBase64_CharTo6Bit( eg_char8 c );
static eg_size_t EGBase64_Char4ToByte3( const eg_char in[] , eg_byte out[] );

eg_bool EGBase64_Encode( const void* DataIn , eg_size_t DataInSize , eg_d_string8& StringOut )
{
	eg_byte DecBytes[3] = { 0, 0, 0 };
	eg_size_t DecBytesCount = 0;
	eg_char8 EncChars[5] = { 0 , 0 , 0 , 0 };

	const eg_byte* DataInAsByte = reinterpret_cast<const eg_byte*>(DataIn);

	EGArray<eg_char8> OutBuffer( eg_mem_pool::DefaultHi );
	OutBuffer.Reserve( DataInSize * 2 );

	const eg_size_t DataSize = DataInSize;
	for( eg_size_t i = 0; i < DataSize; i++ )
	{
		DecBytes[DecBytesCount++] = DataInAsByte[i];
		if( 3 == DecBytesCount )
		{
			EGBase64_Byte3ToChar4( DecBytes, 3, EncChars );
			OutBuffer.Append( EncChars, 4 );
			DecBytesCount = 0;
		}
	}

	// Encode any remaining bytes.
	if( DecBytesCount )
	{
		EGBase64_Byte3ToChar4( DecBytes, DecBytesCount, EncChars );
		OutBuffer.Append( EncChars, 4 );
	}

	OutBuffer.Append( '\0' );
	StringOut = OutBuffer.GetArray();
	return true;
}

eg_size_t EGBase64_Decode( eg_cpstr8 StringIn , void* DataOut , eg_size_t DataOutSize , eg_bool bStrict /*= false */ )
{
	eg_size_t BytesDecoded = 0;
	eg_size_t nFound = 0;
	eg_char8 conv[4];

	eg_byte* DataOutWriter = reinterpret_cast<eg_byte*>(DataOut);
	eg_size_t DataOutWritePos = 0;

	auto Write = [&DataOutWriter,&DataOutWritePos,&DataOutSize]( const void* WriteData , eg_size_t WriteLen ) -> void
	{
		if( (DataOutWritePos + WriteLen) <= DataOutSize )
		{
			EGMem_Copy( DataOutWriter + DataOutWritePos , WriteData , WriteLen );
			DataOutWritePos += WriteLen;
		}
	};

	const eg_size_t StrLen = EGString_StrLen( StringIn );
	for( eg_size_t StrIdx = 0; StrIdx <= StrLen; StrIdx++ )
	{
		const eg_bool bIsValidChar = EGBase64_CharTo6Bit( StringIn[StrIdx] ) != 0x80;

		if( bIsValidChar )
		{
			//When a base64 character is found, save that character.
			conv[nFound++] = StringIn[StrIdx];

			//When four characters are found they should be decoded.
			if( 4 == nFound )
			{
				eg_byte dec[3];
				const eg_size_t nCopyBytes = EGBase64_Char4ToByte3( conv, dec );
				Write( &dec , nCopyBytes );
				BytesDecoded += nCopyBytes;
				nFound = 0;
				conv[0] = conv[1] = conv[2] = conv[3] = 0;
			}
		}
		else
		{
			if( bStrict )
			{
				return false;
			}
		}
	}

	const eg_bool bHadAllData = 0 == nFound && BytesDecoded == DataOutWritePos;

	return bHadAllData ? BytesDecoded : 0;
}

eg_bool EGBase64_Encode( const EGArray<eg_byte>& DataIn, eg_d_string8& StringOut )
{
	return EGBase64_Encode( DataIn.GetArray() , DataIn.Len() , StringOut );
}

eg_bool EGBase64_Decode( const eg_d_string& StringIn, EGArray<eg_byte>& DataOut , eg_bool bStrict /*= false*/ )
{
	const eg_size_t ApproximateSize = ((StringIn.Len()/4)+1)*3;
	DataOut.Resize( ApproximateSize );
	eg_size_t BytesDecoded = EGBase64_Decode( *StringIn , DataOut.GetArray() , DataOut.Len() , bStrict );
	DataOut.Resize( BytesDecoded );
	return 0 != BytesDecoded || StringIn.Len() == 0;
}


//
// Helper functions
//

static eg_char8 EGBase64_6BitToChar( eg_byte b )
{
	if( b < 26 )
	{
		return 'A' + b;
	}
	if( b < 52 )
	{
		return 'a' + ( b - 26 );
	}
	if( b < 62 )
	{
		return '0' + ( b - 52 );
	}
	if( b == 62 )
	{
		return '+';
	}
	if( b == 63 )
	{
		return '/';
	}
	return '/';
}

static void EGBase64_Byte3ToChar4( const eg_byte in[] , eg_size_t nCount , eg_char8 out[] )
{
	eg_byte by[3] = { 0, 0, 0 };
	eg_byte ch[4] = { 0, 0, 0, 0 };

	if( nCount > 0 )
		by[0] = in[0];
	if( nCount > 1 )
		by[1] = in[1];
	if( nCount > 2 )
		by[2] = in[2];

	ch[0] = by[0] >> 2;
	ch[1] = ( ( by[0] & 0x03 ) << 4 ) | ( by[1] >> 4 );
	ch[2] = ( ( by[1] & 0x0F ) << 2 ) + ( by[2] >> 6 );
	ch[3] = by[2] & 0x3F;

	if( nCount > 0 )
	{
		out[0] = EGBase64_6BitToChar( ch[0] );
		out[1] = EGBase64_6BitToChar( ch[1] );
	}
	if( nCount > 1 )
	{
		out[2] = EGBase64_6BitToChar( ch[2] );
	}
	else
	{
		out[2] = '=';
	}
	if( nCount > 2 )
	{
		out[3] = EGBase64_6BitToChar( ch[3] );
	}
	else
	{
		out[3] = '=';
	}
}

static eg_byte EGBase64_CharTo6Bit( eg_char8 c )
{
	if( c >= 'A' && c <= 'Z' )
	{
		return c - 'A';
	}
	if( c >= 'a' && c <= 'z' )
	{
		return c - 'a' + 26;
	}
	if( c >= '0' && c <= '9' )
	{
		return c - '0' + 52;
	}
	if( c == '+' )
	{
		return 62;
	}
	if( c == '/' )
	{
		return 63;
	}
	if( c == '=' )
	{
		return 0xC0;
	}

	return 0x80;
}


static eg_size_t EGBase64_Char4ToByte3( const eg_char in[] , eg_byte out[] )
{
	eg_byte nOut = 0;

	eg_byte dec[4];
	dec[0] = EGBase64_CharTo6Bit( in[0] );
	dec[1] = EGBase64_CharTo6Bit( in[1] );
	dec[2] = EGBase64_CharTo6Bit( in[2] );
	dec[3] = EGBase64_CharTo6Bit( in[3] );

	out[0] = ( dec[0] << 2 ) | ( dec[1] >> 4 );
	nOut++;
	if( dec[2] != 0xC0 )
	{
		out[1] = ( ( dec[1] & 0x0F ) << 4 ) | ( dec[2] >> 2 );
		nOut++;
	}
	if( dec[3] != 0xC0 )
	{
		out[2] = ( ( dec[2] & 0x03 ) << 6 ) | ( dec[3] );
		nOut++;
	}

	return nOut;
}

eg_d_string8 EGBase64_Encode( const class EGFileData& DataIn )
{
	EGArray<eg_byte> TempArray;
	TempArray.Append( DataIn.GetDataAs<eg_byte>(), DataIn.GetSize() );
	return std::move( EGBase64_Encode( TempArray ) );
}

void EGBase64_Decode( const eg_d_string& StringIn, class EGFileData& DataOut )
{
	EGArray<eg_byte> TempData = EGBase64_Decode( StringIn );
	DataOut.Write( TempData.GetArray() , TempData.Len() );
}

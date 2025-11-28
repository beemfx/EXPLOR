/******************************************************************************
eg_string_base - A string that must be supplied memory in the constructor.
eg_string_fixed_size - A template of eg_string_base that holds it's own memory.
eg_string - A fixed size string 128 characters.
eg_string_small - A fixed size string 128 characters.
eg_string_big - A fixed size string 1024 characters.

Strings cannot be mem copied or serialized directly since they contain a 
pointer, so for a copy they must be copied explicitly or CopyTo should be
called to copy it to a relevant point of data. Mainly they are meant to be
stack strings quickly used and discarded, though in some cases arrays of them
are held for lookup tables, etc.

(c) 2011 Beem Media
******************************************************************************/
#pragma once

class eg_string_base
{
public:
	eg_string_base( eg_pstr StrMem , eg_uint StrMemSize , eg_ctor_t ConstructT = CT_Clear ): m_strString(StrMem), m_strStringSize(StrMemSize){ CommonConstruct(); if( CT_Clear == ConstructT ){ Clear(); }else{ UpdateLength(); } }
	~eg_string_base(){ CommonDestruct(); }

	//
	// Access functions:
	//
	operator eg_cpstr()const{ return String(); }
	eg_cpstr operator*()const{ return String(); }
	const eg_char& operator[](const eg_size_t rhs)const{ return CharAt(rhs); }

	eg_cpstr String()const{ return m_strString; }
	eg_uint  Len()const{ return m_nLength; }
	const eg_char& CharAt(const eg_size_t rhs)const{ return m_strString[rhs]; }

	void CopyTo(eg_char8* Dest, eg_size_t DestSize)const{ EGString_Copy( Dest , m_strString , DestSize ); }
	void CopyTo(eg_char16* Dest, eg_size_t DestSize)const{ EGString_Copy( Dest , m_strString , DestSize ); }

	//
	// Comparison functions:
	//
	eg_bool operator==(eg_cpstr rhs)const{ return Equals(rhs); }
	eg_bool operator==(const eg_string_base& rhs)const{ return Equals(rhs); }
	eg_bool operator!=(eg_cpstr rhs)const{ return !Equals(rhs); }
	eg_bool operator!=(const eg_string_base& rhs)const{ return !Equals(rhs); }

	friend inline eg_bool operator==(eg_cpstr lhs, const eg_string_base& rhs){ return rhs == lhs; }
	friend inline eg_bool operator!=(eg_cpstr lhs, const eg_string_base& rhs){ return rhs != lhs; }

	eg_int Compare(const eg_string_base& rhs)const{ return EGString_Compare(m_strString, rhs.m_strString); }
	eg_int Compare(eg_cpstr rhs)const{ return EGString_Compare(m_strString, rhs); }
	eg_int CompareI(const eg_string_base& rhs)const{ return EGString_CompareI(m_strString, rhs); }
	eg_int CompareI(eg_cpstr rhs)const{ return EGString_CompareI(m_strString, rhs); }
	eg_bool Equals(const eg_string_base& rhs)const { return EGString_Equals(m_strString,rhs); }
	eg_bool Equals(const eg_cpstr rhs)const  { return EGString_Equals(m_strString,rhs); }
	eg_bool EqualsI(const eg_string_base& rhs)const{ return EGString_EqualsI(m_strString,rhs); }
	eg_bool EqualsI(eg_cpstr rhs)const { return EGString_EqualsI(m_strString,rhs); }
	eg_bool Contains(eg_cpstr SubStr)const{ return EGString_Contains(m_strString, SubStr); }
	eg_bool EqualsCount(eg_cpstr rhs, eg_size_t Count)const{return EGString_EqualsCount(m_strString,rhs,Count); }
	eg_bool EqualsCount(const eg_string_base& rhs, eg_size_t Count)const{return EGString_EqualsCount(m_strString,rhs.m_strString,Count); }
	
	//
	// Parsing functions:
	//
	eg_bool IsNumber()const;
	eg_bool IsInteger()const;
	eg_real ToFloat()const{ return EGString_ToReal(m_strString); }
	eg_uint ToUInt()const{ return EGString_ToUInt(m_strString); }
	eg_bool ToBool()const;
	eg_uint ToUIntArray(eg_uint* anOut, const eg_uint nMaxNums)const;
	eg_int  ToInt()const{ return EGString_ToInt(m_strString); }
	eg_uint ToUIntFromHex()const{ return EGString_ToUIntFromHex(m_strString); }
	eg_uint ToRealArray(eg_real* Out , eg_uint MaxOut )const;

	//
	// Modification functions:
	//
	void Clear(){ m_strString[0]=0; m_nLength=0; }

	const eg_string_base& operator = (const eg_string_base& rhs){ Set( rhs ); return *this; }
	const eg_string_base& operator = (eg_cpstr8 rhs){ Set( rhs ); return *this; }
	const eg_string_base& operator = (eg_cpstr16 rhs){ Set( rhs ); return *this; }
	void Set( eg_cpstr8 Str ) { Clear(); EGString_Copy( m_strString, Str, m_strStringSize ); UpdateLength(); }
	void Set( eg_cpstr16 Str ){ Clear(); EGString_Copy( m_strString, Str, m_strStringSize ); UpdateLength(); }
	void Set( const eg_color& rhs );

	void operator += (const eg_string_base& rhs){ Append(rhs); }
	void operator += (eg_cpstr rhs){ Append(rhs); }
	void operator += (const eg_char c){ Append(c); }
	void Append(eg_cpstr str);
	void Append(const eg_string_base& str);
	void Append(const eg_char c);
	void AppendSpaces(const eg_uint count);
	
	void ClampEnd(eg_uint nCount);
	void ClampTo(eg_uint NewLength);
	void ConvertToUpper(){ EGString_ToUpper(m_strString,m_strStringSize); }
	void ConvertToLower(){ EGString_ToLower(m_strString,m_strStringSize); }
	void AddSlashes();
	void RemoveSlashes();

	void SetToFilenameFromPathNoExt( eg_cpstr Path );
	void SetToDirectoryFromPath( eg_cpstr strPath );
	void MakeThisFilenameRelativeTo( eg_cpstr strParent );

	void Replace( eg_cpstr RepString , eg_cpstr WithString );

	static void ToXmlFriendly( const eg_string_base& String , eg_string_base& Out );

private:
	eg_pstr const m_strString;
	const eg_uint m_strStringSize;
	eg_uint m_nLength;
private:
	void UpdateLength();
	EG_INLINE void CommonConstruct();
	EG_INLINE void CommonDestruct();
//Some metrics:
#if defined(__DEBUG__)
	static eg_int  s_StringCount;
	static eg_int  s_MaxStringCount;
	static eg_uint s_MaxStringLen;
#endif
};

template<eg_size_t MAX_SIZE> class eg_string_fixed_size: public eg_string_base
{
public:
	static const eg_size_t STR_SIZE=MAX_SIZE;
private:
	eg_char m_Mem[STR_SIZE];
public:
	eg_string_fixed_size(): eg_string_base( m_Mem , countof(m_Mem) ){ }
	eg_string_fixed_size( eg_ctor_t ConstrT ): eg_string_base( m_Mem , countof(m_Mem) , ConstrT ){ }
	eg_string_fixed_size(eg_cpstr8 str): eg_string_base( m_Mem , countof(m_Mem) ){ Set(str); }
	eg_string_fixed_size(eg_cpstr16 str): eg_string_base( m_Mem , countof(m_Mem) ){ Set(str); }
	eg_string_fixed_size(const eg_string_fixed_size<MAX_SIZE>& str): eg_string_base( m_Mem , countof(m_Mem) ){ Set(str); }
	eg_string_fixed_size(const eg_string_base& str): eg_string_base( m_Mem , countof(m_Mem) ){ Set(str); }
	~eg_string_fixed_size(){ }
};

//Strings have a maximum size. This is so that strings never need to
//allocate memory once they are created. This size can be changed as
//necessary, but it should be as small as possible for optimization, and
//so that the stack doesn't get filled up when strings are used.

typedef eg_string_fixed_size<1024> eg_string_big;
typedef eg_string_fixed_size<128> eg_string_small;
typedef eg_string_small eg_string;

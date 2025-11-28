// (c) 2019 Beem Media

#pragma once

struct egsformat_parm;
class EGSParmFormatter;
class IEGSFormatHandler;

class IEGSFormatHandler
{
public:

	virtual void FormatText( eg_cpstr16 Flags, EGSParmFormatter& Formatter ) const = 0;
};

struct egsformat_parm
{
	enum class eg_t
	{
		T_Null,
		T_Int,
		T_UInt,
		T_Int64,
		T_UInt64,
		T_Real,
		T_Bool,
		T_StringCrc,
		T_PtrStr8,
		T_PtrStr16,
		T_Custom,
	};

	egsformat_parm() { Int64 = 0; Type = eg_t::T_Null; }
	egsformat_parm( eg_int InValue ) : egsformat_parm() { Int = InValue; Type = eg_t::T_Int; }
	egsformat_parm( eg_uint InValue ) : egsformat_parm() { UInt = InValue; Type = eg_t::T_UInt; }
	egsformat_parm( unsigned int InValue ) : egsformat_parm(static_cast<eg_uint>(InValue)) { }
	egsformat_parm( eg_int64 InValue ) : egsformat_parm() { Int64 = InValue; Type = eg_t::T_Int64; }
	egsformat_parm( eg_uint64 InValue ) : egsformat_parm() { UInt64 = InValue; Type = eg_t::T_UInt64; }
	egsformat_parm( eg_real InValue ) : egsformat_parm() { Real = InValue; Type = eg_t::T_Real; }
	egsformat_parm( eg_bool InValue ) : egsformat_parm() { Bool = InValue; Type = eg_t::T_Bool; }
	egsformat_parm( eg_string_crc InValue ) : egsformat_parm() { Crc = InValue.ToUint32(); Type = eg_t::T_StringCrc; }
	egsformat_parm( eg_cpstr8 InValue ) : egsformat_parm() { PtrStr8 = InValue; Type = eg_t::T_PtrStr8; }
	egsformat_parm( eg_cpstr16 InValue ) : egsformat_parm() { PtrStr16 = InValue; Type = eg_t::T_PtrStr16; }
	egsformat_parm( IEGSFormatHandler* InValue ) : egsformat_parm() { CustomHandler = InValue; Type = eg_t::T_Custom; }
	egsformat_parm( IEGSFormatHandler& InValue ) : egsformat_parm() { CustomHandler = &InValue; Type = eg_t::T_Custom; }

	eg_t GetType() const { return Type; }

	eg_int as_int() const { return Int; }
	eg_uint as_uint() const { return UInt; }
	eg_int64 as_int64() const { return Int64; }
	eg_uint64 as_uint64() const { return UInt64; }
	eg_real as_real() const { return Real; }
	eg_bool as_bool() const { return Bool; }
	eg_string_crc as_crc() const { return eg_string_crc( Crc ); }
	eg_cpstr8 as_pstr8() const { return PtrStr8; }
	eg_cpstr16 as_pstr16() const { return PtrStr16; }
	IEGSFormatHandler* as_CustomHandler() const { return CustomHandler; }

private:
	
	union
	{
		eg_int             Int;
		eg_uint            UInt;
		eg_int64           Int64;
		eg_uint64          UInt64;
		eg_real            Real;
		eg_bool            Bool;
		eg_uint32          Crc;
		eg_cpstr8          PtrStr8;
		eg_cpstr16         PtrStr16;
		IEGSFormatHandler* CustomHandler;
	};

	eg_t Type;
};

eg_d_string8 EGSFormat8( eg_cpstr8 Format, const egsformat_parm& p0 = egsformat_parm(), const egsformat_parm& p1 = egsformat_parm(), const egsformat_parm& p2 = egsformat_parm(), const egsformat_parm& p3 = egsformat_parm(), const egsformat_parm& p4 = egsformat_parm(), const egsformat_parm& p5 = egsformat_parm(), const egsformat_parm& p6 = egsformat_parm(), const egsformat_parm& p7 = egsformat_parm() );
eg_d_string16 EGSFormat16( eg_cpstr16 Format, const egsformat_parm& p0 = egsformat_parm(), const egsformat_parm& p1 = egsformat_parm(), const egsformat_parm& p2 = egsformat_parm(), const egsformat_parm& p3 = egsformat_parm(), const egsformat_parm& p4 = egsformat_parm(), const egsformat_parm& p5 = egsformat_parm(), const egsformat_parm& p6 = egsformat_parm(), const egsformat_parm& p7 = egsformat_parm() );

class EGSParmFormatter
{
private:

	eg_s_string_sml16 m_String;

public:

	const eg_s_string_sml16& GetString() const { return m_String; }

	void SetText( eg_cpstr8 InValue , eg_cpstr16 FormatFlags );
	void SetText( eg_cpstr16 InValue , eg_cpstr16 FormatFlags );
	void SetNumber( eg_real InValue , eg_cpstr16 FormatFlags );
	void SetNumber( eg_uint InValue , eg_cpstr16 FormatFlags );
	void SetNumber( eg_int InValue , eg_cpstr16 FormatFlags );
	void SetBigNumber( eg_int64 InValue , eg_cpstr16 FormatFlags );
	void SetCrc( const eg_string_crc& InValue , eg_cpstr16 FormatFlags );
	void SetBool( eg_bool InValue , eg_cpstr16 FormatFlags );
	static eg_string_crc GetNextFlag( eg_cpstr16* Flags );

private:

	static void printfcommai( eg_int64 n , eg_s_string_sml8& Out );
	static void printfcommau( eg_uint64 n , eg_s_string_sml8& Out );
	static eg_s_string_sml8 SysNumberToStr( eg_real InValue , eg_cpstr16 Format );
	static eg_s_string_sml8 SysNumberToStr( eg_uint InValue , eg_cpstr16 Format );
	static eg_s_string_sml8 SysNumberToStr( eg_int InValue , eg_cpstr16 Format );
};

class EGTransformFormatter : public IEGSFormatHandler
{
private:

	const eg_transform*const m_Tansform;

public:

	EGTransformFormatter( const eg_transform& InTransform ) : m_Tansform( &InTransform ) { }

	virtual void FormatText( eg_cpstr16 Flags , EGSParmFormatter& Formatter ) const override
	{
		unused( Flags );
		eg_d_string16 Formatted = EGSFormat16( L"T:{0} {1} {2} R:{3} {4} {5} {6} S: {7}" , m_Tansform->GetTranslation().x , m_Tansform->GetTranslation().y , m_Tansform->GetTranslation().z , m_Tansform->GetRotation().x , m_Tansform->GetRotation().y , m_Tansform->GetRotation().z , m_Tansform->GetRotation().w , m_Tansform->GetScale() );
		Formatter.SetText( *Formatted , nullptr );
	}
};

class EGVec3Formatter : public IEGSFormatHandler
{
private:

	const eg_vec3*const m_Data;

public:

	EGVec3Formatter( const eg_vec3& InValue ) : m_Data( &InValue ) { }

	virtual void FormatText( eg_cpstr16 Flags , EGSParmFormatter& Formatter ) const override
	{
		unused( Flags );
		eg_d_string16 Formatted = EGSFormat16( L"{0} {1} {2}" , m_Data->x , m_Data->y , m_Data->z );
		Formatter.SetText( *Formatted , nullptr );
	}
};

class EGVec4Formatter : public IEGSFormatHandler
{
private:

	const eg_vec4*const m_Data;

public:

	EGVec4Formatter( const eg_vec4& InValue ) : m_Data( &InValue ) { }

	virtual void FormatText( eg_cpstr16 Flags , EGSParmFormatter& Formatter ) const override
	{
		unused( Flags );
		eg_d_string16 Formatted = EGSFormat16( L"{0} {1} {2} {3}" , m_Data->x , m_Data->y , m_Data->z , m_Data->w );
		Formatter.SetText( *Formatted , nullptr );
	}
};


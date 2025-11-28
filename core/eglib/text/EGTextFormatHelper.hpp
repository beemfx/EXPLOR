// (c) 2016 Beem Media

struct eg_loc_parm
{
	enum class eg_p_t
	{
		T_REAL,
		T_UINT,
		T_INT,
		T_CRC,
		T_BOOL,
		T_INT64,
		T_UINT64,
		T_LOCSTR,
		T_HANDLER,
	};

	eg_loc_parm(){ Int64 = 0; Type = eg_p_t::T_INT; }
	eg_loc_parm( eg_real R ): eg_loc_parm(){ Real = R; Type = eg_p_t::T_REAL; }
	eg_loc_parm( eg_uint I ): eg_loc_parm(){ Uint = I; Type = eg_p_t::T_UINT; }
	eg_loc_parm( eg_int I ): eg_loc_parm(){ Int = I; Type = eg_p_t::T_INT; }
	eg_loc_parm( eg_string_crc C ): eg_loc_parm(){ Crc = C.ToUint32(); Type = eg_p_t::T_CRC; }
	eg_loc_parm( eg_bool B ): eg_loc_parm(){ Bool = B; Type = eg_p_t::T_BOOL; }
	eg_loc_parm( eg_int64 I64 ): eg_loc_parm(){ Int64 = I64; Type = eg_p_t::T_INT64; }
	eg_loc_parm( eg_uint64 U64 ): eg_loc_parm(){ Uint64 = U64; Type = eg_p_t::T_UINT64; }
	eg_loc_parm( const eg_loc_text& s ): eg_loc_parm( s.GetString() ){ }
	eg_loc_parm( const eg_loc_char* s ): eg_loc_parm(){  Str = s; Type = eg_p_t::T_LOCSTR; }
	eg_loc_parm( const IEGCustomFormatHandler* p ): eg_loc_parm(){ Handler = p; Type = eg_p_t::T_HANDLER; }


	eg_p_t GetType()const{ return Type; }

	eg_real as_real()const{ return Real; }
	eg_uint as_uint()const{ return Uint; }
	eg_int as_int()const{ return Int; }
	eg_string_crc as_crc()const{ return eg_string_crc(Crc); }
	eg_bool as_bool()const{ return Bool; }
	eg_int64 as_int64()const{ return Int64; }
	eg_uint64 as_uint64()const{ return Uint64; }
	const eg_loc_char* as_str()const{ return Str; }
	const IEGCustomFormatHandler* as_handler()const{ return Handler; }

private:
	union
	{
		eg_real   Real;
		eg_uint   Uint;
		eg_int    Int;
		eg_uint32 Crc;
		eg_bool   Bool;
		eg_int64  Int64;
		eg_uint64 Uint64;
		const eg_loc_char* Str;
		const IEGCustomFormatHandler* Handler;
	};

	eg_p_t Type;
};

class EGTextParmHandler
{
public:
	

private:

	eg_loc_parm m_Parms[5];

public:
	EGTextParmHandler()
	{
		
	}

	EGTextParmHandler( eg_loc_parm p1 )
	{
		m_Parms[0] = p1;
	}

	EGTextParmHandler( eg_loc_parm p1 , eg_loc_parm p2 )
	{
		m_Parms[0] = p1;
		m_Parms[1] = p2;
	}

	EGTextParmHandler( eg_loc_parm p1 , eg_loc_parm p2 , eg_loc_parm p3 )
	{
		m_Parms[0] = p1;
		m_Parms[1] = p2;
		m_Parms[2] = p3;
	}

	EGTextParmHandler( eg_loc_parm p1 , eg_loc_parm p2 , eg_loc_parm p3 , eg_loc_parm p4 )
	{
		m_Parms[0] = p1;
		m_Parms[1] = p2;
		m_Parms[2] = p3;
		m_Parms[3] = p4;
	}

	EGTextParmHandler( eg_loc_parm p1 , eg_loc_parm p2 , eg_loc_parm p3 , eg_loc_parm p4 , eg_loc_parm p5 )
	{
		m_Parms[0] = p1;
		m_Parms[1] = p2;
		m_Parms[2] = p3;
		m_Parms[3] = p4;
		m_Parms[4] = p5;
	}

	void FormatParm( eg_uint Index , eg_cpstr Flags , class EGTextParmFormatter* Formatter );
};
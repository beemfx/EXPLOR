// (c) 2017 Beem Media

#pragma once

static const eg_size_t EGSM_MAX_PARMS=4;
static const eg_size_t EGSM_INVALID_INDEX = static_cast<eg_size_t>(0x80000000);
static const eg_uint   EGSM_BYTE_CODE_HEADER = 0x4D534745;

class EGFileData;

enum class egsm_node_t
{
#define DECL_NODE( _name_ , _color_ ) _name_ ,
#include "EGSmNodes.items"
};

enum class egsm_var_t
{
	UNK,
	RETURN_VOID,
	INT,
	REAL,
	BOOL,
	CRC,
	LOC_TEXT,
	NUMBER, // int or real, only good for function calls
	TERMINAL,
};

struct egsm_var
{
	egsm_var(): egsm_var( CT_Preserve ){ }

	egsm_var( eg_ctor_t Ct )
	{
		if( CT_Clear == Ct || CT_Default == Ct )
		{
			Type = egsm_var_t::INT;
			Int = 0;
		}
		else if( CT_Preserve == Ct )
		{

		}
	}

	egsm_var( eg_real Value ){ Real = Value; Type = egsm_var_t::REAL; }
	egsm_var( eg_int Value ){ Int = Value; Type = egsm_var_t::INT; }
	egsm_var( eg_string_crc Value ){ Crc = Value.ToUint32(); Type = egsm_var_t::CRC; }
	egsm_var( eg_bool Value ){ Int = Value ? 1 : 0; Type = egsm_var_t::BOOL; }

	eg_string_crc as_crc()const{ if( GetType() == egsm_var_t::CRC || GetType() == egsm_var_t::LOC_TEXT ){ return eg_string_crc(Crc); } return eg_string_crc( CT_Clear ); }
	eg_real as_real()const{ if( GetType() == egsm_var_t::REAL ){ return Real; } else if( GetType() == egsm_var_t::INT ){ return static_cast<eg_real>(Int); } return 0.f; }
	eg_int as_int()const{ if( GetType() == egsm_var_t::INT ){ return Int; } else if ( GetType() == egsm_var_t::REAL ) { return static_cast<eg_int>(Real); } else if( GetType() == egsm_var_t::BOOL ) { return Int != 0 ? 1 : 0; }  return 0; }
	eg_bool as_bool()const{ return Int != 0 ; } // Type doesn't really matter for this one.

	egsm_var_t GetType() const { return Type; }

	void operator = ( const eg_string_base& rhs )
	{
		if( rhs.IsInteger() )
		{
			*this = egsm_var( rhs.ToInt() );
		}
		else if( rhs.IsNumber() )
		{
			*this = rhs.ToFloat();
		}
		else if( rhs.EqualsI( "true" ) )
		{
			*this = true;
		}
		else if( rhs.EqualsI( "false" ) )
		{
			*this = false;
		}
		else
		{
			*this = egsm_var( eg_string_crc( rhs ) );
		}
	}

	egsm_var operator -() const
	{
		if( GetType() == egsm_var_t::INT )
		{
			return -as_int();
		}
		else if( GetType() == egsm_var_t::REAL )
		{
			return as_int();
		}

		return egsm_var( CT_Default );
	}

	void SetToButPreserveType( const egsm_var& rhs )
	{
		switch( GetType() )
		{
			default:
				assert( false ); // Invalid conversion.
				break;
			case egsm_var_t::INT:
				*this = rhs.as_int();
				break;
			case egsm_var_t::REAL:
				*this = rhs.as_real();
				break;
			case egsm_var_t::BOOL:
				*this = rhs.as_bool();
				break;
			case egsm_var_t::CRC:
				*this = rhs.as_crc();
				break;
		}
	}

private:

	egsm_var_t Type;
	union
	{
		eg_uint32 Crc;
		eg_real   Real;
		eg_int    Int;
	};
};

egsm_var operator + ( const egsm_var& A , const egsm_var& B );
egsm_var operator - ( const egsm_var& A , const egsm_var& B );
egsm_var operator * ( const egsm_var& A , const egsm_var& B );
egsm_var operator / ( const egsm_var& A , const egsm_var& B );
eg_bool operator == ( const egsm_var& A , const egsm_var& B );
eg_bool operator != ( const egsm_var& A , const egsm_var& B );
eg_bool operator >= ( const egsm_var& A , const egsm_var& B );
eg_bool operator > ( const egsm_var& A , const egsm_var& B );
eg_bool operator <= ( const egsm_var& A , const egsm_var& B );
eg_bool operator < ( const egsm_var& A , const egsm_var& B );

struct egsmBranchScr
{
	eg_string_small Id;
	eg_string_small ToNode;
	eg_bool         bTerminal:1;
	// Editor Only
	eg_string_big   EnRef;
};

struct egsmBranchBc
{
	eg_string_crc Id; // Doesn't need to be unique from node to node, but does need to be unique in amongst it's siblings.
	eg_size_t     IdDebugSym;
	eg_string_crc ToNodeCrc;
	eg_size_t     ToNodeAsIndexPlusOne; // Plus one so that zero can be null.
	eg_bool       bTerminal:1;

	egsmBranchBc() = default;

	explicit egsmBranchBc( const egsmBranchScr& rhs )
	{
		Id = eg_string_crc(rhs.Id);
		IdDebugSym = 0;
		ToNodeCrc = eg_string_crc(rhs.ToNode);
		bTerminal = rhs.bTerminal;
		ToNodeAsIndexPlusOne = 0;
	}
};

struct egsmNodeScr
{
	eg_string_small         Id;
	egsm_node_t             Type;
	eg_string_small         Parms[EGSM_MAX_PARMS];
	EGArray<egsmBranchScr>  Branches;
	// Editor Only
	eg_string_big           EnRef;
	eg_size_t               EnParmIndex;
	eg_vec2                 EditorPos;
	eg_bool                 bUsed:1;

	void CopyToClipboard( EGFileData& Dest );
	void PasteFromClipboard( const EGFileData& Source );
};

struct egsmNodeBc
{
	eg_string_crc Id;
	eg_size_t     IdDebugSym;
	egsm_node_t   Type;
	egsm_var      Parms[EGSM_MAX_PARMS];
	eg_size_t     NumBranches;
	eg_size_t     FirstBranch;

	egsmNodeBc() = default;

	explicit egsmNodeBc( const egsmNodeScr& rhs )
	{
		Id = eg_string_crc(rhs.Id);
		IdDebugSym = 0;
		Type = rhs.Type;
		for( eg_size_t i=0; i<EGSM_MAX_PARMS; i++ )
		{
			Parms[i] = rhs.Parms[i];
		}
		NumBranches = rhs.Branches.Len();
		FirstBranch = 0; // Will be filled out by compiler.
	}
};

struct egsmLabelBc
{
	eg_string_crc LabelName;
	eg_size_t     LabelNameDebugSym;
	eg_size_t     NodeToIndexPlusOne;
	eg_bool       bIsDefaultEntryPoint:1;
};

struct egsmMachinePropsScr
{
	eg_string_small Id;
	eg_string_small DefaultEntryPointNode;
	// Editor Only
	eg_string_small NewNodeIdPrefix;
	eg_string_small NewLocKeyPrefix;
	eg_uint         NewNodeIdSuffix;
	eg_uint         NewLocKeySuffix;
	eg_vec2         ViewOffset;

	egsmMachinePropsScr()
	: Id( CT_Clear )
	, DefaultEntryPointNode( CT_Clear )
	, NewNodeIdPrefix( "N_" )
	, NewLocKeyPrefix( "LOCID_" ) // Usually used for localization.
	, NewNodeIdSuffix( 0 )
	, NewLocKeySuffix( 0 )
	, ViewOffset( 0.f , 0.f )
	{

	}
};

struct egsmMachinePropsBc
{
	eg_string_crc Id;
	eg_size_t     IdDebugSym;

	egsmMachinePropsBc() = default;

	explicit egsmMachinePropsBc( const egsmMachinePropsScr& rhs )
	{
		Id = eg_string_crc( rhs.Id );
		IdDebugSym = 0;
	}
};

struct egsmByteCodeHeader
{
	eg_uint            FileType;
	egsmMachinePropsBc Props;
	eg_size_t          NodeCount;
	eg_size_t          NodeOffset;
	eg_size_t          BranchCount;
	eg_size_t          BranchOffset;
	eg_size_t          LabelCount;
	eg_size_t          LabelOffset;
	eg_size_t          DebugSymbolsCharCount;
	eg_size_t          DebugSymbolsOffset;
	egsmNodeBc*        Nodes;
	egsmBranchBc*      Branches;
	egsmLabelBc*       Labels;
	eg_char*           DebugSymbols;
};

enum class egsm_var_decl_t
{
	UNK,
	VAR,
	FUNCTION,
};

struct egsmVarDeclParmScr
{
	egsm_var_t      Type;
	eg_string_small Name;
	eg_string_small DefaultValue;
	eg_bool         bIsRef;
	eg_bool         bIsConst;
};

struct egsmVarDeclScr
{
	egsm_var_decl_t DeclType;
	union
	{
		egsm_var_t VarType;
		egsm_var_t ReturnType;
	};

	eg_string_small Name;

	// If variable
	egsm_var DefaultValue;

	// If function declaration
	egsmVarDeclParmScr ParmInfo[EGSM_MAX_PARMS];
};

struct egsm_node_id
{
	eg_size_t NodeIndexPlusOne;

	egsm_node_id() = default;
	egsm_node_id( eg_ctor_t Ct ){ if( Ct == CT_Default || Ct == CT_Clear ){ NodeIndexPlusOne = 0; } }
	explicit egsm_node_id( eg_size_t NodeIndexPlusOne ): NodeIndexPlusOne(NodeIndexPlusOne){ }

	eg_bool IsNull() const { return NodeIndexPlusOne == 0; }
};
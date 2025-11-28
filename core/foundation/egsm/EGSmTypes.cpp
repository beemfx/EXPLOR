// (c) 2017 Beem Media

#include "EGSmTypes.h"
#include "EGStdLibAPI.h"
#include "EGFileData.h"

egsm_var operator + ( const egsm_var& A , const egsm_var& B )
{
	egsm_var Out( CT_Clear );

	// If both are ints keep as int, otherwise promote to real.
	if( A.GetType() == egsm_var_t::INT && B.GetType() == egsm_var_t::INT )
	{
		// egsm_var's don't wrap around overflow cases go to limits.
		eg_int iA = A.as_int();
		eg_int iB = B.as_int();
		Out = iA + iB;
		if( iA > 0 && iB > 0 )
		{
			if( Out.as_int() < iA )
			{
				Out = static_cast<eg_int>(INT_MAX);
			}
		}
		else if( iA < 0 && iB < 0 )
		{
			if( Out.as_int() > iA )
			{
				Out = static_cast<eg_int>(INT_MIN);
			}
		}
	}
	else
	{
		Out = A.as_real() + B.as_real();
	}

	return Out;
}

egsm_var operator - ( const egsm_var& A , const egsm_var& B )
{
	egsm_var Out( CT_Clear );

	// If both are ints keep as int, otherwise promote to real.
	if( A.GetType() == egsm_var_t::INT && B.GetType() == egsm_var_t::INT )
	{
		// egsm_var's don't wrap around overflow cases go to limits.
		eg_int iA = A.as_int();
		eg_int iB = B.as_int();
		Out = iA - iB;
		if( iA > 0 && iB < 0 )
		{
			if( Out.as_int() < iA )
			{
				Out = static_cast<eg_int>(INT_MAX);
			}
		}
		else if( iA < 0 && iB > 0 )
		{
			if( Out.as_int() > iA )
			{
				Out = static_cast<eg_int>(INT_MIN);
			}
		}
	}
	else
	{
		Out = A.as_real() - B.as_real();
	}

	return Out;
}

egsm_var operator * ( const egsm_var& A , const egsm_var& B )
{
	egsm_var Out( CT_Clear );

	// If both are ints keep as int, otherwise promote to real.
	if( A.GetType() == egsm_var_t::INT && B.GetType() == egsm_var_t::INT )
	{
		// egsm_var's don't wrap around overflow cases go to limits.
		eg_int iA = A.as_int();
		eg_int iB = B.as_int();

		Out = iA * iB;

		if( iA != 0 && (Out.as_int()/iA) != iB ) 
		{
			if( (iA > 0 && iB > 0) || (iA < 0 && iB < 0) )
			{
				Out = static_cast<eg_int>(INT_MAX);
			}
			else
			{
				Out = static_cast<eg_int>(INT_MIN);
			}
		}
	}
	else
	{
		Out = A.as_real() * B.as_real();
	}

	return Out;
}

egsm_var operator / ( const egsm_var& A , const egsm_var& B )
{
	egsm_var Out( CT_Clear );

	// If both are ints keep as int, otherwise promote to real.
	if( A.GetType() == egsm_var_t::INT && B.GetType() == egsm_var_t::INT )
	{
		// egsm_var's don't wrap around overflow cases go to limits.
		eg_int iA = A.as_int();
		eg_int iB = B.as_int();
		if( iB == 0 )
		{
			if( iA < 0 )
			{
				Out = static_cast<eg_int>(INT_MIN);
			}
			else if( iA > 0 )
			{
				Out = static_cast<eg_int>(INT_MAX);
			}
			else
			{
				Out = static_cast<eg_int>(0);
			}
		}
		else if( iA == INT_MIN && iB == -1 )
		{
			Out = static_cast<eg_int>(INT_MAX);
		}
		else
		{
			Out = iA / iB;
		}
	}
	else
	{
		Out = A.as_real() / B.as_real();
	}

	return Out;
}

eg_bool operator == ( const egsm_var& A , const egsm_var& B )
{
	return A.GetType() == B.GetType() && A.as_int() == B.as_int();
}

eg_bool operator != ( const egsm_var& A , const egsm_var& B )
{
	return !(A == B);
}

eg_bool operator >= ( const egsm_var& A , const egsm_var& B )
{
	eg_bool Out = false;

	if( A.GetType() == egsm_var_t::INT && B.GetType() == egsm_var_t::INT )
	{
		Out = A.as_int() >= B.as_int();
	}
	else if( A.GetType() == egsm_var_t::REAL && B.GetType() == egsm_var_t::REAL )
	{
		Out = A.as_real() >= B.as_real();
	}
	else if( A.GetType() == egsm_var_t::INT && B.GetType() == egsm_var_t::REAL )
	{
		Out = A.as_int() >= B.as_real();
	}
	else if( A.GetType() == egsm_var_t::REAL && B.GetType() == egsm_var_t::INT )
	{
		Out = A.as_real() >= B.as_real();
	}
	else
	{
		Out = A == B;
	}

	return Out;
}

eg_bool operator > ( const egsm_var& A , const egsm_var& B )
{
	eg_bool Out = false;

	if( A.GetType() == egsm_var_t::INT && B.GetType() == egsm_var_t::INT )
	{
		Out = A.as_int() > B.as_int();
	}
	else if( A.GetType() == egsm_var_t::REAL && B.GetType() == egsm_var_t::REAL )
	{
		Out = A.as_real() > B.as_real();
	}
	else if( A.GetType() == egsm_var_t::INT && B.GetType() == egsm_var_t::REAL )
	{
		Out = A.as_int() > B.as_real();
	}
	else if( A.GetType() == egsm_var_t::REAL && B.GetType() == egsm_var_t::INT )
	{
		Out = A.as_real() > B.as_real();
	}
	else
	{
		Out = A == B;
	}

	return Out;
}

eg_bool operator <= ( const egsm_var& A , const egsm_var& B )
{
	return !(A > B);
}

eg_bool operator < ( const egsm_var& A , const egsm_var& B )
{
	return !(A >= B);
}

void egsmNodeScr::CopyToClipboard( EGFileData& Dest )
{
	auto WriteStr = [&Dest]( const auto& Str )
	{
		Dest.Write<eg_int>( Str.Len() );
		Dest.Write( Str.String() , Str.Len() );
	};
	
	Dest.Write( Type );
	for( eg_int i=0; i<EGSM_MAX_PARMS; i++ )
	{
		WriteStr( Parms[i] );
	}
	Dest.Write<eg_int>( Branches.LenAs<eg_int>() );
	for( eg_int i=0; i<Branches.LenAs<eg_int>(); i++ )
	{
		const egsmBranchScr& Branch = Branches[i];
		WriteStr( Branch.Id );
		Dest.Write<eg_int>( Branch.bTerminal ? 1 : 0 );
		WriteStr( Branch.EnRef );
	}
	WriteStr( EnRef );
	Dest.Write<eg_int>( EG_To<eg_int>(EnParmIndex) );
}

void egsmNodeScr::PasteFromClipboard( const EGFileData& Source )
{
	auto ReadStr = [&Source]( auto& Str )
	{
		eg_int StrLen = Source.Read<eg_int>();
		EGArray<eg_char> StrBuffer;
		StrBuffer.Resize( StrLen );
		Source.Read( StrBuffer.GetArray() , StrBuffer.Len() );
		StrBuffer.Append( '\0' );
		Str = StrBuffer.GetArray();
	};
	
	Id = CT_Clear;
	Type = Source.Read<egsm_node_t>();
	for( eg_int i=0; i<EGSM_MAX_PARMS; i++  )
	{
		ReadStr( Parms[i] );
	}
	const eg_int NumBranches = Source.Read<eg_int>();
	Branches.Resize( NumBranches );
	for( eg_int i=0; i<Branches.LenAs<eg_int>(); i++ )
	{
		egsmBranchScr& Branch = Branches[i];
		ReadStr( Branch.Id );
		Branch.bTerminal = Source.Read<eg_int>() != 0;
		ReadStr( Branch.EnRef );
	}
	ReadStr( EnRef );
	EnParmIndex = Source.Read<eg_int>();
}

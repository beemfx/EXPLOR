// Behavior Handling
// (c) 2016 Beem Media
#pragma once

#include "EGStringMap.h"
#include "EGDelegate.h"

struct eg_event_parms
{
	union
	{
		eg_int64  AsInt64;
		eg_uint32 AsStringCrc;
		eg_uint   AsEntId;
		eg_ivec2  AsIVec2;
	};

	eg_event_parms( eg_ctor_t Ct )
	{
		static_assert( sizeof(*this) == sizeof(eg_int64) , "Too big?" );

		if( Ct == CT_Default || Ct == CT_Clear )
		{
			AsInt64 = 0;
		}
	}

	eg_event_parms( eg_int64 Value ){ AsInt64 = Value; }
	//eg_behavior_parms( eg_uint32 Value ){ AsInt64 = Value; }
	//eg_behavior_parms( eg_size_t Value ){ AsInt64 = Value; }
	eg_event_parms( eg_string_crc Value ) { AsInt64=0; AsStringCrc = Value.ToUint32(); }
	eg_event_parms( eg_ent_id Value ) { AsInt64 = 0; AsEntId = eg_ent_id::IdToRaw( Value ); }
	eg_event_parms( const eg_ivec2& Value ) { AsIVec2 = Value; }
	//eg_behavior_parms( eg_bool Value ) { AsInt64 = Value ? 1 : 0; }

	const eg_event_parms& operator = ( const eg_int64 Value ){ AsInt64 = Value; return *this; }
	const eg_event_parms& operator = ( const eg_string_crc Value ) { AsInt64 = 0; AsStringCrc = Value.ToUint32(); return *this; }

	eg_int64 as_int64() const { return AsInt64; }
	eg_int32 as_int32() const { return static_cast<eg_int32>(AsInt64); }
	eg_string_crc as_string_crc() const { return eg_string_crc(AsStringCrc); }
	eg_ent_id as_ent_id() const { return eg_ent_id::RawToId( AsEntId ); }
	eg_bool as_bool() const { return AsInt64 != 0; }
	eg_ivec2 as_ivec2() const { return AsIVec2; }
};

struct egRemoteEvent
{
	eg_string_crc  Function;
	eg_event_parms Parms;

	egRemoteEvent(): Function( CT_Clear ), Parms( CT_Clear ) { }
	egRemoteEvent( const eg_string_crc& InBehavior ) : Function( InBehavior ), Parms( CT_Clear ) { }
	egRemoteEvent( const eg_string_crc& InBehavior, eg_event_parms InParms ) : Function( InBehavior ), Parms( InParms ) { }
};


template<typename C> class EGRemoteEventHandler
{
private:

	//
	// Types
	//

	typedef void ( C::* EGRemoteEventCall )( const eg_event_parms& Parms );

	struct egItem
	{
		eg_string_crc     CrcId;
		EGRemoteEventCall Exec;

		egItem(): CrcId( CT_Clear ) , Exec( nullptr ){ }
		egItem( eg_string_crc InCrcId , EGRemoteEventCall InExec ): CrcId( InCrcId ) , Exec( InExec ){ }
	};

	typedef EGStringCrcMap<egItem> EGRemoteEventMap;

private:

	//
	// Attributes
	//

	EGRemoteEventMap m_Map;

public:

	//
	// Interface:
	//

	EGRemoteEventHandler(): m_Map( egItem() ){ }

	void RegisterEvent( eg_string_crc CrcId , EGRemoteEventCall Exec )
	{
		egItem NewItem( CrcId , Exec );
		if( !m_Map.Contains( CrcId ) )
		{
			m_Map.Insert( CrcId , NewItem );
		}
		else
		{
			assert( false ); // Two events had the same crc id.
		}
	}

	eg_bool ExecuteEvent( C* Owner , const egRemoteEvent& Event )
	{
		if( m_Map.Contains( Event.Function ) )
		{
			const egItem& ExecItem = m_Map[Event.Function];
			if( ExecItem.Exec )
			{
				( Owner->*ExecItem.Exec )( Event.Parms );
				return true;
			}
		}

		return false;
	}
};

class EGREHandler2
{
public:

	typedef EGDelegate<void,const eg_event_parms&> egCall;

private:

	struct egItem
	{
		eg_string_crc CrcId;
		egCall        Exec;

		egItem(): CrcId( CT_Clear ){ }
		egItem( eg_string_crc InCrcId , egCall InExec ): CrcId( InCrcId ) , Exec( InExec ){ }
	};

	typedef EGStringCrcMap<egItem> EGRemoteEventMap;

private:

	//
	// Attributes
	//

	EGRemoteEventMap m_Map;

public:

	//
	// Interface:
	//

	EGREHandler2(): m_Map( egItem() ){ }

	void RegisterEvent( eg_string_crc CrcId , egCall Exec )
	{
		egItem NewItem( CrcId , Exec );
		if( !m_Map.Contains( CrcId ) )
		{
			m_Map.Insert( CrcId , NewItem );
		}
		else
		{
			assert( false ); // Two events had the same crc id.
		}
	}

	eg_bool ExecuteEvent( const egRemoteEvent& Event )
	{
		if( m_Map.Contains( Event.Function ) )
		{
			egItem& ExecItem = m_Map[Event.Function];
			ExecItem.Exec.ExecuteIfBound( Event.Parms );
			return true;
		}

		return false;
	}
};

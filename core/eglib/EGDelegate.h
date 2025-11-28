// (c) 2017 Beem Media

#pragma once

#include "EGWeakPtr.h"

class EGObject;

template<typename RetValue,typename...CallArgs>
class EGDelegate
{
private:

	typedef RetValue (EGObject::*egFunction)(CallArgs...args);

	EGWeakPtr<EGObject> m_Target = nullptr;
	egFunction          m_Function = nullptr;

public:

	EGDelegate() = default;

	EGDelegate( const EGDelegate& rhs )
	{
		Bind( const_cast<EGDelegate&>(rhs).m_Target.GetObject() , rhs.m_Function );
	}

	EGDelegate( EGDelegate&& rhs )
	{
		Bind( rhs.m_Target.GetObject() , rhs.m_Function );
		rhs.Clear();
	}

	template<typename FnType>
	EGDelegate( EGObject* InTarget , FnType InFunction )
	{
		Bind( InTarget , InFunction );
	}

	~EGDelegate()
	{
		Clear();
	}

	EGDelegate& operator = ( const EGDelegate& rhs )
	{
		Clear();
		Bind( const_cast<EGDelegate&>(rhs).m_Target.GetObject() , rhs.m_Function );
		return *this;
	}

	EGDelegate& operator = ( EGDelegate&& rhs )
	{
		Clear();
		Bind( rhs.m_Target.GetObject() , rhs.m_Function );
		rhs.Clear();
		return *this;
	}

	template<typename FnType>
	void Bind( EGObject* InTarget , FnType InFunction )
	{
		m_Target = InTarget;
		m_Function = static_cast<egFunction>(InFunction);
	}

	eg_bool IsBound() const
	{
		return ( m_Target.IsValid() && m_Function );
	}

	RetValue Execute( CallArgs...args )
	{
		assert( IsBound() );
		return (m_Target.GetObject()->*m_Function)( args... );
	}

	void ExecuteIfBound( CallArgs...args )
	{
		if( IsBound() )
		{
			(m_Target.GetObject()->*m_Function)( args... );
		}
	}

	void Clear()
	{
		m_Target = nullptr;
		m_Function = nullptr;
	}
};

template<typename RetValue,typename...CallArgs>
class EGRawDelegate
{
private:

	typedef RetValue (*egFunction)(void*,CallArgs...args);

	void*      m_Target = nullptr;
	egFunction m_Function = nullptr;

public:

	EGRawDelegate() = default;
	EGRawDelegate( const EGRawDelegate& rhs ) = delete;
	EGRawDelegate( EGRawDelegate&& rhs ) = delete;

	template<typename FnType>
	EGRawDelegate( void* InTarget , FnType InFunction )
	{
		Bind( InTarget , InFunction );
	}

	~EGRawDelegate()
	{
		Clear();
	}

	EGRawDelegate& operator = ( const EGRawDelegate& rhs ) = delete;
	EGRawDelegate& operator = ( EGRawDelegate&& rhs ) = delete;

	template<typename FnType>
	void Bind( void* InTarget , FnType InFunction )
	{
		m_Target = InTarget;
		m_Function = static_cast<egFunction>(InFunction);
	}

	eg_bool IsBound() const
	{
		return ( m_Target && m_Function );
	}

	RetValue Execute( CallArgs...args )
	{
		assert( IsBound() );
		return (*m_Function)( m_Target , args... );
	}

	void ExecuteIfBound( CallArgs...args )
	{
		if( IsBound() )
		{
			(*m_Function)( m_Target , args... );
		}
	}

	void Clear()
	{
		m_Target = nullptr;
		m_Function = nullptr;
	}
};

template<typename...CallArgs>
class EGMCDelegate
{
private:

	typedef void (EGObject::*egFunction)(CallArgs...args);

	struct egListener
	{
		EGWeakPtr<EGObject> Target = nullptr;
		egFunction          Function = nullptr;

		eg_bool operator==( const egListener& rhs )
		{
			return Target == rhs.Target && Function == rhs.Function;
		}
	};

	EGArray<egListener> m_Listeners;

public:

	EGMCDelegate() = default;
	EGMCDelegate( const EGMCDelegate& rhs ) = delete;

	~EGMCDelegate()
	{
		Clear();
	}

	EGMCDelegate& operator=( const EGMCDelegate& rhs ) = delete;

	template<typename FnType>
	void Add( EGObject* InTarget , FnType InFunction )
	{
		if( InTarget && InFunction )
		{
			egListener NewListener;
			NewListener.Target = InTarget;
			NewListener.Function = static_cast<egFunction>(InFunction);
			m_Listeners.Append( NewListener );
		}
	}

	template<typename FnType>
	void AddUnique( EGObject* InTarget , FnType InFunction )
	{
		if( InTarget && InFunction )
		{
			egListener NewListener;
			NewListener.Target = InTarget;
			NewListener.Function = static_cast<egFunction>(InFunction);
			if( !m_Listeners.Contains( NewListener ) )
			{
				m_Listeners.Append( NewListener );
			}
		}
	}

	template<typename FnType>
	void Remove( EGObject* InTarget , FnType InFunction )
	{
		if( InTarget && InFunction )
		{
			egListener Listener;
			Listener.Target = InTarget;
			Listener.Function = static_cast<egFunction>(InFunction);
			if( m_Listeners.Contains( Listener ) )
			{
				m_Listeners.DeleteByItem( Listener );
			}
		}
	}

	void RemoveAll( EGObject* InTarget )
	{
		m_Listeners.DeleteAllByPredicate( [&InTarget]( const egListener& Listener )->eg_bool{ return Listener.Target == InTarget || !Listener.Target.IsValid(); } );
	}

	void Broadcast( CallArgs...args )
	{
		// Need a copy of listeners so that listeners may be removed while
		// broadcasting.
		EGArray<egListener> ListenerCopy = m_Listeners;

		for( egListener& Listener : ListenerCopy )
		{
			if( Listener.Target.IsValid() && Listener.Function )
			{
				(Listener.Target.GetObject()->*Listener.Function)( args... );
			}
		}
	}

	void Clear()
	{
		for( egListener& Listener : m_Listeners )
		{
			Listener.Target = nullptr;
		}
		m_Listeners.Clear();
	}

	eg_bool HasListeners() const { return m_Listeners.Len() > 0; }
	eg_size_t GetNumListeners() const { return m_Listeners.Len(); }
};

typedef EGDelegate<void> EGSimpleDelegate;
typedef EGMCDelegate<> EGSimpleMCDelegate;
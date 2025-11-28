// (c) 2017 Beem Media

#pragma once

#include "EGWeakPtrBase.h"
#include "EGObject.h"

template<typename ObjClass>
class EGWeakPtr : public EGWeakPtrBase
{
private:

	ObjClass* m_Object = nullptr;

public:

	EGWeakPtr() = default;
	EGWeakPtr( const EGWeakPtr<ObjClass>& rhs )
	{
		SetObject( const_cast<ObjClass*>(rhs.GetObject()) );
	}

	EGWeakPtr( ObjClass* rhs )
	{
		assert( reinterpret_cast<EGObject*>(rhs) == static_cast<EGObject*>(rhs) );
		SetObject( rhs );
	}

	EGWeakPtr( nullptr_t )
	: EGWeakPtr()
	{

	}

	~EGWeakPtr()
	{
		Unregister();
	}

	ObjClass* operator = ( ObjClass* rhs )
	{
		assert( reinterpret_cast<EGObject*>(rhs) == static_cast<EGObject*>(rhs) );
		SetObject( rhs );
		return GetObject();
	}

	ObjClass* operator = ( const EGWeakPtr<ObjClass>& rhs )
	{
		SetObject( const_cast<ObjClass*>(rhs.GetObject()) );
		return GetObject();
	}

	eg_bool operator == ( const ObjClass* rhs ) const
	{
		return GetObject() == rhs;
	}

	eg_bool operator == ( const EGWeakPtr<ObjClass>& rhs ) const
	{
		return GetObject() == rhs.GetObject();
	}

	ObjClass* operator -> ()
	{
		return GetObject();
	}

	const ObjClass* operator ->() const
	{
		return GetObject();
	}

	void SetObject( ObjClass* InObject )
	{
		Unregister();
		m_AsObject = reinterpret_cast<EGObject*>(InObject);
		m_Object = InObject;
		Register();
	}

	operator eg_bool() const { return IsValid(); }

	ObjClass* GetObject(){ return IsValid() ? m_Object : nullptr; }
	const ObjClass* GetObject() const { return IsValid() ? m_Object : nullptr; }

	eg_bool IsValid() const { return m_Object && m_AsObject; }
	eg_bool IsStale() const { return m_Object != nullptr && m_AsObject == nullptr; }
};
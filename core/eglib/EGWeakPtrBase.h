// (c) 2017 Beem Media

#pragma once

#include "EGList.h"

class EGObject;

class EGWeakPtrBase : public IListable
{
friend class EGObject;

protected:

	EGWeakPtrBase() = default;
	EGWeakPtrBase( const EGWeakPtrBase& rhs ) = delete;
	EGWeakPtrBase& operator=( const EGWeakPtrBase& rhs ) = delete;

protected:

	EGObject* m_AsObject = nullptr;

protected:

	void Register();
	void Unregister();
};
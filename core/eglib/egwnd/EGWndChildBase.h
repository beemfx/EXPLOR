// (c) 2017 Beem Media

#pragma once

#include "EGWndBase.h"

class EGWndChildBase : public EGWndBase
{
private: typedef EGWndBase Super;
private: static eg_cpstr16 CLASS_NAME;
protected:

	EGWndBase*const m_OwnerWindow;

public:

	static void InitClass( HINSTANCE hInst );
	static void DeinitClass( HINSTANCE hInst );

	EGWndChildBase( const EGWndChildBase& rhs ) = delete;
	EGWndChildBase( EGWndBase* OnwerWindow );
	virtual ~EGWndChildBase() override;

};


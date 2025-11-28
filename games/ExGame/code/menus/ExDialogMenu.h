// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExConsts.h"
#include "EGDelegate.h"

enum eg_gp_btn;
enum eg_kbm_btn;

class IExDialogListener
{
public:

	virtual void OnDialogClosed( eg_string_crc ListenerParm , eg_string_crc Choice )=0;
};

typedef EGDelegate<void,eg_string_crc /*ListenerParm*/,eg_string_crc /*Choice*/> ExDialogDelegate;
typedef EGDelegate<void,eg_kbm_btn,eg_gp_btn> ExKeyBindDialogDelegate;

struct exDialogParms
{
	eg_loc_text            HeaderText;
	eg_loc_text            BodyText;
	EGArray<eg_string_crc> Choices;
	eg_uint                InitialChoice;
	eg_string_crc          DlgListenerParm;
	eg_string_crc          RemoteEventOnClose;
	IExDialogListener*     DlgListener;
	ExDialogDelegate       DelegateCb;
	eg_bool                bMuteIntro;

	exDialogParms( eg_ctor_t Ct )
	: HeaderText( Ct )
	, BodyText( Ct )
	, DlgListenerParm( CT_Clear )
	, DlgListener( nullptr )
	, RemoteEventOnClose( CT_Clear )
	, InitialChoice( 0 )
	, bMuteIntro( false )
	{
		assert( CT_Clear == Ct );
		Choices.Clear();
	}
};

struct exKeyBindDialogParms
{
	ExKeyBindDialogDelegate DelegateCb;
	eg_loc_text HeaderText;
	eg_loc_text BodyText;
};

void ExDialogMenu_PushDialogMenu( class EGClient* Client , const exDialogParms& Parms );
void ExDialogMenu_PushKeyBindDialog( class EGClient* Client , const exKeyBindDialogParms& Parms );

struct exMessageDialogParms: public exDialogParms
{
	exMessageDialogParms( const eg_loc_text& Message )
		: exDialogParms( CT_Clear )
	{
		HeaderText = eg_loc_text( eg_crc("txtGameTitleShort") );
		BodyText = Message;
	}

	exMessageDialogParms( eg_string_crc Message )
		: exMessageDialogParms( eg_loc_text( Message ) )
	{

	}
};

struct exYesNoDialogParms: public exDialogParms
{
	exYesNoDialogParms( const eg_loc_text& Message , eg_bool bNoByDefault = false )
		: exDialogParms( CT_Clear )
	{

		HeaderText = eg_loc_text( eg_crc("txtGameTitleShort") );
		BodyText = Message;
		Choices.Append( eg_crc("Yes") );
		Choices.Append( eg_crc("No") );

		if(bNoByDefault )
		{
			InitialChoice = 1;
		}
	}

	exYesNoDialogParms( eg_string_crc Message , eg_bool bNoByDefault = false )
	: exYesNoDialogParms( eg_loc_text( Message ) , bNoByDefault )
	{

	}

	exYesNoDialogParms( eg_string_crc Message , IExDialogListener* InListener , eg_string_crc InListenerParm , eg_bool bNoByDefault = false )
		: exYesNoDialogParms( eg_loc_text( Message ) , bNoByDefault )
	{
		DlgListener = InListener;
		DlgListenerParm = InListenerParm;
	}

	exYesNoDialogParms( const eg_loc_text& Message , IExDialogListener* InListener , eg_string_crc InListenerParm , eg_bool bNoByDefault = false )
		: exYesNoDialogParms( Message , bNoByDefault )
	{
		DlgListener = InListener;
		DlgListenerParm = InListenerParm;
	}
};

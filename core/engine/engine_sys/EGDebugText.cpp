// (c) 2015 Beem Media

#include "EGDebugText.h"
#include "EGTextNode.h"
#include "EGRenderer.h"
#include "EGLocText.h"
#include "EGEngineConfig.h"

eg_color32 DebugText_GetStringColorAndRemoveColorTag( eg_string* String )
{
	static const eg_color32 DEFAULT_COLOR(170,171,89);

	//If the string isn't log enough it's not colored.
	if( nullptr == String || String->Len() < 11 )
	{
		return DEFAULT_COLOR;
	}

	//If we don't have the color code, it's not colored.
	if( String->CharAt(0) != '&' || String->CharAt(1) != 'c' || String->CharAt(2) != '!' )
	{
		return DEFAULT_COLOR;
	}

	eg_string Original( *String );
	*String = Original.String() + 11;

	Original.ClampTo(11);
	Original = eg_string(Original.String()+3);
	eg_color32 Color;
	Color.AsU32 = Original.ToUIntFromHex();

	return Color;
}

//////////////////////////////////////////////////////////////////////////////

EGLogToScreen EGLogToScreen::s_Inst;

void EGLogToScreen::Init()
{

}

void EGLogToScreen::Update( eg_real DeltaTime )
{
	EGFunctionLock FnLock( &m_LogLock );

	for( egLogInfo& Log : m_Logs )
	{
		Log.TimeLeft -= DeltaTime;
	}

	m_Logs.DeleteAllByPredicate( []( egLogInfo& Log ) -> eg_bool { return Log.TimeLeft <= 0.f; } );
}

void EGLogToScreen::Draw( eg_real AspectRatio )
{
	EGFunctionLock FnLock( &m_LogLock );

	auto DoDraw = [this]( EGTextNode& TextNode , eg_uint& y, const eg_real HEIGHT, const eg_real DEBUG_TEXT_SIZE ) -> void
	{
		eg_mat mT;

		const eg_uint NumLines = m_Logs.LenAs<eg_uint>();

		for(eg_uint i=0; i<NumLines; i++)
		{
			TextNode.ClearText();
			eg_cpstr Line = m_Logs[i].Message;
			eg_loc_text LocText( static_cast<const eg_loc_char*>(EGString_ToWide( Line )) );
			if( LocText.GetLen() > 0 )
			{
				TextNode.SetText( LocText );
				mT = eg_mat::BuildTranslation( eg_vec3(0, HEIGHT/2.0f-(y+0.5f)*DEBUG_TEXT_SIZE, 0));
				MainDisplayList->SetWorldTF( mT );
				TextNode.Draw();
				y++;
			}
		}
	};

	m_TextNode.ClearText();
	EGFontBase* Font = EGFontMgr::Get()->GetFont( CON_FONT_ID );
	if( nullptr == Font )return; //Can't draw if there is no font.

	const eg_real HEIGHT = 2.0f;
	const eg_real WIDTH  = 2.0f*AspectRatio;

	MainDisplayList->ClearDS( 1.0f , 0 );
	eg_mat M = eg_mat::I;
	MainDisplayList->SetWorldTF( M );
	MainDisplayList->SetViewTF( M );
	M = eg_mat::BuildOrthographicLH( WIDTH , HEIGHT , -1.0f , 1.0f );
	MainDisplayList->SetProjTF( M );

	eg_uint y = 0;

	eg_bool bCanDraw = false;
#if defined(__EGEDITOR__) || defined(__DEBUG__)
	bCanDraw = true;
#endif

	//Frame strings are rendered in the upper left.
	if( bCanDraw )
	{
		const eg_real DEBUG_TEXT_SIZE = HEIGHT/60.0f;
		m_TextNode.SetupNode( Font, WIDTH, DEBUG_TEXT_SIZE, DEBUG_TEXT_SIZE );

		m_TextNode.SetColor(eg_color(1,1,0,1));
		DoDraw( m_TextNode , y , HEIGHT , DEBUG_TEXT_SIZE );
	}
}

void EGLogToScreen::Deinit()
{
	m_Logs.Clear();
	m_TextNode.ClearAndFreeMem();
}

void EGLogToScreen::Log( eg_uintptr_t UniqueId , eg_real Duration , eg_cpstr Message )
{
	EGFunctionLock FnLock( &m_LogLock );

	for( egLogInfo& Log : m_Logs )
	{
		if( Log.UniqueId == UniqueId )
		{
			Log.TimeLeft = Duration;
			Log.Message = Message;
			return;
		}
	}

	egLogInfo NewLog;
	NewLog.UniqueId = UniqueId;
	NewLog.TimeLeft = Duration;
	NewLog.Message = Message;
	m_Logs.Append( NewLog );
}

void EGLogToScreen::Log( const void* Owner , eg_uint Index , eg_real Duration , eg_cpstr Message )
{
	Log( reinterpret_cast<eg_uintptr_t>(Owner) + Index , Duration , Message );
}

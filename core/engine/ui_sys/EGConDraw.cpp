// ConDraw - A module for drawing the contents of MainConsole.
#include "EGConDraw.h"
#include "EGFont.h"
#include "EGEngineConfig.h"
#include "EGRenderer.h"
#include "EGConsole.h"
#include "EGTextNode.h"
#include "EGInput.h"
#include "EGDebugText.h"

static struct egConDraw
{
	egv_material BgMtrl;
	eg_bool      IsActive;
	eg_int       ScrollPos;
	eg_real      ConsolePosition;
	EGTextNode*  TextNode = nullptr;

	egConDraw(): IsActive(false), ScrollPos(0), ConsolePosition(0)
	{
	}
}
ConDraw_Data;

void ConDraw_Init( void )
{
	ConDraw_Data.BgMtrl = EGRenderer::Get().CreateMaterialFromTextureFile( CON_BG );
	ConDraw_Data.TextNode = new ( eg_mem_pool::System ) EGTextNode( CT_Default );
}

void ConDraw_Deinit( void )
{
	EG_SafeDelete( ConDraw_Data.TextNode );
	EGRenderer::Get().DestroyMaterial( ConDraw_Data.BgMtrl );
	ConDraw_Data.BgMtrl = EGV_MATERIAL_NULL;
}

void ConDraw_ToggleActive( void )
{
	ConDraw_Data.IsActive = !ConDraw_Data.IsActive;
	if( ConDraw_IsActive() )
	{
		ConDraw_Data.ConsolePosition = 2.0f;
	}
}

bool ConDraw_IsActive( void )
{
	return ConDraw_Data.IsActive;
}

void ConDraw_OnChar( eg_char c )
{
	if( !ConDraw_IsActive() ){ assert( false ); return; } //Shouldn't burn char input on the console if it isn't active.

	EGConsole* Con = &MainConsole;
	Con->OnChar(c);
}

void ConDraw_Update( eg_real DeltaTime , const egLockstepCmds* Cmds )
{
	if( !ConDraw_IsActive() )return;
	EGConsole* Con = &MainConsole;

	eg_bool bShouldRedraw=false;

	//Check to see if a scroll command was issued:
	//Might want to do this as KEY_DOWN though.
	if(Cmds->WasPressed(CMDA_SYS_CONPAGEUP))
	{
		ConDraw_Data.ScrollPos++;
		bShouldRedraw = true;
	}
		
	if(Cmds->WasPressed(CMDA_SYS_CONPAGEDOWN))
	{
		ConDraw_Data.ScrollPos--;
		bShouldRedraw = true;
	}

	if(Cmds->WasPressed(CMDA_SYS_CONHISTORY))
	{
		Con->SetCmdLineNextHistory();
	}

	if( Cmds->WasMenuPressed( CMDA_MENU_BACK ) )
	{
		ConDraw_ToggleActive();
	}

	if( Cmds->WasMenuPressed( CMDA_SYS_CONTOGGLE ) )
	{
		ConDraw_ToggleActive();
	}

	if( Con->WasUpdated() )
	{
		bShouldRedraw = true;
	}


	if(bShouldRedraw)
	{

	}

	//The speed is the number of screen units per second. At 3 units per seconds
	//it takse about .33 seconds for the console to move 1 screen unit (which is
	//where we want it to be).
	const eg_real MOVE_SPEED = 4.0f;
	ConDraw_Data.ConsolePosition -= MOVE_SPEED*DeltaTime;
	if( ConDraw_Data.ConsolePosition < 0.5f )
	{
		ConDraw_Data.ConsolePosition = 0.5f;
	}
}

static void ConDraw_Draw_OutText(  eg_real AspectRatio , eg_uint nPos , eg_cpstr strLine , eg_real ConsolePosition )
{
	if( ConDraw_Data.TextNode == nullptr )
	{
		return;
	}

	EGFontBase* Font = EGFontMgr::Get()->GetFont( CON_FONT_ID );
	const eg_real WIDTH = 2.0f*AspectRatio-0.08f;
	const eg_real HEIGHT = 2.0f-0.02f;

	//The first two characters can determine what color the text should be
	//if the first character is & then the second character determes the color.
	eg_string strOut = strLine;
	eg_color32 nColor = DebugText_GetStringColorAndRemoveColorTag( &strOut );

	const eg_real FONT_HEIGHT = 0.075f;

	EGTextNode& Node = *ConDraw_Data.TextNode;
	Node.SetupNode( Font , WIDTH, HEIGHT, FONT_HEIGHT);
	Node.SetAllowFormatting( false );
	Node.SetAllowStyles( false );
	const eg_vec2 ShadowOffset = Node.GetFontShadowOffset();
	const eg_loc_text LocText( static_cast<const eg_loc_char*>(EGString_ToWide(strOut)) );

	eg_mat mT;

	for( eg_uint i=0; i<2; i++ )
	{
		mT = eg_mat::BuildTranslation(eg_vec3(0,ConsolePosition-HEIGHT+(nPos+1)*FONT_HEIGHT, 0));
		if( 1 == i )
		{
			mT *= eg_mat::BuildTranslation( eg_vec3(-ShadowOffset.x , -ShadowOffset.y , 0.f) );// eg_vec3(-SHADOW_OFFSET, SHADOW_OFFSET, 0));
		}
		MainDisplayList->SetWorldTF( mT );
		Node.ClearText();
		Node.SetColor( eg_color(0 == i ? eg_color32(0,0,0) : nColor) );
		Node.SetText( LocText );
		Node.Draw();
	}
}


void ConDraw_Draw( eg_real AspectRatio )
{
	const eg_uint NUM_LINES = 20;
	EGFontBase* Font = EGFontMgr::Get()->GetFont( CON_FONT_ID );
	if( !ConDraw_IsActive() || nullptr == Font )return;
	EGConsole* Con = &MainConsole;

	Con->Lock();

	eg_uint nNumLines = Con->GetNumLines();
	//If the last line is empty, then ignore it.
	if((nNumLines > 0) && (0 == Con->GetLineAt(nNumLines-1)[0]))
		nNumLines--;

	//Always clamp the scroll position, just in case the console
	//was cleared, and for calls to ScrollUp and ScrollDown.
	ConDraw_Data.ScrollPos = nNumLines<3 ? 0 : EG_Clamp<eg_int>(ConDraw_Data.ScrollPos, 0, nNumLines-1);

	//Draw a white background:
	//this->DrawBackground(D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF));
	
	//Render the background, we just have the background texture
	//take up the whole space:
	MainDisplayList->ClearDS(1.0f,0);
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::TEXTURE );
	MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
	MainDisplayList->SetMaterial( ConDraw_Data.BgMtrl );
	eg_mat M;
	M = eg_mat::I;
	MainDisplayList->SetProjTF(M);
	MainDisplayList->SetViewTF(M);
	M *= eg_mat::BuildTranslation( eg_vec3( 0 , ConDraw_Data.ConsolePosition , 0 ) );
	MainDisplayList->SetWorldTF(M);
	MainDisplayList->DrawBasicQuad();
	MainDisplayList->PopDepthStencilState();
	MainDisplayList->PopDefaultShader();

	//Draw the actual text:
	//Draw the console text:
	MainDisplayList->ClearDS(1.0f,0);
	M = eg_mat::BuildOrthographicLH( 2.0f*AspectRatio , 2.0f , -1.0f , 1.0f );
	MainDisplayList->SetViewTF( M );

	{
		//The bottom most line will always be printed.
		eg_string strBottomLine((":"));
		strBottomLine+=Con->GetCmdLine();
		strBottomLine.Append('_');
		eg_cpstr strLine = strBottomLine.String();
		eg_uint nDrawPos=0;
		ConDraw_Draw_OutText(AspectRatio , nDrawPos, strLine, ConDraw_Data.ConsolePosition);
		nDrawPos++;
	
		eg_int nStart = (nNumLines-1)-ConDraw_Data.ScrollPos;

		if(ConDraw_Data.ScrollPos>0)
		{
			ConDraw_Draw_OutText(AspectRatio , nDrawPos, ("^ ^ ^ ^"), ConDraw_Data.ConsolePosition);
			nStart--;
			nDrawPos++;
		}
	
		for(eg_int i=nStart; (i>=0) && (nDrawPos<NUM_LINES); i--, nDrawPos++)
		{
			strLine = Con->GetLineAt(i);
			ConDraw_Draw_OutText(AspectRatio , nDrawPos, strLine, ConDraw_Data.ConsolePosition);
		}
	}

	Con->Unlock();
}

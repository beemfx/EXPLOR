// (c) 2016 Beem Media

#include "EGLoader.h"
#include "ExMenu.h"
#include "EGList.h"
#include "EGLoader_Loadable.h"
#include "ExConsts.h"
#include "EGEngineConfig.h"
#include "ExCreditsData.h"
#include "EGFileData.h"
#include "EGSoundScape.h"

static const eg_real CREDITS_MENU_HEADER_POP_TIME = 7.f;

enum class ex_credit_node_t : eg_uint
{
	NONE,
	LEFT,
	RIGHT,
	CENTER,
	HEADER,
	BLOB,
	FUTURE_POP,
	FUTURE_HEADER_POP,
	HEADER_POP,

	COUNT,
};


static const struct exCreditTypeInfo
{
	ex_credits_line_t Type;
	eg_bool           bPop:1;
	ex_credit_node_t  Node1Type;
	eg_color32        Node1Color;
	ex_credit_node_t  Node2Type;
	eg_color32        Node2Color;
}
CreditsMenuTable[] =
{
	{ ex_credits_line_t::Delay           , false , ex_credit_node_t::NONE              , eg_color32(255,255,255) ,  ex_credit_node_t::NONE  , eg_color32(255,255,255) } ,
	{ ex_credits_line_t::Normal          , false , ex_credit_node_t::LEFT              , ExColor_HeaderMinor     ,  ex_credit_node_t::RIGHT , eg_color32(255,255,255) } ,
	{ ex_credits_line_t::Header          , false , ex_credit_node_t::HEADER            , ExColor_HeaderYellow    ,  ex_credit_node_t::NONE  , eg_color32(255,255,255) } ,
	{ ex_credits_line_t::Center          , false , ex_credit_node_t::CENTER            , eg_color32(252,255,255) ,  ex_credit_node_t::NONE  , eg_color32(255,255,255) } ,
	{ ex_credits_line_t::HeaderPop       , true  , ex_credit_node_t::HEADER_POP        , ExColor_HeaderYellow    ,  ex_credit_node_t::NONE  , eg_color32(255,255,255) } ,
	{ ex_credits_line_t::FutureHeaderPop , true  , ex_credit_node_t::FUTURE_HEADER_POP , ExColor_HeaderYellow    ,  ex_credit_node_t::NONE  , eg_color32(255,255,255) } ,
	{ ex_credits_line_t::Blob            , false , ex_credit_node_t::BLOB              , eg_color32(255,255,255) ,  ex_credit_node_t::NONE  , eg_color32(255,255,255) } ,
	{ ex_credits_line_t::FuturePop       , true  , ex_credit_node_t::FUTURE_POP        , eg_color32(255,255,255) ,  ex_credit_node_t::NONE  , eg_color32(255,255,255) } ,

};

const exCreditTypeInfo& ExCreditsMenu_GetInfoForType( ex_credits_line_t Type )
{
	for( eg_size_t i=0; i<countof(CreditsMenuTable); i++ )
	{
		if( CreditsMenuTable[i].Type == Type )
		{
			return CreditsMenuTable[i];
		}
	}

	assert( false );
	return CreditsMenuTable[0];
};

class ExCreditsMenu: public ExMenu , public ILoadable
{
	EG_CLASS_BODY( ExCreditsMenu , ExMenu )

private:
		
	struct exCreditNode: public IListable
	{
		EGUiWidget*      TextNode = nullptr;
		eg_loc_text       LocText = eg_loc_text( eg_crc("") );
		ExCreditsMenu*    OwnerMenu = nullptr;
		eg_color32        Color = eg_color32(255,255,255);
		ex_credit_node_t  Type = ex_credit_node_t::NONE;
		eg_real           LifeSeconds = 0.f;
		eg_bool           bInMotion = false;
		eg_bool           bPop = false;

		void Update( eg_real DeltaTime )
		{
			if( bInMotion )
			{
				if( bPop )
				{
					TextNode->SetOffset( EGUiWidget::eg_offset_t::POST , eg_transform( CT_Default ) );
					eg_real Alpha = 3.f*EGMath_CubicInterp( 0.f, 0.f, 1.f, 0.f, EGMath_GetMappedRangeValue( LifeSeconds , eg_vec2( 0.f, CREDITS_MENU_HEADER_POP_TIME ), eg_vec2( 0.f, 1.f ) ) );
					
					if( Alpha > 1.f )
					{
						if( Alpha > 2.f )
						{
							Alpha = 3.f - Alpha;
						}
						else
						{
							Alpha = 1.f;
						}
					}

					eg_color ColorVec( Color );
					ColorVec.a = Alpha;

					TextNode->SetPalette( 0 , ColorVec.ToVec4() );

					if( LifeSeconds > CREDITS_MENU_HEADER_POP_TIME )
					{
						OwnerMenu->FreeNode( this );
					}
				}
				else
				{
					eg_transform StartPose;
					eg_transform EndPose;
					StartPose = eg_transform::BuildTranslation( 0.f , -120.f , 0.f );
					EndPose = eg_transform::BuildTranslation( 0.f , 130.f , 0.f );

					static const eg_real DURATION = 16.f;

					if( LifeSeconds >= DURATION )
					{
						OwnerMenu->FreeNode( this );
					}
					else
					{
						eg_real BlendPct = EGMath_GetMappedRangeValue( LifeSeconds , eg_vec2( 0.f, DURATION ), eg_vec2( 0.f, 1.f ) );

						eg_transform FinalPose = eg_transform::Lerp( StartPose , EndPose , BlendPct );

						TextNode->SetOffset( EGUiWidget::eg_offset_t::POST , FinalPose );
					}
				}
				LifeSeconds += DeltaTime;
			}
		}
	};

private:

	EGArray<exCreditsLine> m_Credits = eg_mem_pool::DefaultHi; // The credits will get populated from another thread, but we're not going to use mutexes to do it because we know once the data is valid.
	EGArray<exCreditNode>  m_MasterNodeList; // Only holds the instances, does not manage anything.
	EGList<exCreditNode>   m_FreeNodes[ex_credit_node_t::COUNT];

	eg_size_t              m_NextCreditEvent;
	eg_real                m_TimeTillNextCreditEvent;
	eg_real                m_BaseCreditsTime = 0.f;
	eg_real                m_CreditsSpeed = 1.f;
	eg_real                m_DesiredDuration = 30.f;
	eg_real                m_SpeedAdjustment = 1.f;
	eg_bool                m_bCreditsReady;
	eg_bool                m_bSaveCredits = false;

public:
	ExCreditsMenu()
	: ExMenu()
	{ 
		for( eg_size_t i=0; i<countof(m_FreeNodes); i++ )
		{
			m_FreeNodes[i].Init( EGList<exCreditNode>::DEFAULT_ID ); // Not shared os all can be initialized with the same id.
		}
	}

	void FreeNode( exCreditNode* Node )
	{
		assert( Node->bInMotion );
		Node->bInMotion = false;
		Node->TextNode->SetVisible( false );
		if( Node )
		{
			switch( Node->Type )
			{
			case ex_credit_node_t::COUNT:
			case ex_credit_node_t::NONE:
				assert( false );
				break;
			default:
				m_FreeNodes[static_cast<eg_uint>(Node->Type)].Insert( Node );
				break;
			}
		}
	}

	void Init_TextNodes()
	{
		m_MasterNodeList.Reserve( 22*3 + 5 + 20 ); // Might as well avoid allocations since we know about how many there are.

		auto StoreTextNode = [this]( EGUiWidget* TextNode , ex_credit_node_t Type , eg_size_t Extras ) -> void
		{
			if( TextNode )
			{
				exCreditNode NewNode;
				NewNode.OwnerMenu = this;
				NewNode.Type = Type;
				NewNode.TextNode = TextNode;
				NewNode.TextNode->SetVisible( false );
				m_MasterNodeList.Append( NewNode );

				for( eg_size_t i=0; i<Extras; i++ )
				{
					EGUiWidget* Duplicate = DuplicateWidget( TextNode );
					if( Duplicate )
					{
						exCreditNode NewNode;
						NewNode.OwnerMenu = this;
						NewNode.Type = Type;
						NewNode.TextNode = Duplicate;
						NewNode.TextNode->SetVisible( false );
						m_MasterNodeList.Append( NewNode );
					}
				}
			}
		};

		EGUiWidget* Left    = GetWidget( eg_string_crc("Left"));
		EGUiWidget* Right   = GetWidget( eg_string_crc("Right"));
		EGUiWidget* Center  = GetWidget( eg_string_crc("Center"));
		EGUiWidget* Header  = GetWidget( eg_string_crc("Header"));
		EGUiWidget* Blob    = GetWidget( eg_string_crc("Blob"));
		EGUiWidget* FutureH = GetWidget( eg_string_crc("FutureHeaderPop"));
		EGUiWidget* HPop    = GetWidget( eg_string_crc("HeaderPop"));
		EGUiWidget* FutureP = GetWidget( eg_string_crc("FuturePop"));

		StoreTextNode( Left , ex_credit_node_t::LEFT , 21 );
		StoreTextNode( Right , ex_credit_node_t::RIGHT , 21 );
		StoreTextNode( Center , ex_credit_node_t::CENTER , 21 );
		StoreTextNode( Header , ex_credit_node_t::HEADER , 4 );
		StoreTextNode( Blob , ex_credit_node_t::BLOB , 8 );
		StoreTextNode( FutureH , ex_credit_node_t::FUTURE_HEADER_POP , 2 );
		StoreTextNode( HPop , ex_credit_node_t::HEADER_POP , 2 );
		StoreTextNode( FutureP , ex_credit_node_t::FUTURE_POP , 2 );


		//
		// We now have the master list so the pointers to the items in it won't change,
		// so we can populate the type lists.
		//
		for( exCreditNode& Node : m_MasterNodeList )
		{
			Node.bInMotion = true;
			FreeNode( &Node );
		}
	}

	void AddCreditToScreen( const exCreditsLine& Credit )
	{
		auto AddLocalizedNode = [&Credit]( EGList<exCreditNode>& List , eg_bool bIsLocText , const eg_color32& Color , eg_bool bPop ) -> void
		{
			exCreditNode* Node = List.GetOne(); // If this asserts we didn't have enough nodes on the stage.
			if( Node )
			{
				List.Remove( Node );
				assert( !Node->bInMotion );
				Node->bInMotion = true;
				Node->LocText = bIsLocText ? eg_loc_text(Credit.Title.Name) : eg_loc_text(Credit.Name.Name);
				Node->LifeSeconds = 0.f;
				Node->TextNode->SetVisible( true );
				Node->Color = Color;
				Node->TextNode->SetPalette( 0 , eg_color(Color).ToVec4() );
				Node->TextNode->SetText( eg_crc("") , Node->LocText );
				Node->bPop = bPop;
				Node->Update( 0.f ); // This will make it so it doesn't flash on in the wrong position when first created.
			}
		};

		const exCreditTypeInfo& Info = ExCreditsMenu_GetInfoForType( Credit.Type );

		eg_bool bIsLocText = true;

		if( Info.Node2Type != ex_credit_node_t::NONE )
		{
			bIsLocText = true; // Node 1 will be a localized title.
			AddLocalizedNode( m_FreeNodes[static_cast<eg_uint>(Info.Node2Type)] , false , Info.Node2Color , Info.bPop );
		}
		else
		{
			bIsLocText = Credit.Title.Name != eg_crc("");
		}

		if( Info.Node1Type != ex_credit_node_t::NONE )
		{
			AddLocalizedNode( m_FreeNodes[static_cast<eg_uint>(Info.Node1Type)] , bIsLocText , Info.Node1Color , Info.bPop );
		}
	}

	virtual void OnInit() override final
	{	
		if( m_bSaveCredits )
		{
			EGCrcDb::Init();
		}

		Super::OnInit();		
		assert( !m_bCreditsReady );
		m_bCreditsReady = false;
		MainLoader->BeginLoad( GAME_DATA_PATH "/gamedata/CreditsCollection.egasset" , this , EGLoader::LOAD_THREAD_MAIN );

		Init_TextNodes();

		if( EGSoundScape* SoundScape = GetPrimaryClient() ? GetPrimaryClient()->SDK_GetSoundScape() : nullptr )
		{
			SoundScape->PushBgMusic("/Music/Credits");
			SoundScape->ResetBgMusicTrack();
		}
	}

	virtual void OnDeinit() override final
	{
		Super::OnDeinit();
		if( !m_bCreditsReady )
		{
			MainLoader->CancelLoad( this );
		}

		if( m_bSaveCredits )
		{
			EGCrcDb::Deinit();
		}

		if( EGSoundScape* SoundScape = GetPrimaryClient() ? GetPrimaryClient()->SDK_GetSoundScape() : nullptr )
		{
			SoundScape->PopBgMusic();
		}
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final
	{
		Super::OnUpdate( DeltaTime , AspectRatio );

		DeltaTime *= m_CreditsSpeed*m_SpeedAdjustment;

		for( exCreditNode& Node : m_MasterNodeList )
		{
			Node.Update( DeltaTime );
		}

		if( m_bCreditsReady )
		{
			m_TimeTillNextCreditEvent -= DeltaTime;
			if( m_TimeTillNextCreditEvent < 0.f )
			{
				if( m_Credits.IsValidIndex( m_NextCreditEvent ) )
				{
					const exCreditsLine& Credit = m_Credits[m_NextCreditEvent];
					m_TimeTillNextCreditEvent = Credit.DelayTime;
					AddCreditToScreen( Credit );
					m_NextCreditEvent++;
				}
				else
				{
					m_NextCreditEvent = 0;
					MenuStack_Pop();
				}
			}
		}

	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			MenuStack_Pop();
			return true;
		}

		return false;
	}

	//
	// ILoadable Interface
	//

	virtual void DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size) override final
	{
		m_Credits.Clear( false );
		m_BaseCreditsTime = 0.f;
		
		EGFileData Data( eg_file_data_init_t::SetableUserPointer );
		Data.SetData( pMem , Size );
		// eg_uint64 DeserializeStartTime = Timer_GetRawTime();
		ExCreditsCollection* CreditsData = EGDataAsset::LoadDataAsset<ExCreditsCollection>( Data , EGString_ToWide(strFile) );
		// eg_uint64 DeserializeEndTime = Timer_GetRawTime();
		// eg_real DeserializeSeconds = Timer_GetRawTimeElapsedSec( DeserializeStartTime , DeserializeEndTime );
		// EGLogf( eg_log_t::Warning , "Took %g seconds to deserialize." , DeserializeSeconds );
		if( CreditsData )
		{
			m_DesiredDuration = CreditsData->m_DesiredCreditsDurationSeconds;
			for( exCreditsDataInfo& CreditsInfo : CreditsData->m_CreditsDataInfo )
			{
				if( CreditsInfo.LoadedCreditsData )
				{
					m_Credits.Append( CreditsInfo.LoadedCreditsData->m_Lines );
				}
			}
			EGDeleteObject( CreditsData );
		}

		for( exCreditsLine& Line : m_Credits )
		{
			switch( Line.Type )
			{
				default:
					Line.DelayTime = 1.f;
					break;
				case ex_credits_line_t::Blob:
					Line.DelayTime = 2.3f;
					break;
				case ex_credits_line_t::HeaderPop:
				case ex_credits_line_t::FuturePop:
				case ex_credits_line_t::FutureHeaderPop:
				case ex_credits_line_t::Delay:
					break;
			}

			m_BaseCreditsTime += Line.DelayTime;
		}

		m_SpeedAdjustment = m_BaseCreditsTime/m_DesiredDuration;
	}

	virtual void OnLoadComplete(eg_cpstr strFile) override final
	{
		unused( strFile );
		m_bCreditsReady = true;

		if( m_bSaveCredits )
		{
			// SaveCredits();
		}

	}

	/*
	void SaveCredits()
	{
		ExCreditsData* CreditConv = EGCast<ExCreditsData>(EGDataAsset::CreateDataAsset( &ExCreditsData::GetStaticClass() , eg_mem_pool::DefaultHi , egRflEditor() ));
		if( CreditConv )
		{
			CreditConv->m_Lines.Resize( m_Credits.Len() );

			for( eg_size_t i=0; i<m_Credits.Len(); i++ )
			{
				exCreditsLine& Target = CreditConv->m_Lines[i];
				const exCredit& Source = m_Credits[i];

				Target.Name.Name = EGCrcDb::StringToCrc( EGString_ToMultibyte(Source.Name) );
				Target.Name.Name_enus = EGString_ToMultibyte(Source.Name);
				Target.Title.Name_enus = eg_string_big(EGString_ToMultibyte( eg_loc_text( Source.Title ).GetString() )).String();
				Target.Title.Name = Source.Title;
				Target.DelayTime = Source.DelayTillNext;

				switch( Source.Type )
				{
				case ex_credit_t::DELAY:
					Target.Type = ex_credits_line_t::Delay;
					Target.Comment = EGCrcDb::StringToCrc( "Delay" );
					break;
				case ex_credit_t::PAIR:
					Target.Type = ex_credits_line_t::Normal;
					Target.Comment = EGCrcDb::StringToCrc( "Credit" );
					break;
				case ex_credit_t::HEADER:
					Target.Type = ex_credits_line_t::Header;
					Target.Comment = EGCrcDb::StringToCrc( "Header" );
					break;
				case ex_credit_t::CENTER:
					Target.Type = ex_credits_line_t::Center;
					Target.Comment = EGCrcDb::StringToCrc( "Center Credit" );
					break;
				case ex_credit_t::FUTURE_POP:
					Target.Type = ex_credits_line_t::FuturePop;	
					Target.Comment = EGCrcDb::StringToCrc( "Future Pop Credit" );
					break;
				case ex_credit_t::HEADER_POP:
					Target.Type = ex_credits_line_t::HeaderPop;
					Target.Comment = EGCrcDb::StringToCrc( "Header Pop Credit" );
					break;
				case ex_credit_t::FUTURE_HEADER_POP:
					Target.Type = ex_credits_line_t::FutureHeaderPop;
					Target.Comment = EGCrcDb::StringToCrc( "Future Header Pop Credit" );
					break;
				case ex_credit_t::BLOB:
					Target.Type = ex_credits_line_t::Blob;
					Target.Comment = EGCrcDb::StringToCrc( "Blob" );
					break;
				}
			}

			EGDataAsset::SaveDataAsset( L"C:/projects/emergence/!credits.egasset" , CreditConv , EGReflection_GetEditor( *CreditConv , "DataAsset" ) );

			EGDeleteObject( CreditConv );
		}
	}
	*/
};

EG_CLASS_DECL( ExCreditsMenu )

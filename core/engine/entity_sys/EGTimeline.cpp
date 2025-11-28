// (c) 2017 Beem Media

#include "EGTimeline.h"
#include "EGCrcDb.h"
#include "EGParse.h"
#include "EGFileData.h"

EG_CLASS_DECL( EGTimeline )

void EGTimeline::ProcessTimeline( egTimelinePlayState& PlayState , IEGTimelineHandler* Handler ) const
{
	assert( m_bTimelineCompiled );
	if( !m_bTimelineCompiled )
	{
		PlayState.bIsDone = true;
		return;
	}

	// We begin our search from PlayState.NextToPlay, and play everything less than the current time.
	const eg_size_t NumKeyframes = m_Keyframes.Len();
	for( eg_uint i=PlayState.NextToPlay; i<NumKeyframes; i++ )
	{
		const egTimelineKeyframe& Keyframe = m_Keyframes[i];

		if( Keyframe.Time < PlayState.Time )
		{
			if( Handler )
			{
				for( eg_uint ActionIndex = 0; ActionIndex<Keyframe.NumActions; ActionIndex++ )
				{
					const egTimelineAction& Action = m_Actions[Keyframe.FirstAction+ActionIndex];
				
					Handler->OnTimelineAction( Action );
				}
			}
			PlayState.NextToPlay = i+1;
		}
		else
		{
			break;
		}
	}

	if( PlayState.NextToPlay >= NumKeyframes )
	{
		PlayState.bIsDone = true;
	}
}

void EGTimeline::SerializeToXml( class EGFileData& File, eg_cpstr PrefixTabs ) const
{
	auto Write = [&File]( auto... Args ) -> void
	{
		eg_char8 Buffer[1024];
		EGString_FormatToBuffer( Buffer , countof(Buffer) , Args... );
		File.WriteStr8( Buffer );
	};

	Write( "%s<timeline id=\"%s\" editor_frame_time=\"%g\">\r\n" , PrefixTabs , EGCrcDb::CrcToString(m_Id).String() , m_EditorSpacing );
	eg_bool bSameKeyframeAsLast = false;
	for( eg_size_t i=0; i<m_Keyframes.Len(); i++ )
	{
		const egTimelineKeyframe& Keyframe = m_Keyframes[i];
		Write( "%s\t<keyframe time=\"%g\">" , PrefixTabs , Keyframe.Time );

		const eg_d_string8 KeyframScriptXML = EGStringEx_ToXmlFriendly( *Keyframe.Script );
		File.WriteStr8( *KeyframScriptXML );

		Write( "</keyframe>\r\n" , PrefixTabs );
	}
	Write( "%s</timeline>\r\n" , PrefixTabs );
}

void EGTimeline::InsertKeyframe( const egTimelineKeyframe& NewKeyframe )
{
	m_Keyframes.Append( NewKeyframe ); 
	m_Keyframes.Sort();
	m_bTimelineCompiled = false;
}

void EGTimeline::EditorSetSpacing( eg_real Spacing )
{
	m_EditorSpacing = EG_Max( EG_SMALL_NUMBER , Spacing );
	EditorEvenlySpaceKeyframes();
}

void EGTimeline::EditorInsertAtAndRespace( eg_uint Index, const egTimelineKeyframe& NewKeyframe )
{
	m_Keyframes.InsertAt( Index , NewKeyframe );
	EditorEvenlySpaceKeyframes();
	m_bTimelineCompiled = false;
}

void EGTimeline::EditorDeleteKeyframeAndRespace( eg_uint Index )
{
	if( m_Keyframes.IsValidIndex( Index ) )
	{
		m_Keyframes.DeleteByIndex( Index );
		EditorEvenlySpaceKeyframes();
		m_bTimelineCompiled = false;
	}
}

void EGTimeline::EditorEvenlySpaceKeyframes()
{
	for( eg_size_t i=0; i<m_Keyframes.Len(); i++ )
	{
		m_Keyframes[i].Time = i*m_EditorSpacing;
	}
	m_bTimelineCompiled = false;
}

void EGTimeline::CompileScript()
{
	m_Keyframes.Sort();

	m_Actions.Clear();

	for( egTimelineKeyframe& Keyframe : m_Keyframes )
	{
		Keyframe.FirstAction = m_Actions.LenAs<eg_uint>();
		Keyframe.NumActions = 0;

		auto HandleFnCall = [this,&Keyframe]( const egParseFuncInfo& FnInfo ) -> void
		{
			egTimelineAction CompressedAction;
			CompressedAction.FnCall = FnInfo;

			if( CompressedAction.FnCall.IsValid() )
			{
				m_Actions.Append( CompressedAction );
				Keyframe.NumActions++;
			}
			else
			{
				EGLogf( eg_log_t::Error , "Timeline script call was invalid." );
			}
		};

		EGParse_ProcessFnCallScript( *Keyframe.Script , Keyframe.Script.Len() , HandleFnCall );
	}

	m_bTimelineCompiled = true;
}

void EGTimeline::OnTag( const eg_string_base& Tag, const EGXmlAttrGetter& AttGet )
{
	eg_string_crc TagAsCrc = eg_string_crc(Tag);

	switch_crc(TagAsCrc)
	{
		case_crc("timeline"):
		{
			m_Id = EGCrcDb::StringToCrc(AttGet.GetString("id"));
			if( AttGet.DoesAttributeExist( "editor_frame_time" ) )
			{
				m_bGotSpacing = true;
				m_EditorSpacing = AttGet.GetFloat( "editor_frame_time" , .5f );
			}
		} break;
	
		case_crc("keyframe"):
		{
			egTimelineKeyframe NewKeyframe;
			NewKeyframe.Time = AttGet.GetFloat("time");
			m_Keyframes.Append( NewKeyframe );

			if( !m_bGotSpacing && NewKeyframe.Time > 0.f )
			{
				m_bGotSpacing = true;
				m_EditorSpacing = NewKeyframe.Time;
			}

			assert( !m_bReadingKeyframeScript );
			m_bReadingKeyframeScript = true;
		} break;
	}
}

void EGTimeline::OnTagEnd( const eg_string_base& Tag )
{
	eg_string_crc TagAsCrc = eg_string_crc(Tag);

	switch_crc(TagAsCrc)
	{
		case_crc("timeline"):
		{
			m_Keyframes.Sort();
			CompileScript();
		} break;

		case_crc("keyframe"):
		{
			assert( m_bReadingKeyframeScript );
			m_bReadingKeyframeScript = false;
		} break;
	}
}

void EGTimeline::OnData( eg_cpstr Data, eg_uint Len )
{
	unused( Len );

	assert( Len == 0 || Data[Len] == 0 );

	if( m_bReadingKeyframeScript )
	{
		assert( m_Keyframes.Len() > 0 );
		m_Keyframes.Top().Script.Append( Data );
	}
}

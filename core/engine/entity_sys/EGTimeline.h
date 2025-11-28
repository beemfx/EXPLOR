// (c) 2017 Beem Media

#pragma once

#include "EGTimelineTypes.h"
#include "EGXMLBase.h"

class EGTimeline : public EGObject , public IXmlBase
{
	EG_CLASS_BODY( EGTimeline , EGObject )

public:

	eg_string_crc GetId() const { return m_Id; }
	eg_uint GetNumKeyframes() const { return m_Keyframes.LenAs<eg_uint>(); }
	egTimelineKeyframe& GetKeyframe( eg_uint Index ) { return m_Keyframes[Index]; }
	const egTimelineKeyframe& GetKeyframe( eg_uint Index ) const { return m_Keyframes[Index]; }
	void ProcessTimeline( egTimelinePlayState& PlayState , IEGTimelineHandler* Handler ) const;
	void SerializeToXml( class EGFileData& File , eg_cpstr PrefixTabs ) const;

	void SetId( eg_string_crc Id ){ m_Id = Id; }
	void InsertKeyframe( const egTimelineKeyframe& NewKeyframe );

	void EditorSetSpacing( eg_real Spacing );
	void EditorInsertAtAndRespace( eg_uint Index , const egTimelineKeyframe& NewKeyframe );
	void EditorDeleteKeyframeAndRespace( eg_uint Index );
	void EditorEvenlySpaceKeyframes();
	eg_real EditorGetSpacing() const { return m_EditorSpacing; }

	void CompileScript();

private:

	typedef EGArray<egTimelineKeyframe> EGKeyframesArray;
	typedef EGArray<egTimelineAction> EGActionsArray;

private:

	eg_string_crc    m_Id;
	eg_real          m_EditorSpacing = .5f;
	EGKeyframesArray m_Keyframes;
	EGActionsArray   m_Actions;
	eg_bool          m_bReadingKeyframeScript = false;
	eg_bool          m_bTimelineCompiled = false;
	eg_bool          m_bGotSpacing = false;

private:

	friend class EGEntDef;

	// BEGIN IXmlBase
	virtual void OnTag( const eg_string_base& Tag , const EGXmlAttrGetter& AttGet );
	virtual void OnTagEnd( const eg_string_base& Tag );
	virtual void OnData( eg_cpstr Data , eg_uint Len );
	virtual eg_cpstr XMLObjName()const{ return "EGTimeline"; }
	// END IXmlBase
};
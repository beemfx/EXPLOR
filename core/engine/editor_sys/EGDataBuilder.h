// (c) 2018 Beem Media

#pragma once

#if !defined( __EGEDITOR__ )
#pragma __ERROR__("Cannot include EGDataBuilder.h in non-editor builds.")
static_assert( false , "Cannot include EGDataBuilder.h in non-editor builds.h" );
#endif

class EGDataBuilder
{
private:

	eg_bool m_bIsDone = false;
	eg_string m_GameName = "";
	eg_string_big m_OutDir = "";
	eg_string_big m_SrcDir = "";

public:
	
	void ExecuteFirstStep( eg_cpstr GameName );
	void ExecuteNextStep( eg_real DeltaTime );
	eg_bool IsDone() const { return m_bIsDone; }
};
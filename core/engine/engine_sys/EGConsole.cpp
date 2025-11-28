//(c) 2011 Beem Software

#include "EGConsole.h"
#include "EGFileData.h"
#include "EGDebugText.h"

EGConsole MainConsole;

EGConsole::EGConsole()
: m_CmdHistoryPos(0)
, m_Lines()
, m_CmdCb( nullptr )
, m_CmdCbUserData( nullptr )
, m_bWasUpdated(true)
, m_bIsLocked(false)
{

}

EGConsole::~EGConsole()
{
	Clear();
}

void EGConsole::operator=( const EGConsole& rhs)
{
	rhs.Lock();

	m_Lines.Clear();

	for( eg_uint i=0; i<rhs.GetNumLines(); i++ )
	{
		InsertString( rhs.GetLineAt(i) );
		InsertString( "\n" );
	}

	rhs.Unlock();
}

void EGConsole::SetCmdLine(eg_cpstr strCmd)
{
	Lock();
	m_LineIn = strCmd;
	Unlock();
}

void EGConsole::SetCmdLineNextHistory()
{
	eg_string s;
	Lock();
	if(m_CmdHistory.HasItems())
	{
		m_CmdHistoryPos = (m_CmdHistoryPos+1)%(m_CmdHistory.Len()+1);
		if(0 == m_CmdHistoryPos)
		{
			s = ("");
		}
		else
		{
			s = m_CmdHistory[m_CmdHistory.Len()-1-(m_CmdHistoryPos-1)];
		}
	}
	Unlock();

	SetCmdLine(s);
}

eg_size_t EGConsole::GetCmdLineHistory( eg_string Out[] , eg_size_t OutSize )const
{
	Lock();
	eg_size_t NumOut = EG_Min<eg_size_t>( OutSize , m_CmdHistory.Len() );


	for( eg_uint i=0; i<NumOut; i++ )
	{
		Out[i] = m_CmdHistory[i];
		Out[i].AddSlashes();
	}
	Unlock();

	return NumOut;
}

void EGConsole::SetCmdLineHistory( eg_string In[] , eg_size_t InSize )
{
	Lock();

	m_CmdHistory.Clear();
	m_CmdHistoryPos = 0;

	for( eg_uint i=0; i<InSize; i++ )
	{
		eg_string Temp = In[i];
		Temp.RemoveSlashes();
		m_CmdHistory.Push( Temp );
	}

	Unlock();
}

void EGConsole::OnChar(eg_char c)
{
	Lock();

	if(c != 0)
	{
		switch(c)
		{
		case '\r':
			if(m_LineIn.Len() > 0)		
			{
				if( m_CmdHistory.Contains( m_LineIn ) )
				{
					m_CmdHistory.DeleteByItem( m_LineIn );
				}
				//We put commands in the command history.
				//This will help for correcting of incorrectly typed commands.
				if(m_CmdHistory.IsFull())
				{
					m_CmdHistory.DeleteByIndex(0);
				}
				m_CmdHistory.Append(m_LineIn);
				m_CmdHistoryPos = 0;
				if( nullptr != m_CmdCb )
				{
					m_CmdCb( m_LineIn , m_CmdCbUserData );
				}
			}
			m_LineIn.Append('\n');
			InsertString_Internal(m_LineIn);
			m_LineIn.Clear();
			break;
		case '\b':
			m_LineIn.ClampEnd(1);
			break;
		default:
			m_LineIn.Append(c);
			break;
		}
	}

	Unlock();
}

eg_bool EGConsole::WasUpdated()
{
	eg_bool bRes=m_bWasUpdated;
	m_bWasUpdated=false;
	return bRes;
}

void EGConsole::SetCommandCallback( CmdCbFn Cb , void* UserData )
{
	m_CmdCb = Cb;
	m_CmdCbUserData = UserData;
}

void EGConsole::InsertString(eg_cpstr strChars)
{
	Lock();
	InsertString_Internal(strChars);
	Unlock();
}

void EGConsole::InsertString_Internal(eg_cpstr strChars)
{
	//Make sure that at least one line exists.
	if(m_Lines.Len()==0)
		InsertNewLine();

	//This is a pretty raw way of appending a string to the
	//console. The basic idea is to output one character at
	//a time, and if a newline character or the maximum length
	//of the console is reached then a new line is created.
	while(*strChars)
	{
		if(*strChars=='\n')
		{
			InsertNewLine();
		}
		else if(m_Lines.Top().Len()==CON_WIDTH)
		{
			//Check to see if we had coloring.
			eg_bool bHasColor = false;
			eg_string Color;
			if(m_Lines.Top()[static_cast<eg_uint>(0)] == '&' && m_Lines.Top()[static_cast<eg_uint>(1)] == 'c' && m_Lines.Top()[static_cast<eg_uint>(2)] == '!')
			{
				bHasColor = true;
				Color = m_Lines.Top();
				Color.ClampTo(11);
			}
			InsertNewLine();
			if(bHasColor)
			{
				m_Lines.Top().Append(Color);
			}
			m_Lines.Top().Append(*strChars);
		}
		else if(*strChars=='\b')
		{
			m_Lines.Top().Append('_');
		}
		else if(*strChars=='\t')
		{
			//Convert tabs to spaces:
			InsertString_Internal(("   "));
		}
		else
		{
			//Not a special char, just append it to the last line.
			m_Lines.Top().Append(*strChars);
		}
			
		strChars++;
	}
	
	m_bWasUpdated = true;
}

void EGConsole::Clear()
{
	Lock();
	
	m_Lines.Clear();
	m_bWasUpdated=true;
	
	Unlock();
}

eg_uint EGConsole::GetNumLines()const
{
	assert(m_bIsLocked);
	return static_cast<eg_uint>(m_Lines.Len());
}

void EGConsole::Dump( EGFileData& MemFile )
{	
	Lock();
	for(eg_uint i=0; i<GetNumLines(); i++)
	{
		eg_string str(GetLineAt(i));
		DebugText_GetStringColorAndRemoveColorTag( &str );
		eg_char8 strMB[256]; 
		str.CopyTo(strMB, countof(strMB));
		MemFile.Write((void*)strMB, str.Len()*sizeof(eg_char8));
		MemFile.Write((void*)"\r\n", 2);
	}
	Unlock();
}

void EGConsole::Lock()const
{		
	m_ThreadLock.Lock();
	assert(!m_bIsLocked);
	m_bIsLocked = true;
}
void EGConsole::Unlock()const
{
	assert(m_bIsLocked);
	m_bIsLocked = false;
	m_ThreadLock.Unlock();
}

eg_cpstr EGConsole::GetLineAt(eg_uint nLine)const
{
	assert(m_bIsLocked);
	eg_cpstr strOut="";
	if(nLine<m_Lines.Len())
	{
		strOut = m_Lines[nLine];
	}
	
	return strOut;
}

eg_cpstr EGConsole::GetCmdLine()const
{
	assert(m_bIsLocked);
	return m_LineIn;
}

void EGConsole::InsertNewLine()
{
	//If the console is full, then erase the oldest line.
	if( m_Lines.Len() >= m_Lines.ARRAY_SIZE )
	{
		m_Lines.DeleteByIndex(0);
	}

	//Add a new blank line.
	m_Lines.Push((""));
}

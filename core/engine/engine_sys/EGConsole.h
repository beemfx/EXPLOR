/******************************************************************************
File: Console.h
Class: EGConsole
Purpose: Historically the console was a sort of operating system that
ran the game, in it's current state it is mostly a text buffer for storing
debug messages, and also a command line that passes commands to the game class,
rather than process commands itself (as it used to do).

(c) 2011 Beem Software
******************************************************************************/
#pragma once
#include "EGMutex.h"

class EGConsole
{
public:
	typedef void ( * CmdCbFn )( eg_cpstr StrCmd , void* UserData );
	static const eg_uint HISTORY_SIZE=5;

public:
	EGConsole();
	~EGConsole();

	//Thread safe functions:
	void OnChar(eg_char c); //Should only be called from the main thread, especially if a script is running.
	eg_bool WasUpdated();
	void InsertString(eg_cpstr strLine);
	void Clear(); //Clears all text in the console.
	void SetCmdLine(eg_cpstr strCmd);
	void SetCmdLineNextHistory();
	void Dump( class EGFileData& MemFile ); //Dump the history of the console into a text file.
	eg_size_t GetCmdLineHistory( eg_string Out[] , eg_size_t OutSize )const;
	void SetCmdLineHistory( eg_string In[] , eg_size_t InSize );
	
	//Lock and Unlock allow the calling of functions that aren't thread safe.
	void Lock()const;
	void Unlock()const;
	
	//Functions that require the console to be locked before calling:
	eg_cpstr GetLineAt(eg_uint nLine)const;
	eg_cpstr GetCmdLine()const;
	eg_uint GetNumLines()const;

	void operator=( const EGConsole& rhs);

	//If a command function is set, it will usually be called from OnChar. The 
	//Command function should not interact with the console at all (it's 
	//usually  best to cache the string and process it later.) 
	void SetCommandCallback( CmdCbFn Cb , void* UserData ); 

private:	

	//Console dimensions:
	static const eg_uint CON_WIDTH=79;

private:

	typedef EGFixedArray<eg_string,200> EGLineStack;
	typedef EGFixedArray<eg_string,HISTORY_SIZE> EGHistoryArray;

private:

	EGLineStack              m_Lines;
	eg_string                m_LineIn;
	mutable EGMutex          m_ThreadLock;
	EGHistoryArray           m_CmdHistory;
	eg_uint                  m_CmdHistoryPos;
	CmdCbFn                  m_CmdCb;
	void*                    m_CmdCbUserData;
	mutable eg_bool          m_bIsLocked:1;
	mutable eg_bool          m_bWasUpdated:1;
	
private:

	void InsertNewLine();
	void InsertString_Internal(eg_cpstr strLine);
};

extern EGConsole MainConsole;

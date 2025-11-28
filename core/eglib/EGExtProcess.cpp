// (c) 2018 Beem Media

#include "EGExtProcess.h"
#include "EGWindowsAPI.h"
#include "EGThread.h"
#include "EGThreadProc.h"

class EGExtProcessStdOutProc : public IThreadProc
{
private:

	static const DWORD BUFFER_SIZE = eg_string_big::STR_SIZE/2;

	HANDLE const  m_ReadHandle;	
	eg_char8      m_CharBuffer[BUFFER_SIZE];
	eg_string_big m_CurrentLine = CT_Clear;

public:

	EGExtProcessStdOutProc( HANDLE ReadHandleIn )
	: m_ReadHandle( ReadHandleIn )
	{

	}

	virtual void Update( eg_real DeltaTime ) override
	{
		unused( DeltaTime );

		FlushPipe( m_ReadHandle );
	}

	virtual void OnStop() override
	{
		FlushPipe( m_ReadHandle );
	}

	void FlushPipe( HANDLE ReadHandle )
	{
		if( ReadHandle )
		{			
			DWORD BytesAvailable = 0;
			if( PeekNamedPipe( ReadHandle , NULL , 0 , NULL , &BytesAvailable , NULL ) )
			{
				while( BytesAvailable > 0 )
				{
					const DWORD BytesToRead = EG_Min<DWORD>( BytesAvailable , BUFFER_SIZE-1 );
					BytesAvailable -= BytesToRead;
					DWORD dwRead = 0;
					if( ReadFile( ReadHandle , m_CharBuffer , BytesToRead , &dwRead , NULL ) )
					{
						m_CharBuffer[dwRead] = '\0';
						eg_string_big CurrentOut = m_CurrentLine;
						CurrentOut += m_CharBuffer;


						m_CurrentLine.Clear();
						for( DWORD i=0; i<CurrentOut.Len(); i++ )
						{
							if( CurrentOut[i] == '\n' || CurrentOut[i] == '\r' )
							{
								if( m_CurrentLine.Len() > 0 )
								{
									EGLogf( eg_log_t::General , "%s" , m_CurrentLine.String() );
								}
								m_CurrentLine = "";
							}
							else
							{
								m_CurrentLine += CurrentOut[i];
							}
						}
					}
				}
			}
		}
	}
};

class EGExtProcess
{
private:

	HANDLE m_StdOutRead = NULL;
	HANDLE m_StdOutWrite = NULL;

public:

	void CreatePipes()
	{
		SECURITY_ATTRIBUTES SecAttr; 
		SecAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
		SecAttr.bInheritHandle = TRUE; 
		SecAttr.lpSecurityDescriptor = NULL;

		{
			eg_bool bCreatedPipe = TRUE == CreatePipe( &m_StdOutRead , &m_StdOutWrite , &SecAttr , 0 );
			if( !bCreatedPipe )
			{
				EGLogf( eg_log_t::Error , "Failed to create STD OUT pipes." );
			}
			eg_bool bSetHandleInfo = TRUE == SetHandleInformation( m_StdOutRead , HANDLE_FLAG_INHERIT , 0 );
			if( !bSetHandleInfo )
			{
				EGLogf( eg_log_t::Error , "Was not able to set handle info for STD OUT pipes." );
			}
		}
	}

	void ClosePipes( EGExtProcessStdOutProc* PipeProcToFlush )
	{
		if( m_StdOutWrite )
		{
			CloseHandle( m_StdOutWrite );
			m_StdOutWrite = nullptr;
		}
		if( PipeProcToFlush )
		{
			PipeProcToFlush->FlushPipe( m_StdOutRead );
		}
		if( m_StdOutRead )
		{
			CloseHandle( m_StdOutRead );
			m_StdOutRead = nullptr;
		}
	}

	eg_bool Run( eg_cpstr CmdLine , eg_int* Result )
	{
		CreatePipes();

		EGThread PipeThread( EGThread::egInitParms( "Pipe Thread" , EGThread::eg_init_t::LowPriority ) );
		EGExtProcessStdOutProc PipeProc( m_StdOutRead );
		PipeThread.RegisterProc( &PipeProc );
		PipeThread.Start();

		//We'll flush the console so that this output appears after any output
		//that has been put out by this compiler.
		fflush(stdout);
		fflush(stderr);

		SECURITY_ATTRIBUTES SA;
		zero( &SA );
		SA.nLength = sizeof(SA);
		SA.bInheritHandle = TRUE;
		SA.lpSecurityDescriptor = NULL;

		STARTUPINFOA SI;
		zero(&SI);
		SI.cb = sizeof(SI);
		SI.dwFlags = STARTF_USESTDHANDLES;
		SI.hStdOutput = m_StdOutWrite; // ::GetStdHandle( STD_OUTPUT_HANDLE );
		SI.hStdError  = m_StdOutWrite;// ::GetStdHandle( STD_ERROR_HANDLE );
		SI.hStdInput  =::GetStdHandle( STD_INPUT_HANDLE );


		PROCESS_INFORMATION PI;
		zero(&PI);

		BOOL bSucc = CreateProcessA( NULL , const_cast<LPSTR>(CmdLine) , &SA , &SA , TRUE , CREATE_NO_WINDOW , NULL , NULL , &SI , &PI );

		if(bSucc)
		{
			DWORD Res = WaitForSingleObject( PI.hProcess , INFINITE );
			bSucc = SUCCEEDED(Res);
		}

		eg_int ResOut = -1;
		if( bSucc )
		{
			DWORD Res;
			bSucc = GetExitCodeProcess( PI.hProcess , &Res );
			if( bSucc )
			{
				ResOut = Res;
			}
			CloseHandle( PI.hProcess );
		}

		if(!bSucc)
		{
			EGLogf( eg_log_t::Error , __FUNCTION__ " Error: Failed to process the command \"%s\". It's possible that the executable is not in the PATH environment." , CmdLine );
		}

		if( Result )
		{
			*Result = ResOut;
		}

		PipeThread.Stop();
		PipeThread.UnregisterProc( &PipeProc );

		ClosePipes( &PipeProc );

		return TRUE == bSucc;
	}
};

eg_bool EGExtProcess_Run( eg_cpstr CmdLine , eg_int* Result )
{
	EGExtProcess Processor;
	return Processor.Run( CmdLine , Result );
}

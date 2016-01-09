

#include "stdafx.h"
#include "fsqar.h"
#include "simclient.h"
#include "resource.h"

CSimThread::CSimThread()
{
	m_hThread = NULL;
	m_hEventExit = NULL;
}

CSimThread::~CSimThread()
{
}

BOOL CSimThread::CreateThread(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter)
{
	DWORD dwThreadId;
	m_hEventExit = ::CreateEvent(NULL, TRUE, FALSE, _T("ExitEvent"));
	
	if (!m_hEventExit)
		return FALSE;
		
	m_hThread = ::CreateThread(NULL, 0, lpStartAddress, lpParameter, 0, &dwThreadId);
	//m_hThread = _beginthreadex(NULL, 0, lpStartAddress, lpParameter, 0, NULL);
	
	if (!m_hThread)
		return FALSE;
		
	TRACE1("Create SimClient Thread ID: %#x, Look these values when the program will be terminated.\n",
	          dwThreadId);
	// Dump all threads in the current process
	ListProcessThreads(GetCurrentProcessId());
	//SetThreadName(dwThreadId, "SimClientThread");
	// Force to set thread priority
	::SetThreadPriority(m_hThread, THREAD_PRIORITY_NORMAL);
	
	if (m_hEventExit)
		::ResetEvent(m_hEventExit);
		
	return TRUE;
}

BOOL CSimThread::ExitThread()
{
	if (m_hThread)
	{
		// Decrements a thread's suspend count
		::ResumeThread(m_hThread);
		
		if (::WaitForSingleObject(m_hThread, 500) != TERMINATION_TIMEOUT)
		{
			::CloseHandle(m_hThread);
			m_hThread = NULL;
			::CloseHandle(m_hEventExit);
			m_hEventExit = NULL;
			TRACE0("SimClient thread was terminated\n");
		}
	}
	
	return TRUE;
}


// ======================================================================
// The local variables of a function are unique to each thread that 
// runs the function. However, all threads in the process share the static 
// and global variables.
// 
// This instance of the application object belongs to the thread only 
// and it should be stored in the thread local storage. Thread local 
// storage enables multiple threads of the same process to use an index 
// allocated by the TlsAlloc function to store and retrieve a value that 
// is local to the thread. 
// ======================================================================

DWORD WINAPI SimClientThreadProc(LPVOID lpThreadParameter)
{
	ASSERT(lpThreadParameter);
	QARApplication* pApp = static_cast<QARApplication*>(lpThreadParameter);
	DWORD dwIndex = TlsAlloc();
	TlsSetValue(dwIndex, pApp);
	pApp->SimThreadProc(APP_NAME);
	TlsFree(dwIndex);
	return 0;
}

// ======================================================================
// This method belongs to an instance of our application object 
// that is working in the second thread.
// ======================================================================
DWORD QARApplication::SimThreadProc(LPCSTR pszSessionName)
{
	ASSERT(AfxIsValidString(pszSessionName));
	ASSERT(m_pSimClient);

	if (m_pSimClient->OpenConnection(pszSessionName))
	{
		m_pSimClient->Initialize();
		
		while (m_pSimClient->IsRunning())
		{
			m_pSimClient->CallDispatch(SimClientDispatchProc);
			Sleep(1);   // Introduce a delay for a lower resolution.
			
			if (m_pSimClient->IsTerminated())
				m_pSimClient->Quit();
		}


		// May be corruption here
		ASSERT(m_pSimClient);
		m_pSimClient->CloseConnection();
		ASSERT(m_pFrameWnd);
		m_pFrameWnd->SendMessage(WM_CLOSE, 0, 0);
		return 0;
	}
	else
	{
		Error(IDS_ERROR_CONNECT_TO_SIM);
		m_pFrameWnd->SendMessage(WM_DESTROY, 0, 0);
		return 0;
	}
}
//////////////////////////////////////////////////////////////////////////
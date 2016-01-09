

// ======================================================================= 
// Flight Simulator Quick Access Recorder and Analyzer (FSQAR)
// -----------------------------------------------------------------------
// 
// Created by AlexShag, 2014
//
// This program is free software: you can redistribute it and/or modify 
// it under the terms of the GNU GPL (see License.txt).
//
// WARNING:
// This is a very bad code! This is not C++ code but "C with objects" one. 
// It breaks many basic principles of Objected-Oriented Programming.  
// It is badly designed and ugly written. I don’t advise to use it directly 
// without any modifications. This code may be used only as a starting point 
// for developing more advanced application.
//
// See more details in Readme.txt
// 
// =======================================================================

#include "stdafx.h"
#include "fsqar.h"
#include "simclient.h"
#include "aboutgpl.h"
#include "registry.h"
#include "resource.h"


// Debug version of CRT library uses this operator
#ifdef _DEBUG
#ifndef DEBUG_NEW
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new DEBUG_NEW
#endif
#endif  // _DEBUG



//========================================================================
//					Callback functions
//========================================================================

LRESULT CALLBACK GDisplayWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK FrameWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK NDisplayWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK StatusWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SDisplayWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ControlPadWndProc(HWND, UINT, WPARAM, LPARAM);

DWORD WINAPI SimClientThreadProc(LPVOID lpThreadParameter);

CRITICAL_SECTION g_csAppData;

// Singleton implementation
QARApplication* volatile QARApplication::theApp = NULL;
LONG QARApplication::m_nRefCount = 0;

HINSTANCE QARApplication::m_hInstance = NULL;
LPCTSTR QARApplication::m_pszAppName = _T(APP_NAME);

QARApplication* QARApplication::GetInstance()
{
	QARApplication* volatile temp = theApp;

	// See Meyers and Alexandrescu
	// 'C++ and the Perils of Double-Checked Locking'
	MemoryBarrier();

	if (temp == NULL)
	{
		Lock lock(&g_csAppData);
		if (temp == NULL)
		{
			temp = new QARApplication();
			MemoryBarrier();
			theApp = temp;
			m_nRefCount++;
		}
		lock.Release();
	}
	
	return temp;
}

void QARApplication::FreeInstance()
{
	m_nRefCount--; 
	if(!m_nRefCount) 
	{
		delete theApp; 
		m_nRefCount = NULL; 
	} 
}
//////////////////////////////////////////////////////////////////////////

QARApplication::QARApplication() :  m_hActiveWnd(NULL)
{
	m_hInstance = NULL;
	
	m_pFrameWnd = new FrameWnd();
	m_pSimClient = new CSimClient();
}

QARApplication::~QARApplication()
{
	TRACE0("DTOR QARApplication\n");

	delete m_pFrameWnd;
	delete m_pSimClient;
}


void QARApplication::Error(UINT uResID)
{
	TCHAR szMsg[LONG_STR_LEN];
	HWND hWnd;

	if (m_pFrameWnd)
	{
		ASSERT_VALID(m_pFrameWnd);
		hWnd = m_pFrameWnd->GetSafeHwnd();
	}
	else
		hWnd = GetTopWindow(NULL);

	if (MyLoadString(uResID, (LPTSTR)szMsg, LONG_STR_LEN) != 0)
		MessageBox(hWnd, (LPTSTR)szMsg, AfxGetAppName(), 
			MB_OK | MB_ICONEXCLAMATION);
}

BOOL QARApplication::RegisterWindowClass(WNDPROC lpfnWndProc,  LPCTSTR pszIconName, LPCTSTR pszMenuName, LPCTSTR pszClassName)
{
	WNDCLASS wc;
	
	wc.style = CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc = lpfnWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	if (pszIconName)
		wc.hIcon = ::LoadIcon(m_hInstance, pszIconName);
	else
		wc.hIcon = NULL;
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = pszMenuName;
	wc.lpszClassName = pszClassName;

	if (!::RegisterClass(&wc))
	{
		Error(IDS_ERROR_CREATE_WINDOW);
		return FALSE;
	}

	return TRUE;
}



BOOL QARApplication::InitApplication(HINSTANCE hInstance)
{
	m_hInstance = hInstance;

	// Main Window
	RegisterWindowClass(FrameWndProc,
		MAKEINTRESOURCE(IDI_MAIN),
		MAKEINTRESOURCE(IDR_MAIN_MENU),
		szMainWndClass);

	// Display Window
	RegisterWindowClass(NDisplayWndProc, NULL, NULL, szNDisplayWndClass);

	// GDisplay Window
	RegisterWindowClass(GDisplayWndProc, NULL, NULL, szGDisplayWndClass);

	// Static data Display 
	RegisterWindowClass(SDisplayWndProc, NULL, NULL, szSDisplayWndClass);

	// ControlPad Window
	RegisterWindowClass(ControlPadWndProc, NULL, NULL, szControlPadWndClass);

	return TRUE;
}

BOOL QARApplication::InitInstance(LPTSTR lpszCmdLine,  int nCmdShow)
{

	RECT rcDefault;
	// Width of the screen of the primary display monitor 
	int cxScreen = GetSystemMetrics(SM_CXSCREEN);
	// Height of the screen of the primary display monitor 
	//int cyScreen = GetSystemMetrics(SM_CYSCREEN);
	// Height of a caption area 
	int cyCaption = GetSystemMetrics(SM_CYCAPTION);
	
	SetRect(&rcDefault, 
		cxScreen - FRAME_WIDTH, 
		cyCaption, 
		cxScreen, 
		cyCaption + FRAME_HEIGHT);

	// Check registry keys where stored position of the frame
	CRegistry Reg(HKEY_CURRENT_USER);
	if (!Reg.IsKeyExist(APP_MAIN_KEY))
		Reg.CreateKey(APP_MAIN_KEY);

	//GetUserOptions();


	ASSERT(m_pFrameWnd != NULL);
	__analysis_assume(m_pFrameWnd);


	if(!m_pFrameWnd->CreateEx(
		WS_EX_APPWINDOW|WS_EX_TOPMOST, 
		szMainWndClass, 
		AfxGetAppName(),
		dwMainWndStyle,
		rcDefault.left, rcDefault.top, 
		rcDefault.right - rcDefault.left, 
		rcDefault.bottom - rcDefault.top, NULL, NULL, m_pFrameWnd))
	{
		Error(IDS_ERROR_CREATE_WINDOW);
		return FALSE;
	}


	m_pFrameWnd->ShowWindow(nCmdShow);
	m_pFrameWnd->UpdateWindow();

	UNREFERENCED_PARAMETER(lpszCmdLine);
	return TRUE;
}



BOOL QARApplication::BeginSession()
{
#ifndef TEST_WITHOUT_SIM

	ASSERT(m_pSimClient);
	// Start SimConnect thread
	return m_pSimClient->BeginSession(&SimClientThreadProc, this);

#else
	// open file here
#endif // TEST_WITHOUT_SIM


}

void QARApplication::SendTerminationSignal()
{
#ifndef TEST_WITHOUT_SIM

	ASSERT(m_pSimClient);
	// Send signal to the SimConnect thread
	m_pSimClient->SendTerminationSignal();

	TRACE0("QARApplication::OnClose()\nSend termination signal to the working thread\n");
#else

	m_pFrameWnd->DestroyWindow();

#endif // TEST_WITHOUT_SIM
}


void QARApplication::EndSession()
{

#ifndef TEST_WITHOUT_SIM

	ASSERT(m_pSimClient);
	m_pSimClient->EndSession();

	// Dump all threads in the current process
	ListProcessThreads(GetCurrentProcessId());

#endif
}

void QARApplication::UpdateAllDisplays()
{
	GetGDisplay()->Update(&g_GraphData);
	GetNDisplay()->InvalidateRect(NULL, FALSE);
	GetSDisplay()->InvalidateRect(NULL, FALSE);
	//GetCtrlPad()->InvalidateRect(NULL, FALSE);
}




int QARApplication::Run()
{
	while (::GetMessage(&m_uMsg, NULL, 0, 0))
	{
		::TranslateMessage(&m_uMsg);
		::DispatchMessage(&m_uMsg);
	}

	return (INT)m_uMsg.wParam;
}


void QARApplication::OnSimContinue()
{
#ifndef TEST_WITHOUT_SIM

	ASSERT(m_pSimClient);
	m_pSimClient->SetPause(FALSE);

#else

	GDisplay* pDisplay = GetGDisplay();
	pDisplay->StartTimer();

#endif // TEST_WITHOUT_SIM

}


void QARApplication::OnSimPause()
{
#ifndef TEST_WITHOUT_SIM

	ASSERT(m_pSimClient);
	m_pSimClient->SetPause(TRUE);

#else

	GDisplay* pDisplay = GetGDisplay();
	pDisplay->StopTimer();

#endif // TEST_WITHOUT_SIM
}



void QARApplication::SetActiveWnd(enum CHILD_WINDOW ActiveWnd)
{
	switch (ActiveWnd)
	{
	case GDISPLAY:
		m_hActiveWnd = GetGDisplay()->GetSafeHwnd();
		break;

	case NDISPLAY:
		m_hActiveWnd = GetNDisplay()->GetSafeHwnd();
		break;

	case SDISPLAY:
		m_hActiveWnd = GetSDisplay()->GetSafeHwnd();
		break;
	}


	::ShowWindow(m_hActiveWnd, SW_SHOWNORMAL);
	::InvalidateRect(m_hActiveWnd, NULL, FALSE);
};

void QARApplication::SetCaption(LPCTSTR lpString)
{
	m_pFrameWnd->SetWindowText(lpString);
}

//////////////////////////////////////////////////////////////////////////
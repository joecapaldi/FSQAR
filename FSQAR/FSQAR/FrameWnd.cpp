// ======================================================================= 
// Flight Simulator Quick Access Recorder and Analyzer (FSQAR)
// -----------------------------------------------------------------------
// 
// Created by AlexShag, 2016
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
#include "registry.h"
#include "simclient.h"
#include "AboutGPL.h"
#include "resource.h"
#include "webcont.h"
#include "ADisplay.h"
#include "SimVarIDS.h"

#include "..\SaveJpeg\jpge.h"


// Debug version of CRT library uses this operator
#ifdef _DEBUG
#ifndef DEBUG_NEW
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new DEBUG_NEW
#endif
#endif  // _DEBUG

LRESULT CALLBACK AircraftOptionDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SimOptionDlgProc(HWND, UINT, WPARAM, LPARAM);

FrameWnd::FrameWnd()
{
	m_pCtrlPad = new ControlPad();
	m_pToolbar = new Toolbar();
	m_pTabWnd = new TabWindow();
	m_pStatus = new StatusBar();
	m_pNDisplay = new NDisplay();
	m_pGDisplay = new GDisplay();
	m_pSDisplay = new SDisplay();

	m_hMenu = NULL;
	m_uState = FALSE;
	m_fShowStatus = TRUE;
	m_fShowToolbar = TRUE;

#ifdef _DEBUG
	m_fTopMost = FALSE;
#else
	m_fTopMost = TRUE;
#endif // _DEBUG
}

FrameWnd::~FrameWnd()
{
	TRACE0("DTOR FrameWnd\n");

	delete m_pCtrlPad;
	delete m_pToolbar;
	delete m_pTabWnd;
	delete m_pStatus;
	delete m_pGDisplay;
	delete m_pNDisplay;
	delete m_pSDisplay;
}



BOOL FrameWnd::OnCreate()
{
	// IMPORTANT !!! Attach menu to the window
	m_hMenu = ::GetMenu(m_hWnd);

	// Read stored window position from registry
	if (!LoadWindowPos(&nWindowPosX, &nWindowPosY, &nWindowPosDX, &nWindowPosDY))
	{
		nWindowPosX = DEFAULT_WINDOWS_X;
		nWindowPosY = DEFAULT_WINDOWS_Y;
		nWindowPosDX = DEFAULT_WINDOWS_DX;
		nWindowPosDY = DEFAULT_WINDOWS_DY;
	}
	// Move window to the previous stored position
	MoveWindow(m_hWnd, nWindowPosX, nWindowPosY, nWindowPosDX, nWindowPosDY, TRUE);

	// Create status window
	ASSERT_VALID(m_pStatus);
	if(!m_pStatus->Create(this))
	{
		AfxGetApp()->Error(IDS_ERROR_CREATE_WINDOW);
		return FALSE;
	}
	m_pStatus->ShowStatus(0, _T("Ready..."));

#ifdef TEST_WITHOUT_SIM
	m_pStatus->ShowStatus(1, _T("Test without simulator !!!"));
#endif // TEST_WITHOUT_SIM

#ifndef _ALX_NO_TOOLBAR

	// Create tools window
	ASSERT_VALID(m_pToolbar);
	if (!m_pToolbar->Create(this))
	{
		AfxGetApp()->Error(IDS_ERROR_CREATE_WINDOW);
		return FALSE;
	}

#endif

	RECT rcClient;
	GetClientRect(&rcClient);

	ASSERT_VALID(m_pTabWnd);
	if (!m_pTabWnd->Create(this, &rcClient))
	{
		AfxGetApp()->Error(IDS_ERROR_CREATE_WINDOW);
		return FALSE;
	}

	// Create graphic display
	ASSERT_VALID(m_pGDisplay);
	if (!m_pGDisplay->Create(m_pTabWnd, &rcClient))
	{
		AfxGetApp()->Error(IDS_ERROR_CREATE_WINDOW);
		return FALSE;
	}

	// Set active tab window
	AfxGetApp()->SetActiveWnd(GDISPLAY);

	// Create dynamical data display
	ASSERT_VALID(m_pNDisplay);

	TabCtrl_AdjustRect(m_pTabWnd->GetSafeHwnd(), FALSE, &rcClient);

	if(!m_pNDisplay->Create(m_pTabWnd, &rcClient))
	{
		AfxGetApp()->Error(IDS_ERROR_CREATE_WINDOW);
		return FALSE;
	}

	// Hide dynamical data display window
	m_pNDisplay->ShowWindow(SW_HIDE);


	// Create statical data display
	ASSERT_VALID(m_pSDisplay);

	TabCtrl_AdjustRect(m_pTabWnd->GetSafeHwnd(), FALSE, &rcClient);

	if(!m_pSDisplay->Create(m_pTabWnd, &rcClient))
	{
		AfxGetApp()->Error(IDS_ERROR_CREATE_WINDOW);
		return FALSE;
	}

	// Hide statical data display window
	m_pNDisplay->ShowWindow(SW_HIDE);

	TabCtrl_AdjustRect(m_pTabWnd->GetSafeHwnd(), FALSE, &rcClient);


	SetOptions();	// Read and set user interface options

#ifndef TEST_WITHOUT_SIM
	// Start connection with the simulator.
	// Application object is responsible for interaction with the simulator.

	if (!AfxGetApp()->BeginSession())
	{
		AfxGetApp()->Error(IDS_ERROR_CREATE_THREAD);
		return FALSE;
	}

#endif // TEST_WITHOUT_SIM

#ifndef _ALX_NO_TRAYICON

	if (AfxGetApp()->m_pUserOptions->fUseTray)
	{
		TrayMessage(NIM_ADD, NULL, NULL);
		ChangeState();
	}

#endif

	return TRUE;
}

// This is a bad function!
// It makes assumption that statusbar and toolbar already are created
void FrameWnd::SetOptions()
{
	ReadOptions();

	CheckMenuItem(ID_SHOW_STATUSBAR, m_fShowStatus);
	CheckMenuItem(ID_SHOW_TOOLBAR,	 m_fShowToolbar);
	CheckMenuItem(ID_PLACE_TOP_MOST, m_fTopMost);

	ASSERT(m_pStatus);
	ASSERT(m_pToolbar);

	m_pStatus->ShowWindow(m_fShowStatus ? SW_SHOW : SW_HIDE);
	m_pToolbar->ShowWindow(m_fShowToolbar ? SW_SHOW : SW_HIDE);

	SetTopMost(m_fTopMost);

}

//------------------------------------------------------------------------
// The WM_CLOSE message is sent as a signal that a window or an application
// should terminate. An application can prompt the user for confirmation,
// prior to destroying a window, by processing the WM_CLOSE message and
// calling the DestroyWindow function only if the user confirms the choice.
//
//------------------------------------------------------------------------

void FrameWnd::OnClose()
{
#ifndef TEST_WITHOUT_SIM

	// Application object is responsible for interaction with the simulator.
	// Our application has m_pSimThread member. Let it sets the event.
	AfxGetApp()->SendTerminationSignal();

	// Destroy this window and all its child windows
	// see CDisplay::OnDestroy() to storing information in registry
	DestroyWindow();

#else
	DestroyWindow();

#endif // TEST_WITHOUT_SIM
}

//------------------------------------------------------------------------
// The WM_DESTROY message is sent when a window is being destroyed. It is
// sent to the window procedure of the window being destroyed after (!) the
// window is removed from the screen.
//------------------------------------------------------------------------

void FrameWnd::OnDestroy()
{
#ifndef TEST_WITHOUT_SIM

	// Say 'good-bye'
	AfxGetApp()->EndSession();

#endif // TEST_WITHOUT_SIM

	// Store position of the window in registry
	if (!SaveWindowPos(m_hWnd, nWindowPosX, nWindowPosY, 
		nWindowPosDX, nWindowPosDY))
	{
		TRACE0("FrameWnd::OnDestroy(): Failed to save window position\n");
	}

	SaveOptions();

#ifndef _ALX_NO_TRAYICON
	if (AfxGetApp()->m_pUserOptions->fUseTray)
	{
		TrayMessage(NIM_DELETE, NULL, NULL);
	}
#endif

}


// Save user interface options to registry
void FrameWnd::SaveOptions()
{
	CRegistry Reg(HKEY_CURRENT_USER);
	DWORD dw;

	Reg.OpenKey(APP_MAIN_KEY);

	dw = m_fShowStatus;
	Reg.WriteInteger(_T("ShowStatus"), &dw);
	dw = m_fShowToolbar;
	Reg.WriteInteger(_T("ShowToolbar"), &dw);
	dw = m_fTopMost;
	Reg.WriteInteger(_T("TopMost"), &dw);

	Reg.CloseKey();

}

// Read user interface options from registry
void FrameWnd::ReadOptions()
{
	CRegistry Reg(HKEY_CURRENT_USER);
	DWORD dw;

	Reg.OpenKey(APP_MAIN_KEY);

	if (Reg.ReadInteger(_T("ShowStatus"), &dw))
		m_fShowStatus = dw;

	if (Reg.ReadInteger(_T("ShowToolbar"), &dw))
		m_fShowToolbar = dw;

	if (Reg.ReadInteger(_T("TopMost"), &dw))
		m_fTopMost = dw;

	Reg.CloseKey();
}

void FrameWnd::OnSize(UINT nWidth, UINT nHeight)
{
	INT nStatusHeight = 0;
	INT nToolbarHeight = 0;

	// The toolbar and status window procedures automatically set 
	// the size and position of the toolbar and status windows.

	if (m_fShowStatus)
	{
		if (m_pStatus->GetSafeHwnd())
		{
			nStatusHeight = m_pStatus->GetWindowHeight();
			m_pStatus->SendMessage(WM_SIZE, 0, 0);
		}
	}


#ifndef _ALX_NO_TOOLBAR

	if (m_fShowToolbar)
	{
		if (m_pToolbar->GetSafeHwnd())
		{
			m_pToolbar->SendMessage(WM_SIZE, 0, 0);
			nToolbarHeight = m_pToolbar->GetWindowHeight();
		}
	}

#endif

	MoveWindow(m_pTabWnd->GetSafeHwnd(),
		0,
		nToolbarHeight,			// y
		nWidth, 
		nHeight - nStatusHeight-nToolbarHeight, // height
		TRUE);

	RECT rc;
	SetRect(&rc, 0, 0, nWidth, nHeight);
	TabCtrl_AdjustRect(m_pTabWnd->GetSafeHwnd(), 0, &rc);
	MoveWindow(m_pNDisplay->GetSafeHwnd(),
			rc.left,					// x
			rc.top,						// y
			rc.right - rc.left,			// width
			rc.bottom-rc.top - nStatusHeight-nToolbarHeight, // height
			TRUE);

	MoveWindow(m_pGDisplay->GetSafeHwnd(),
		rc.left,					// x
		rc.top,						// y
		rc.right - rc.left,			// width
		rc.bottom-rc.top - nStatusHeight-nToolbarHeight, // height
		TRUE);


	MoveWindow(m_pSDisplay->GetSafeHwnd(),
		rc.left,					// x
		rc.top,						// y
		rc.right - rc.left,			// width
		rc.bottom-rc.top - nStatusHeight-nToolbarHeight, // height
		TRUE);
}


BOOL FrameWnd::CreatePropertySheet(__in HINSTANCE hInstance)
{
	PROPSHEETPAGE psp[2];
	PROPSHEETHEADER psh;
	// Page#1
	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USEICONID | PSP_USETITLE;
	psp[0].hInstance = hInstance;
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PAGE_SCREEN);
	psp[0].pszIcon = MAKEINTRESOURCE(IDI_PAGE_SCREEN);
	psp[0].pfnDlgProc = (DLGPROC)SimOptionDlgProc;
	psp[0].pszTitle = MAKEINTRESOURCE(IDS_PAGE_SCREEN);
	psp[0].lParam = 0;
	psp[0].pfnCallback = NULL;
	// Page#2
	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags =  PSP_USEICONID | PSP_USETITLE;
	psp[1].hInstance = hInstance;
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_PAGE_FILE);
	psp[1].pszIcon = MAKEINTRESOURCE(IDI_PAGE_FILE);
	psp[1].pfnDlgProc = (DLGPROC)AircraftOptionDlgProc;
	psp[1].pszTitle = MAKEINTRESOURCE(IDS_PAGE_FILE);
	psp[1].lParam = 0;
	psp[1].pfnCallback = NULL;
	// Property Sheet
	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags =  PSH_USEICONID | PSH_PROPSHEETPAGE;
	psh.hwndParent = m_hWnd;
	psh.hInstance = hInstance;
	psh.pszIcon = MAKEINTRESOURCE(IDI_PROPERTIES);
	psh.pszCaption = MAKEINTRESOURCE(IDS_PROPERTIES_CAPTION);
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.nStartPage = 0;
	psh.ppsp = (LPCPROPSHEETPAGE)&psp;
	psh.pfnCallback = NULL;

	// Returns a positive value if successful, or -1 otherwise 
	// for modal property sheets.
	PropertySheet(&psh);

	return TRUE;
}

void FrameWnd::OnHelpAbout()
{
	PRODUCT_INFORMATION pi;
	StringCbCopy(pi.szProductName, SHORT_STR_LEN, _T("FSQAR"));
	StringCbCopy(pi.szProductDescription, LONG_STR_LEN, 
		_T("The Flight Simulator Quick Access Recorder"));
	StringCbCopy(pi.szAuthors, LONG_STR_LEN, _T("Alex Shag"));
	pi.nMajorVersion = APP_VERSION_MAJOR;
	pi.nMinorVersion = APP_VERSION_MINOR;
	pi.nRevision = APP_VERSION_REVISION;
	pi.hIcon = MyLoadIcon(MAKEINTRESOURCE(IDI_MAIN));
	AboutFreewareDialog(m_hWnd, &pi, NULL, 0);

}
void FrameWnd::OnHelpIndex()
{
	if (HtmlHelp(m_hWnd, _T("FSQAR.chm"), HH_DISPLAY_TOPIC, 0) == NULL)
		AfxGetApp()->Error(IDS_CANNOT_DISPLAY_HELP);
}

void FrameWnd::OnFileExit()
{
	OnClose();
}

void FrameWnd::OnViewOptions()
{
	CreatePropertySheet(AfxGetInstanceHandle());
}


void FrameWnd::OnControlPad()
{
	RECT rc1;


	if (IsZoomed(m_hWnd))
	{
		int height = 400, width = 400;
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		SetRect(&rc1, rc.left+100, rc.top+100, rc.left+width, rc.top+height);

	} 
	else
	{
		int height = 200, width = 200;
		RECT rc;
		// Width of the screen of the primary display monitor 
		int cxScreen = GetSystemMetrics(SM_CXSCREEN);
		// Height of the screen of the primary display monitor 
		int cyScreen = GetSystemMetrics(SM_CYSCREEN);
		GetWindowRect(m_hWnd, &rc);
		int x = rc.left+100;
		if ((x+width) >= cxScreen)
			x = cxScreen - width;

		int y = rc.top-200;
		if ( y < 0)
			y = 0;

		if ((y+height) >= cyScreen)
			y = cyScreen;

		SetRect(&rc1, x, y, x+width, y+height);
	}

	if (!m_pCtrlPad->Create(this, &rc1))
	{
		AfxGetApp()->Error(IDS_ERROR_CREATE_WINDOW);
		return;
	}
	m_pCtrlPad->ShowWindow(SW_SHOW);
	m_pCtrlPad->UpdateWindow();

}

//------------------------------------------------------------------------
// This function returns C++ object by Window handle
// It is similarly to CHadleMap
//------------------------------------------------------------------------
Window* FrameWnd::GetChildWindow(enum CHILD_WINDOW childWindow) const
{
	Window* pRes = NULL;
	switch (childWindow)
	{
	case GDISPLAY:
		pRes = GetGDisplay();
		break;

	case NDISPLAY:
		pRes = GetNDisplay();
		break;

	case SDISPLAY:
		pRes = GetSDisplay();
		break;

	case TABWINDOW:
		pRes = GetTabWindow();
		break;

	case STATUSBAR:
		pRes = GetStatus();
		break;

	case CTRLPAD:
		pRes = GetCtrlPad();
		break;
	}

	return pRes;
};

BOOL FrameWnd::TrayMessage(DWORD dwMessage, HICON hIcon, LPCTSTR pszText)
{
	BOOL retCode = FALSE;
	NOTIFYICONDATA nid;

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = m_hWnd;
	nid.uID = IDC_NOTIFY;
	nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP|NIF_INFO;
	nid.uCallbackMessage = WM_NOTIFYICON;
	nid.hIcon = hIcon;
	
	TCHAR szTitle[MAX_PATH];
	size_t nSize;

	if(MyLoadString(IDS_TRAY_TITLE, szTitle, MAX_PATH))
	{
		if(SUCCEEDED(StringCbLength(szTitle, MAX_PATH, &nSize)))
		{
			StringCbCopy(nid.szInfoTitle, 63, szTitle);
		}
	}

	if(MyLoadString(IDS_TRAY_TEXT, szTitle, MAX_PATH))
	{
		if(SUCCEEDED(StringCbLength(szTitle, MAX_PATH, &nSize)))
		{
			StringCbCopy(nid.szInfo, MAX_PATH, szTitle);
		}
	}


	nid.uTimeout = 1000;
	nid.dwInfoFlags = NIIF_USER;


	if (pszText)
	{
		size_t s;
		if (SUCCEEDED(StringCbLength(pszText, LONG_STR_LEN, &s)))
			StringCbCopy(nid.szTip, 64, pszText);
	} 
	else
	{
		nid.szTip[0] = _T('\0');
	}

	// Sends a message to the taskbar's status area
	retCode = Shell_NotifyIcon(dwMessage, &nid);

	if (hIcon)
	{
		DestroyIcon(hIcon);
	}

	return retCode;
}

void FrameWnd::ChangeState()
{
	TCHAR szTip[LONG_STR_LEN];
	LPTSTR pszIDIcon;
	HICON hIcon;

	if (!MyLoadString(m_uState == 0 ? IDS_STATE1: IDS_STATE2, szTip, LONG_STR_LEN))
	{
		*szTip = _T('\0');
	}

	pszIDIcon = MAKEINTRESOURCE(m_uState ? IDI_STATE1 : IDI_STATE2);

	hIcon = MyLoadIcon(pszIDIcon);

	TrayMessage(NIM_MODIFY, hIcon, szTip);
}

void FrameWnd::SetChartType(enum CHART_TYPE nType)
{
	m_pGDisplay->SetChartType(nType);
}

void FrameWnd::OnTabChange()
{ 

	int selection = TabCtrl_GetCurSel(AfxGetApp()->GetTabWindow()->GetSafeHwnd()); 
	::ShowWindow(AfxGetApp()->GetActiveWnd(), SW_HIDE);
	AfxGetApp()->SetActiveWnd(static_cast<enum CHILD_WINDOW>(selection));
}


void FrameWnd::OnMouseWheel(WPARAM wParam)
{
	short zDelta = static_cast<short>(HIWORD(wParam));
	m_pNDisplay->SendMessage(WM_VSCROLL, 
		zDelta < 0? SB_LINEDOWN: SB_LINEUP, 0);
}


void FrameWnd::OnKeyDown(WPARAM wParam)
{
	WORD wScrollNotify = 0xFFFF;
	switch (wParam) 
	{ 
	case VK_UP: 
		wScrollNotify = SB_LINEUP; 
		break; 

	case VK_PRIOR: 
		wScrollNotify = SB_PAGEUP; 
		break; 

	case VK_NEXT: 
		wScrollNotify = SB_PAGEDOWN; 
		break; 

	case VK_DOWN: 
		wScrollNotify = SB_LINEDOWN; 
		break; 

	case VK_HOME: 
		wScrollNotify = SB_TOP; 
		break; 

	case VK_END: 
		wScrollNotify = SB_BOTTOM; 
		break; 
	} 

	if (wScrollNotify != -1) 
		m_pNDisplay->SendMessage(WM_VSCROLL, MAKELONG(wScrollNotify, 0), 0L); 
}

void FrameWnd::OnHelp()
{

}

void FrameWnd::OnScreenshot()
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH] = TEXT("\0");
	TCHAR szMyPictures[MAX_PATH];

	// Get 'MyPictures' folder location for current user
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYPICTURES | CSIDL_FLAG_CREATE,
		NULL, 0, szMyPictures)))
	{
		StringCbCopy(szFile, MAX_PATH, TEXT(""));

		memset(&ofn, 0, sizeof(OPENFILENAME));

		// Fill in the ofn structure to support a template and hook.
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = m_hWnd;
		ofn.hInstance = AfxGetInstanceHandle();
		ofn.lpstrFilter = _T("JPEG image files (*.jpg)\0*.jpg\0");
		ofn.lpstrDefExt = TEXT("*.jpg");
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = szMyPictures;
		ofn.Flags = OFN_EXPLORER  | OFN_HIDEREADONLY |OFN_OVERWRITEPROMPT;

		if (GetSaveFileName(&ofn))
		{
			GetGDisplay()->SaveAsJpeg(ofn.lpstrFile);
		}
	}
}



void FrameWnd::OnNotify(WPARAM wParam, LPARAM lParam)
{

	LPTOOLTIPTEXT ttStr;
	TCHAR szString[SHORT_STR_LEN];

	ttStr = (LPTOOLTIPTEXT)lParam;

	// Must be display tooltips
	if (ttStr->hdr.code == TTN_NEEDTEXT)
	{
		switch (ttStr->hdr.idFrom)
		{
		case ID_SIM_CONTINUE:
			if (MyLoadString(IDS_SHORT_SIM_CONTINUE, szString, SHORT_STR_LEN))
			StringCbCopy(ttStr->lpszText, SHORT_STR_LEN, szString);
			break;

		case ID_SIM_PAUSE:
			if (MyLoadString(IDS_SHORT_SIM_PAUSE, szString, SHORT_STR_LEN))
			StringCbCopy(ttStr->lpszText, SHORT_STR_LEN,  szString);
			break;

		case ID_CONTROL_PAD:
			if (MyLoadString(IDS_SHORT_CONTROL_PAD, szString, SHORT_STR_LEN))
			StringCbCopy(ttStr->lpszText, SHORT_STR_LEN,  szString);
			break;

		case ID_SIM_QUIT:
			if (MyLoadString(IDS_SHORT_SIM_QUIT, szString, SHORT_STR_LEN))
			StringCbCopy(ttStr->lpszText, SHORT_STR_LEN,  szString);
			break;

		case ID_CHART_SIMPLE:
			if (MyLoadString(IDS_SHORT_CHART_SIMPLE, szString, SHORT_STR_LEN))
			StringCbCopy(ttStr->lpszText, SHORT_STR_LEN,  szString);
			break;

		case ID_CHART_LANDING:
			if (MyLoadString(IDS_SHORT_CHART_LANDING, szString, SHORT_STR_LEN))
			StringCbCopy(ttStr->lpszText, SHORT_STR_LEN,  szString);
			break;

		case ID_CHART_ANGLES:
			if (MyLoadString(IDS_SHORT_CHART_ANGLES, szString, SHORT_STR_LEN))
			StringCbCopy(ttStr->lpszText, SHORT_STR_LEN,  szString);
			break;

		case ID_CHART_PITCH:
			if (MyLoadString(IDS_SHORT_CHART_PITCH, szString, SHORT_STR_LEN))
			StringCbCopy(ttStr->lpszText, SHORT_STR_LEN,  szString);
			break;

		case ID_CHART_ROLL:
			if (MyLoadString(IDS_SHORT_CHART_ROLL, szString, SHORT_STR_LEN))
			StringCbCopy(ttStr->lpszText, SHORT_STR_LEN,  szString);
			break;

		case ID_CHART_YAW:
			if (MyLoadString(IDS_SHORT_CHART_YAW, szString, SHORT_STR_LEN))
			StringCbCopy(ttStr->lpszText,  SHORT_STR_LEN, szString);
			break;

		case ID_CHART_THRUST:
			if (MyLoadString(IDS_SHORT_CHART_THRUST, szString, SHORT_STR_LEN))
				StringCbCopy(ttStr->lpszText,  SHORT_STR_LEN, szString);
			break;

		case ID_SCREENSHOOT:
			if (MyLoadString(IDS_SHORT_SCREENSHOOT, szString, SHORT_STR_LEN))
			StringCbCopy(ttStr->lpszText,  SHORT_STR_LEN, szString);
			break;
		}
		//end toltips for toolbar
	}

	UNREFERENCED_PARAMETER(wParam);
}

inline void FrameWnd::Resize()
{
	RECT rc;
	GetClientRect(&rc);
	OnSize(rc.right - rc.left, rc.bottom - rc.top);
}

void FrameWnd::OnViewToolbar()
{
	if (m_fShowToolbar)
	{
		CheckMenuItem(ID_SHOW_TOOLBAR, FALSE);
		m_fShowToolbar = FALSE;
		m_pToolbar->ShowWindow(SW_HIDE);
	}
	else
	{
		CheckMenuItem(ID_SHOW_TOOLBAR, TRUE);
		m_fShowToolbar = TRUE;
		m_pToolbar->ShowWindow(SW_SHOW);
	}

	Resize();

}

void FrameWnd::OnViewStatus()
{
	if (m_fShowStatus)
	{
		CheckMenuItem(ID_SHOW_STATUSBAR, FALSE);
		m_fShowStatus = FALSE;
		m_pStatus->ShowWindow(SW_HIDE);
	}
	else
	{
		CheckMenuItem(ID_SHOW_STATUSBAR, TRUE);
		m_fShowStatus = TRUE;
		m_pStatus->ShowWindow(SW_SHOW);
	}

	Resize();
}


void FrameWnd::OnPlaceTopMost()
{
	if (m_fTopMost)
	{
		CheckMenuItem(ID_PLACE_TOP_MOST, FALSE);
		m_fTopMost = FALSE;
	}
	else
	{
		CheckMenuItem(ID_PLACE_TOP_MOST, TRUE);
		m_fTopMost = TRUE;
	}
	
	SetTopMost(m_fTopMost);
}

LRESULT CALLBACK StartupDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL hResData = NULL;

	switch (Msg)
	{
	case WM_INITDIALOG:
		{
			BOOL fShowDialog = TRUE;
			CRegistry Reg(HKEY_CURRENT_USER);
			DWORD dw;

			Reg.OpenKey(APP_MAIN_KEY);
			if (Reg.ReadInteger(_T("FirstStart"), &dw))
				fShowDialog = dw;
			Reg.CloseKey();

			CheckDlgButton(hDlg, IDC_SHOW_DIALOG, fShowDialog ? TRUE : FALSE);
			CenterWindow(hDlg, GetDesktopWindow());

			// Find HTML page in resource
			HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_FSQAR_ABOUT), RT_HTML);
			if (hRsrc)
			{
				hResData = LoadResource(AfxGetInstanceHandle(), hRsrc);
				if (hResData)
				{
					LPVOID pHTML = LockResource(hResData);
					// The pointer pHTML must be converted into const char*
					if (pHTML)
						DisplayHTMLStr(GetDlgItem(hDlg, IDC_WEB_CONTROL), (LPCSTR)pHTML);
				}

			}

		}
		
		return TRUE;

	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			{
				BOOL fShowDialog;
				fShowDialog = IsDlgButtonChecked(hDlg, IDC_SHOW_DIALOG);
				CRegistry Reg(HKEY_CURRENT_USER);
				DWORD dw;
				Reg.OpenKey(APP_MAIN_KEY);
				dw = fShowDialog;
				Reg.WriteInteger(_T("FirstStart"), &dw);
				Reg.CloseKey();
				if (hResData)
				{
					FreeResource(hResData);
				}
				
				EndDialog(hDlg, TRUE);
			}

			break;

			// FOR COMMERCICAL USE
		//case IDC_HELP_DETAILS:
		//	{
		//		if (HtmlHelp(hDlg, _T("FSQAR.chm"), HH_DISPLAY_TOPIC, 0) == NULL)
		//			AfxGetApp()->Error(IDS_CANNOT_DISPLAY_HELP);
		//		
		//	}
		//	break;

		case IDCANCEL:
			if (hResData)
			{
				FreeResource(hResData);
			}
			EndDialog(hDlg, TRUE);
			break;
		}
		return TRUE;
	}

	UNREFERENCED_PARAMETER(hDlg);
	UNREFERENCED_PARAMETER(Msg);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	return FALSE;
}


void FrameWnd::ShowStartupDialog()
{
	BOOL fShowDialog = TRUE;
	CRegistry Reg(HKEY_CURRENT_USER);
	DWORD dw;

	Reg.OpenKey(APP_MAIN_KEY);
	if (Reg.ReadInteger(_T("FirstStart"), &dw))
		fShowDialog = dw;

	Reg.CloseKey();

	if (fShowDialog)
	{
		DialogBox(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD_STARTUP), 
			m_hWnd, (DLGPROC)StartupDlgProc);
	}
}

void FrameWnd::OnManageOutputFolder()
{
	DialogBox(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD_OUTPUT_FOLDER),
		m_hWnd, (DLGPROC)OutputFolderDlgProc);
}

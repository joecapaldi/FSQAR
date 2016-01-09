
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
#include <zmouse.h>
#include "fsqar.h"
#include "webcont.h"
#include "SimClient.h"
#include "SimVarIDS.h"

#include "resource.h"

// Debug version of CRT library uses this operator
#ifdef _DEBUG
#ifndef DEBUG_NEW
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new DEBUG_NEW
#endif
#endif  // _DEBUG


inline QARApplication* AfxGetApp()
{
	return QARApplication::GetInstance();
}


inline void AfxFreeApp()
{
	return QARApplication::FreeInstance();
}

//inline 
HINSTANCE AfxGetInstanceHandle()
{
	return AfxGetApp()->Instance();
}

inline LPCTSTR AfxGetAppName()
{
	return AfxGetApp()->Name();
}



int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
					 LPTSTR lpszCmdLine, int nCmdShow)
{
	// It is need for memory leaks detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	TRACE0("===================== FSX_QAR STARTED ============================\n");

	// Output information about the process
	TRACE2("The program [%#x] 'QAR.exe', MainThread ID: %#x.\n",
	          GetCurrentProcessId(), GetCurrentThreadId());

	// Dump all threads in the current process
	ListProcessThreads(GetCurrentProcessId());

	INITCOMMONCONTROLSEX cs;
	cs.dwSize = sizeof(INITCOMMONCONTROLSEX);
	cs.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES | 
		ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&cs);

	// According to Rihter's advise: 400 (= 0x80000400)
	if (!InitializeCriticalSectionAndSpinCount(&g_csSimData, 0x80000400))
		return 0;

	if (!InitializeCriticalSectionAndSpinCount(&g_csAppData, 0x80000400))
		return 0;

	// Create instances of global objects
	//g_pSim = new CSimulator();

	// Find simulators which are installed on the machine
	//g_pSim->FindSimulator(P3D30);


	QARApplication* pApp = AfxGetApp();

	if (!hPrevInstance)
		if (!pApp->InitApplication(hInstance))
			return 0;
			
	if (!pApp->InitInstance(lpszCmdLine, nCmdShow))
		return 0;
	
	pApp->GetMainWnd()->ShowStartupDialog();

	pApp->Run();
	
	// Delete instances of global objects
	//delete g_pSim;
	AfxFreeApp();

	DeleteCriticalSection(&g_csAppData);
	DeleteCriticalSection(&g_csSimData);

	// Dump all threads in the current process
	ListProcessThreads(GetCurrentProcessId());
	TRACE0("===================== QAR FINISHED ============================\n");

	_CrtDumpMemoryLeaks();
	return 0;
}

//-------------------------------------------------------------------------
// Print all variable in the program into a file vars_dump.txt
// Then open the file with MS Word, select all and convert into a table
//-------------------------------------------------------------------------
static void DumpVariables()
{
	FILE* file = NULL;
	fopen_s(&file, "vars_dump.txt", "wt");
	if (file)
	{
		TCHAR szUniName[LONG_STR_LEN];
		static SimData* pVars = SimData::GetSimData();
		for (int k = 0; k < SIM_VARS_COUNT; k++)
		{
			UINT id = SIMVAR_BASE_ID + k;
			if(MyLoadString(id, szUniName, LONG_STR_LEN))
			{
				// MyLoadString gets Unicode resource string
				CHAR szAnsiName[LONG_STR_LEN];
				
#ifndef _UNICODE
				StringCbCopy(szAnsiName, LONG_STR_LEN, szUniName);
#else
				_wcstombsz(szAnsiName, szUniName, LONG_STR_LEN);
#endif
				
				INT_PTR nPosition = id - SIMVAR_BASE_ID;
				char* buffer[MAX_PATH];
				StringCbPrintfA((STRSAFE_LPSTR)buffer, MAX_PATH, 
					"%d\t%s\t%s\t%s\n", 
					nPosition,
					pVars->GetVariable(nPosition)->DatumName,
					szAnsiName,
					pVars->GetVariable(nPosition)->UnitsName);

				fprintf(file, (const char*)buffer);
			}
		}
		SimData::FreeSimData();
		fclose(file);
	}
}


LRESULT CALLBACK FrameWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	if (Msg == WM_CREATE)
	{
		// Pointer to a value to be passed to the window through 
		// the CREATESTRUCT structure is out window object instance.
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		FrameWnd* pFrameWnd = static_cast<FrameWnd*>(pcs->lpCreateParams);
		// Store the pointer as private data of the window
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, PtrToUlong(pFrameWnd)); 
		pFrameWnd->Attach(hWnd);
		pFrameWnd->OnCreate();
		return 0;
	}

	// Extract the pointer to out window object
	FrameWnd* pFrameWnd = static_cast<FrameWnd*>(UlongToPtr(::GetWindowLongPtr(hWnd, GWLP_USERDATA)));

	if (pFrameWnd)
	{
		switch (Msg)
		{
		case WM_SIZE:
			pFrameWnd->OnSize(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_CLOSE:
			pFrameWnd->OnClose();
			return 0;
			
		// Parent window should process this message
		case WM_MOUSEWHEEL:
			pFrameWnd->OnMouseWheel(wParam);
			return 1;

		// Parent window should process this message
		case WM_KEYDOWN: 
			pFrameWnd->OnKeyDown(wParam);
			return 0; 


		case WM_DESTROY:
			pFrameWnd->OnDestroy();
			PostQuitMessage(0);
			return 0;

#ifndef _ALX_NO_TRAYICON

		case WM_NOTIFYICON:
			if (AfxGetApp()->m_pUserOptions->fUseTray)
			{
				switch (lParam)
				{
				case WM_LBUTTONDOWN:
					{
						if (pFrameWnd->GetState())
							pFrameWnd->SetState(FALSE);
						else
							pFrameWnd->SetState(TRUE);

						pFrameWnd->ChangeState();
					}
					break;


				case WM_RBUTTONDOWN:
					{
						ShowWindow(hWnd, SW_SHOW);
						SetForegroundWindow(hWnd);
					}
					break;
				}
			} // [option fUseTray]
			return 0;

#endif // _ALX_NO_TRAYICON

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
			case IDM_FILE_EXIT:
				pFrameWnd->OnFileExit();
				break;

			case ID_TRAY_COLLAPSE:
#ifndef _ALX_NO_TRAYICON
					pFrameWnd->ShowWindow(SW_HIDE);
#endif // _ALX_NO_TRAYICON
				break;

			case IDM_VIEW_OPTIONS:
				pFrameWnd->OnViewOptions();
				break;


			case IDM_HELP_INDEX:
				pFrameWnd->OnHelpIndex();
				break;
			case IDM_HELP_ABOUT:
				pFrameWnd->OnHelpAbout();
				break;
			case ID_CONTROL_PAD:
				pFrameWnd->OnControlPad();
				break;

			case ID_SCREENSHOOT:
				pFrameWnd->OnScreenshot();
				break;

			case ID_SIM_CONTINUE:
				AfxGetApp()->OnSimContinue();
				break;

			case ID_SIM_PAUSE:
				AfxGetApp()->OnSimPause();
				break;

			case ID_SIM_QUIT:
				{

					TCHAR szBuffer[MAX_PATH];
					if (MyLoadString(IDS_PROMPT_EXIT, szBuffer, MAX_PATH))
					{
						if (::MessageBox(hWnd, szBuffer, AfxGetAppName(),
								MB_YESNO | MB_ICONQUESTION) == IDYES)
								
						{
							pFrameWnd->OnFileExit();
						}
					}
				}
				break;


			case ID_CHART_LANDING:
				pFrameWnd->SetChartType(CHART_TYPE_LANDING);
				{
#ifdef TEST_WITHOUT_SIM
					// Debug dump all variable in the program into a file vars_dump.txt
					DumpVariables();
#endif // TEST_WITHOUT_SIM
				}
				break;

			case ID_CHART_ANGLES:
				pFrameWnd->SetChartType(CHART_TYPE_ANGLES);
				pFrameWnd->GetGDisplay()->InvalidateRect(NULL, FALSE);
				break;

			case ID_CHART_PITCH:
				pFrameWnd->SetChartType(CHART_TYPE_PITCH);
				pFrameWnd->GetGDisplay()->InvalidateRect(NULL, FALSE);
				break;

			case ID_CHART_ROLL:
				pFrameWnd->SetChartType(CHART_TYPE_ROLL);
				pFrameWnd->GetGDisplay()->InvalidateRect(NULL, FALSE);
				break;

			case ID_CHART_YAW:
				pFrameWnd->SetChartType(CHART_TYPE_YAW);
				pFrameWnd->GetGDisplay()->InvalidateRect(NULL, FALSE);
				break;

			case ID_CHART_THRUST:
				pFrameWnd->SetChartType(CHART_TYPE_THRUST);
				pFrameWnd->GetGDisplay()->InvalidateRect(NULL, FALSE);
				break;

			case ID_CHART_SIMPLE:
				pFrameWnd->SetChartType(CHART_TYPE_SIMPLE);
				pFrameWnd->GetGDisplay()->InvalidateRect(NULL, FALSE);
				break;

			case ID_SHOW_TOOLBAR:
				pFrameWnd->OnViewToolbar();
				break;

			case ID_SHOW_STATUSBAR:
				pFrameWnd->OnViewStatus();
				break;

			case ID_PLACE_TOP_MOST:
				pFrameWnd->OnPlaceTopMost();
				break;

				/********************************************************
			case ID_ADD_OUTPUTFOLDER:
				pFrameWnd->OnManageOutputFolder();
				break;
				*********************************************************/
			}
			return 0;


			// An overridden OnEraseBkgnd should return nonzero in response to 
			// WM_ERASEBKGND if it processes the message and erases the background; 
			// this indicates that no further erasing is required. If it returns 0, 
			// the window will remain marked as needing to be erased.
		case WM_ERASEBKGND:
			return 0;

		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->code)
			{
			case TCN_SELCHANGE:    
				pFrameWnd->OnTabChange();
				break;
			}
			pFrameWnd->OnNotify(wParam, lParam);
			return 0;

		case WM_HELP:
			pFrameWnd->OnHelp();
			return 0;
		}
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}


LRESULT CALLBACK NDisplayWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	if (Msg == WM_CREATE)
	{
		// Pointer to a value to be passed to the window through 
		// the CREATESTRUCT structure is out window object instance.
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		NDisplay* pNDisplay = static_cast<NDisplay*>(pcs->lpCreateParams);
		// Store the pointer as private data of the window
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, PtrToUlong(pNDisplay)); 
		pNDisplay->Attach(hWnd);
		pNDisplay->OnCreate();
		return 0;
	}

	// Extract the pointer to out window object
	NDisplay* pNDisplay = static_cast<NDisplay*>(UlongToPtr(::GetWindowLongPtr(hWnd, GWLP_USERDATA)));

	if (pNDisplay)
	{
		switch (Msg)
		{
		case WM_PAINT:
			::BeginPaint(hWnd, &ps);
			pNDisplay->OnPaint(ps.hdc);
			::EndPaint(hWnd, &ps);
			return 0;

		case WM_SIZE:
			pNDisplay->OnSize(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_CLOSE:
			pNDisplay->OnClose();
			return 0;

		case WM_DESTROY:
			pNDisplay->OnDestroy();
			return 0;

		case WM_HSCROLL:
			pNDisplay->OnHScroll(wParam);
			return 0;

		case WM_VSCROLL:
			pNDisplay->OnVScroll(wParam);
			return 0;

		case WM_RBUTTONUP:
			pNDisplay->OnRightButtonUp(LOWORD(lParam), HIWORD(lParam));
			return 0;

		}
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////

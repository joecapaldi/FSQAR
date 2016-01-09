
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
#include "HelpIDs.h"
#include "SimClient.h"
#include "resource.h"

// Important !!!
#define DATA_SEPARATORS_IDR		IDS_SEPARATOR_COMMA

// The file-scope variables
static SIMCLIENT_OPTIONS LocalOpt;
static HWND hSeparatorCombo;

// Use static as far as the function may be
// already  in defined in another module
static void DrawHyperlinkControl(LPCTSTR pszCaption, HDC hDC)
{
	HFONT hFont, hOldFont;
	//
	// Create font "Arial Cyr", size = 14,normal,underline
	//
	hFont = ::CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
		RUSSIAN_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, 34,
		TEXT("Arial Cyr"));
	::SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
	::SetTextColor(hDC, RGB(0, 0, 255));	// Blue
	hOldFont = (HFONT)::SelectObject(hDC, hFont);

	if (pszCaption != NULL)
	{
		size_t nSize;
		if (SUCCEEDED(StringCbLength(pszCaption, LONG_STR_LEN, &nSize)))
			::TextOut(hDC, 0, 0, pszCaption, (int)nSize/sizeof(TCHAR));
	}

	::SelectObject(hDC, hOldFont);
	::DeleteObject(hFont);
}

static LRESULT CALLBACK HelpLinkProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	static TCHAR szCaption[LONG_STR_LEN];

	switch (Msg)
	{
	case WM_CREATE:
		return TRUE;
	case WM_PAINT:
		BeginPaint(hWnd, & ps);
		if (MyLoadString(IDS_OPTIONS_HELP, szCaption, LONG_STR_LEN))
			DrawHyperlinkControl((LPTSTR)szCaption, ps.hdc);
		EndPaint(hWnd, & ps);
		return TRUE;
	case WM_LBUTTONUP:
		{
			if (HtmlHelp(hWnd, _T("FSQAR.chm::/Options.htm"), HH_DISPLAY_TOPIC, 0) == NULL)
				AfxGetApp()->Error(IDS_CANNOT_DISPLAY_HELP);
		}
		return TRUE;
	case WM_MOUSEMOVE:
		ShowWindow(hWnd, SW_SHOW);
		return TRUE;
	case WM_SETCURSOR:
#if _MSC_VER > 1400
		SetCursor(LoadCursor(NULL, IDC_HAND));
#else
		SetCursor(LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_CURSOR_HAND)));
#endif
		return TRUE;
	default:
		return (DefWindowProc(hWnd, Msg, wParam, lParam));
	}
}



static void OnCreate(HWND hDlg)
{
	Lock lock(&g_csAppData);
	AfxGetApp()->GetSimClient()->GetOptions(&LocalOpt);
	lock.Release();


	LocalOpt.fUseTimer = !LocalOpt.fUseSchedule;

	::CheckDlgButton(hDlg, IDC_USE_SCHEDULE, 
		LocalOpt.fUseSchedule ? TRUE : FALSE);

	::CheckDlgButton(hDlg, IDC_USE_TIMER, 
		LocalOpt.fUseTimer ? TRUE : FALSE);

	::CheckDlgButton(hDlg, IDC_USE_FILE, 
		LocalOpt.fPrintFile ? TRUE : FALSE);

	// Create hyperlink control
	SetWindowLongPtr(GetDlgItem(hDlg, IDC_OPTIONS_HELP), GWL_WNDPROC, 
		(LONG_PTR)HelpLinkProc);

	// Load datafile separator strings from resources and fill the combobox
	hSeparatorCombo = GetDlgItem(hDlg, IDC_SEPARATOR_COMBO);
	for (UINT i = 0; i < (IDS_SEPARATOR_UNKNOWN - IDS_SEPARATOR_COMMA); i++)
	{
		TCHAR szString[SHORT_STR_LEN];
		UINT id = DATA_SEPARATORS_IDR + i;
		if (MyLoadString(id, szString, SHORT_STR_LEN))
		{
			::SendMessage(hSeparatorCombo, CB_ADDSTRING, 0, (LPARAM)szString);
		}
	}

	// Set current datafile separator 
	if (LocalOpt.nSeparator < (IDS_SEPARATOR_UNKNOWN - IDS_SEPARATOR_COMMA))
		::SendMessage(hSeparatorCombo, CB_SETCURSEL, (WPARAM)(LocalOpt.nSeparator), 0);
	else
		::SendMessage(hSeparatorCombo, CB_SETCURSEL, 0, 0);

	// Disable the combox if we don't use file
	if (!IsDlgButtonChecked(hDlg, IDC_USE_FILE))
	{
		::EnableWindow(hSeparatorCombo, FALSE);
		// here other 'disable' ... 
	}
	
}


static void OnOK(HWND hDlg)
{
	LocalOpt.fUseSchedule = IsDlgButtonChecked(hDlg, IDC_USE_SCHEDULE);
	LocalOpt.fPrintFile = IsDlgButtonChecked(hDlg, IDC_USE_FILE);
	LocalOpt.fUseTimer = !LocalOpt.fUseSchedule;

	// Get current datafile separator from combobox
	INT selected = ::SendMessage(hSeparatorCombo, CB_GETCURSEL, 0, 0);
	switch (selected)
	{
	case 0:
		LocalOpt.nSeparator = DATAFILE_SEPARATOR_COMMA;
		break;

	case 1:
		LocalOpt.nSeparator = DATAFILE_SEPARATOR_SEMICOLON;
		break;

	case 2:
		LocalOpt.nSeparator = DATAFILE_SEPARATOR_SPACE;
		break;

	case 3:
		LocalOpt.nSeparator = DATAFILE_SEPARATOR_TAB;
		break;

	default:
		LocalOpt.nSeparator = DATAFILE_SEPARATOR_COMMA;
		break;
	}


	Lock lock(&g_csAppData);
	AfxGetApp()->GetSimClient()->SetOptions(&LocalOpt);
	lock.Release();

}



LRESULT CALLBACK SimOptionDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	switch (Msg)
	{
	case WM_INITDIALOG:
		OnCreate(hDlg);
		return TRUE;

	case WM_COMMAND:
		{
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
			{
				if (LOWORD(wParam) == IDC_USE_FILE)
				{
					if (::IsWindowEnabled(hSeparatorCombo))
						::EnableWindow(hSeparatorCombo, FALSE);
					else
						::EnableWindow(hSeparatorCombo, TRUE);
				}
			}
			return 0;
			}
		}
		return TRUE;

	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code)
			{
			case PSN_APPLY:
				OnOK(hDlg);
				break;

			default:
				return false;
			}
		}
		return TRUE;

	}

	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	return FALSE;
}


LRESULT CALLBACK AircraftOptionDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_INITDIALOG:
		{
			::CheckDlgButton(hDlg, IDC_SET_MANOEUVRE, 
				AfxGetApp()->GetGDisplay()->IsManoeuverable() ? TRUE : FALSE);
		}
		return TRUE;

	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code)
			{
			case PSN_APPLY:
				{
					BOOL flag = IsDlgButtonChecked(hDlg, IDC_SET_MANOEUVRE);
					AfxGetApp()->GetGDisplay()->SetManoeuverable(flag);
					AfxGetApp()->GetGDisplay()->UpdateChartType();
				}
				break;

			default:
				return false;
			}
		}
		return TRUE;

	}

	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	return FALSE;
}

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
#include "aboutgpl.h"
#include "webcont.h"
#include "resource.h"


static HWND s_hWndOwner;
static LPPRODUCT_INFORMATION s_ProductInfo;
static HBITMAP s_hBmpBanner;

LRESULT CALLBACK ViewLicenseDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
static void DrawBitmap(HDC hdc, HBITMAP hBitmap, int xStart, int yStart);
static void StretchBitmap(HDC hDC, LPRECT lpRect, HBITMAP hBitmap);

BOOL AboutFreewareDialog(HWND hWndOwner,
          LPPRODUCT_INFORMATION pProductInfo,
          HBITMAP hBmpBanner,
          DWORD dwDrawStyle)
{
	s_hWndOwner = hWndOwner;
	s_ProductInfo = pProductInfo;
	s_hBmpBanner = hBmpBanner;
	dwDrawStyle;	// Unused
	DialogBox(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD_ABOUT_GPL), s_hWndOwner, (DLGPROC)AboutDlgProc);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// lstrlen_s
//
// The safe version of lstrlen function. Get the length of a string.
// Parameters Null-terminated string.
// Return Values
// Each of these functions returns the number of characters in string, excluding
// the terminal NULL. No return value is reserved to indicate an error.
//
size_t lstrlen_sA(LPCSTR lpString)
{
#if _MSC_VER < 1400
	return lstrlenA(lpString);
#else
	size_t cb;
	
	if (SUCCEEDED(StringCbLengthA(lpString, STRSAFE_MAX_CCH*sizeof(TCHAR), &cb)))
	{
		return (cb / sizeof(TCHAR));
	}
	
	return 0;
#endif
}

size_t lstrlen_sW(LPCWSTR lpString)
{
#if _MSC_VER < 1400
	return lstrlenW(lpString);
#else
	size_t cb;
	
	if (SUCCEEDED(StringCbLengthW(lpString, STRSAFE_MAX_CCH*sizeof(TCHAR), &cb)))
	{
		return (cb / sizeof(TCHAR));
	}
	
	return 0;
#endif
}



// Create a tooltip control over the entire window area
void CreateTooltip(HWND hWndOwner, LPCTSTR pszTipText)
{
	HWND hWndTooltip;
	TOOLINFO ti;
	unsigned int uid = 0;
	LPTSTR lptstr = (LPTSTR)pszTipText;
	RECT rect;
	hWndTooltip = CreateWindowEx(WS_EX_TOPMOST,
	          TOOLTIPS_CLASS,
	          NULL,
	          WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
	          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	          hWndOwner,
	          NULL,
	          AfxGetInstanceHandle(),
	          NULL);
	SetWindowPos(hWndTooltip,
	          HWND_TOPMOST,
	          0, 0, 0, 0,
	          SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	// Get coordinates of the main client area
	GetClientRect(hWndOwner, &rect);
	// Initialize members of the toolinfo structure
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hWndOwner;
	ti.hinst = AfxGetInstanceHandle();
	ti.uId = uid;
	ti.lpszText = lptstr;
	// ToolTip control will cover the whole window
	ti.rect.left = rect.left;
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;
	// Send an addtool message to the tooltip control window
	SendMessage(hWndTooltip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO) &ti);
}

// Use static as far as the function may be
// already  in defined in another module
static void DrawHyperlinkControl(LPCTSTR pszCaption, HDC hDC)
{
	HFONT hFont, hOldFont;
	//
	// Create font "Arial Cyr", size = 14,normal,underline
	//
	hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
	          RUSSIAN_CHARSET,
	          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
	          DEFAULT_QUALITY, 34,
	          TEXT("Arial Cyr"));
	SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
	SetTextColor(hDC, RGB(0, 0, 255));	// Blue
	hOldFont = (HFONT)SelectObject(hDC, hFont);
	
	if (pszCaption != NULL)
	{
		TextOut(hDC, 0, 0, pszCaption, lstrlen(pszCaption));
	}
	
	SelectObject(hDC, hOldFont);
	DeleteObject(hFont);
}

static LRESULT CALLBACK HLinkProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	static TCHAR szCaption[LONG_STR_LEN];
	
	switch (Msg)
	{
	case WM_CREATE:
		return TRUE;
	case WM_PAINT:
		BeginPaint(hWnd, & ps);
		LoadString(AfxGetInstanceHandle(), IDS_VIEW_LICENSE, szCaption, LONG_STR_LEN);
		DrawHyperlinkControl((LPTSTR)szCaption, ps.hdc);
		EndPaint(hWnd, & ps);
		return TRUE;
	case WM_LBUTTONUP:
	{
		DialogBox(AfxGetInstanceHandle(),
		          MAKEINTRESOURCE(IDD_VIEW_LICENSE),
		          s_hWndOwner,
		          (DLGPROC)ViewLicenseDlgProc);
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



void DrawBanner(HDC hDC, LPRECT lprc)
{
	HFONT hLedFont, hSmallFont, hOldFont;
	int nHeight, PointSize;
	HBITMAP hBmp;
	HBRUSH hBrush = CreateSolidBrush(COLOR_BANNER);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	// Draw white rectangle
	FillRect(hDC, lprc, hBrush);
	SelectObject(hDC, hOldBrush);
	DeleteObject(hBrush);
	hBmp = s_hBmpBanner;
	DrawBitmap(hDC, hBmp, 0, 0);
	StretchBitmap(hDC, lprc, hBmp);
	// Set mode
	SetMapMode(hDC, MM_TEXT);
	SetBkMode(hDC, TRANSPARENT);
	// Draw icon
	DrawIcon(hDC, ICON_X, ICON_Y, s_ProductInfo->hIcon);
	// Store system GUI font
	hOldFont = (HFONT)SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));
	// Create font "Tahoma",bold,size = 22
	PointSize = 22;
	nHeight = -MulDiv(PointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	hLedFont = CreateFont(nHeight, 0, 0, 0, FW_NORMAL,
	          FALSE, FALSE, FALSE,
	          RUSSIAN_CHARSET,
	          OUT_DEFAULT_PRECIS,
	          CLIP_DEFAULT_PRECIS,
	          DEFAULT_QUALITY, 34, _T("Tahoma"));
	SetBkColor(hDC, RGB(255, 255, 255));
	SetTextColor(hDC, COLOR_APPNAME);
	SelectObject(hDC, (HGDIOBJ)hLedFont);
	TextOut(hDC, APPNAME_X, ICON_Y, s_ProductInfo->szProductName, lstrlen(s_ProductInfo->szProductName));
	// Create font "Arial",size = 8, bold
	PointSize = 8;
	nHeight = -MulDiv(PointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	hSmallFont = CreateFont(nHeight, 0, 0, 0, FW_BOLD,
	          FALSE, FALSE, FALSE,
	          RUSSIAN_CHARSET,
	          OUT_DEFAULT_PRECIS,
	          CLIP_DEFAULT_PRECIS,
	          DEFAULT_QUALITY, 34, _T("Arial"));
	// Black
	SetTextColor(hDC, RGB(0, 0, 0));
	SelectObject(hDC, (HGDIOBJ)hSmallFont);
	TextOut(hDC, APPNAME_X, BANNER_HEIGHT - 24, s_ProductInfo->szProductDescription, lstrlen(s_ProductInfo->szProductDescription));
	// Restore system GUI font
	SelectObject(hDC, (HGDIOBJ)hOldFont);

	// Free GDI resource (!!!)
	DeleteObject((HGDIOBJ)hLedFont);
	DeleteObject((HGDIOBJ)hSmallFont);
}


#pragma warning(push) 
#pragma warning(disable: 4244) // conversion from 'LONG_PTR' to 'LONG', possible loss of data
LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HFONT hLedFont = NULL, hSmallFont = NULL;
	RECT rcClient;
	TCHAR szCaption[LONG_STR_LEN];
	TCHAR szAppName[SHORT_STR_LEN];
	TCHAR szVersion[LONG_STR_LEN];
	TCHAR szCopyright[LONG_STR_LEN];
	TCHAR szGPLTip[LONG_STR_LEN];
	
	switch (Msg)
	{
	case WM_INITDIALOG:
		// Set banner position
		GetClientRect(hDlg, &rcClient);
		SetWindowPos(GetDlgItem(hDlg, IDC_BANNER), hDlg, 0, 0,
		          rcClient.right - rcClient.left,
		          BANNER_HEIGHT,
		          SWP_SHOWWINDOW | SWP_NOZORDER);
		StringCbPrintf(szCaption, LONG_STR_LEN, _T("About %s"), s_ProductInfo->szProductName);
		SetWindowText(hDlg, (LPTSTR)szCaption);
		// Init strings
		StringCbCopy(szAppName, SHORT_STR_LEN, s_ProductInfo->szProductName);
		StringCbCopy(szVersion, LONG_STR_LEN, _T("\0"));
		StringCbCopy(szCopyright, LONG_STR_LEN, s_ProductInfo->szAuthors);
		// Will be read from resource
		StringCbCopy(szGPLTip, LONG_STR_LEN, _T("\0"));
		// Create hyperlink control
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_VIEW_LICENSE), GWL_WNDPROC, (LONG_PTR)HLinkProc);
		LoadString(AfxGetInstanceHandle(), IDS_GPL_TIP, szGPLTip, LONG_STR_LEN);
		CreateTooltip(GetDlgItem(hDlg, IDC_VIEW_LICENSE), (LPCTSTR)&szGPLTip);
		StringCbPrintf(szVersion, LONG_STR_LEN,  _T("%u.%u.%u"),
		          s_ProductInfo->nMajorVersion,
		          s_ProductInfo->nMinorVersion,
		          s_ProductInfo->nRevision);
		// Send text to static controls
		SetDlgItemText(hDlg, IDC_APPVERSION, (LPTSTR)szVersion);
		SetDlgItemText(hDlg, IDC_APPCOPYRIGHT, (LPTSTR)szCopyright);
		return TRUE;
	case WM_DRAWITEM:
	{
		UINT nID = (UINT)wParam;
		LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lParam;
		
		if (nID == IDC_BANNER)
		{
			if (lpDrawItem->itemAction == ODA_DRAWENTIRE)
			{
				DrawBanner(lpDrawItem->hDC, &lpDrawItem->rcItem);
			}
		}
	}
	return TRUE;
	case WM_COMMAND:
	
		switch (wParam)
		{
		case IDOK:
		
			if (hLedFont != NULL) DeleteObject((HGDIOBJ)hLedFont);
			
			if (hSmallFont != NULL) DeleteObject((HGDIOBJ)hSmallFont);
			
			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
		
			if (hLedFont != NULL) DeleteObject((HGDIOBJ)hLedFont);
			
			if (hSmallFont != NULL) DeleteObject((HGDIOBJ)hSmallFont);
			
			EndDialog(hDlg, TRUE);
			break;
		}
		
		return TRUE;
	}
	
	return FALSE;
}

#pragma warning(pop) 

static void DrawBitmap(HDC hdc, HBITMAP hBitmap, int xStart, int yStart)
{
	BITMAP  bm;
	HDC hdcMem;
	POINT ptSize, ptOrg;
	// Create memory device context and put bitmap into it
	hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem, hBitmap);
	SetMapMode(hdcMem, GetMapMode(hdc));
	// Find physical dimensions of pixels so BitBlt won't scale bitmap
	GetObject(hBitmap, sizeof(BITMAP), (LPVOID)(&bm));
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;
	DPtoLP(hdc, &ptSize, 1);
	ptOrg.x = 0;
	ptOrg.y = 0;
	DPtoLP(hdcMem, &ptOrg, 1);
	// Copy over the bitmap
	BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y,
	          hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);
	
	DeleteDC(hdcMem);
}


void StretchBitmap(HDC hDC, LPRECT lpRect, HBITMAP hBitmap)
{
	HDC memDC;
	BITMAP bm;
	HGDIOBJ hOldObj;
	memDC = CreateCompatibleDC(hDC);
	hOldObj = SelectObject(memDC, hBitmap);
	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
	StretchBlt(hDC, 0, 0, lpRect->right,
	          lpRect->bottom,
	          memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	SelectObject(memDC, hOldObj);
	DeleteDC(memDC);
	
	if (hBitmap)
		DeleteObject(hBitmap);
}


LRESULT CALLBACK ViewLicenseDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HRSRC hRsrc;
	HGLOBAL hResData = NULL;
	LPVOID pHTML = NULL;
	
	switch (Msg)
	{
	case WM_INITDIALOG:
		// Find HTML page in resource
		hRsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_GPL_TEXT), RT_HTML);
		if (hRsrc)
		{
			hResData = LoadResource(AfxGetInstanceHandle(), hRsrc);
			if (hResData)
			{
				pHTML = LockResource(hResData);
				if (pHTML)
					// The pointer pHTML must be converted into const char*
					DisplayHTMLStr(GetDlgItem(hDlg, IDC_WEB_CONTROL), (LPCSTR)pHTML);
			}

		}

		return TRUE;
	case WM_COMMAND:
	
		switch (wParam)
		{
		case IDOK:
			if (hResData)
			{
				FreeResource(hResData);
			}
			EndDialog(hDlg, TRUE);
			break;
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
	
	UNREFERENCED_PARAMETER(lParam);
	return FALSE;
}

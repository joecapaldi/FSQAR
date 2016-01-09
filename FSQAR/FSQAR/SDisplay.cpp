
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
#include "SimClient.h"
#include "SimVarIDS.h"
#include "registry.h"
#include "resource.h"

// Debug version of CRT library uses this operator
#ifdef _DEBUG
#ifndef DEBUG_NEW
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new DEBUG_NEW
#endif
#endif  // _DEBUG

//----------------------------------------------------------------- 
// SDisplay implementation
//----------------------------------------------------------------- 

LRESULT CALLBACK ChartOptionsDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK SDisplayWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	if (Msg == WM_CREATE)
	{
		// Pointer to a value to be passed to the window through 
		// the CREATESTRUCT structure is out window object instance.
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		SDisplay* pSDisplay = static_cast<SDisplay*>(pcs->lpCreateParams);
		// Store the pointer as private data of the window
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, PtrToUlong(pSDisplay)); 
		pSDisplay->OnCreate();
		return 0;
	}

	// Extract the pointer to out window object
	SDisplay* pSDisplay = static_cast<SDisplay*>(UlongToPtr(::GetWindowLongPtr(hWnd, GWLP_USERDATA)));

	if (pSDisplay)
	{
		switch (Msg)
		{
		case WM_SIZE:
			pSDisplay->OnSize(wParam, lParam);
			return 0;

		case WM_ERASEBKGND:
			return 0;

		case WM_DISPLAYCHANGE:
		case WM_PAINT:
			BeginPaint(hWnd, &ps);
			pSDisplay->OnPaint(ps.hdc);
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			pSDisplay->OnDestroy();
			return 0;

		case WM_RBUTTONUP:
			pSDisplay->OnRightButtonUp(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}



SDisplay::SDisplay()
{ 
	m_clrBackground = DISPLAY_BACKGROUND_COLOR;
	m_clrText = DISPLAY_TEXT_COLOR;
	
	m_hFont = NULL;
	
	m_nWidth = 200;
	m_nHeight = 200;
	
	m_yStep = 0;
	m_xStep = 0;
	
	m_nLinesCount = 20;
	
} 

SDisplay::~SDisplay()
{ 
	TRACE0("DTOR SDisplay\n");

	DeleteObject(m_hFont);
} 

HFONT SDisplay::CreateDefaultFont()
{
	LOGFONT lf;

	// Height = '11' for 'Segoe UI' font , 0xFFFFFFF2 in registry
	lf.lfHeight = -14;			
	lf.lfWeight = FW_SEMIBOLD;	
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfPitchAndFamily = FF_ROMAN;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = DEFAULT_CHARSET;

	// Find appropriate font
	if (SearchFont(_T("Segoe UI")))
		::StringCbCopy(lf.lfFaceName, LF_FACESIZE, _T("Segoe UI"));
	else
		::StringCbCopy(lf.lfFaceName, LF_FACESIZE, _T("Arial"));

	return ::CreateFontIndirect(&lf);
}

BOOL SDisplay::Create(const Window* pFrameWnd, LPCRECT lpRect)
{
	return Window::Create(0, szSDisplayWndClass, NULL, 
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		lpRect, pFrameWnd, 0, this);
}

BOOL SDisplay::OnCreate()
{
	CRegistry Reg(HKEY_CURRENT_USER);
	int buffer[LONG_STR_LEN];

	Reg.OpenKey(APP_MAIN_KEY);
	if (Reg.ReadBinary(_T("Favorites"), (LPBYTE)&buffer, LONG_STR_LEN))
	{
		int count = buffer[0];
		for (int i = 1; i <= count; i++)
		{
			m_listFavorites.insert(buffer[i]);
		}
	}

	Reg.CloseKey();	

	m_hFont = CreateDefaultFont();

	return TRUE;
}

void SDisplay::OnSize(UINT_PTR nWidth, UINT_PTR nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight; 
}

void SDisplay::OnPaint(HDC hDC)
{

	RECT rcClient;
	GetClientRect(&rcClient);
	int nWidth = rcClient.right - rcClient.left;
	int nHeight = rcClient.bottom - rcClient.top;
	RECT rcWork;
	CopyRect(&rcWork, &rcClient);


	// Standard Setup

	HDC hMemDC = ::CreateCompatibleDC(hDC);
	if (hMemDC)
	{
		int dx = -8;
		int dy = 0;	
		HBITMAP hBmp   = ::CreateCompatibleBitmap(hDC, nWidth, nHeight);
		HGDIOBJ hOldBmp = ::SelectObject(hMemDC, hBmp);

		// Outline 3D shadow
		Paint3DShadow(hMemDC, &rcClient);
		InflateRect(&rcWork, -1, -1);	// '2' is shadow pen size
		// Draw background
		MyFillRect(hMemDC, &rcWork, m_clrBackground);

		// Shift text to the right and to the down
		InflateRect(&rcWork, dx, dy);

		// Create font from current LOGFONT
		HFONT hOldFont = (HFONT)::SelectObject(hMemDC, m_hFont);

		// Calculate xStep and yStep which are based on the font metrics
		TEXTMETRIC tm;
		GetTextMetrics(hMemDC, &tm);
		m_yStep = tm.tmHeight + tm.tmExternalLeading;
		m_xStep = tm.tmAveCharWidth;
		if (m_yStep == 0)
			m_yStep  = 8;

		// How many lines the working rectangle can contain ?
		m_nLinesCount = (rcWork.bottom - rcWork.top)/m_yStep;

		DoPaint(hMemDC, &rcWork);

		::SelectObject(hMemDC, hOldFont);
		//::DeleteObject((HGDIOBJ)hFont);

		// Copies the source rectangle directly to the destination rectangle
		::BitBlt(hDC, 0, 0, nWidth, nHeight, hMemDC, 0, 0, SRCCOPY);

		// Standard End
		::SelectObject(hMemDC, hOldBmp);
		::DeleteDC(hMemDC);
		::DeleteObject(hBmp); 

	}
	else
	{
		TRACE2("Memory DC failed: %#X, %d\n", hDC, GetLastError());
	}

}

void SDisplay::PrintDouble(HDC hDC, int xPos, int yPos, double Value)
{
	TCHAR szText[LONG_STR_LEN];
	size_t nSize;
	StringCbPrintf(szText, LONG_STR_LEN,
		_T("%5.3f"), Value);

	if (SUCCEEDED(StringCbLength(szText, LONG_STR_LEN, &nSize)))
		MyTextOut(hDC, xPos, yPos, szText, nSize/sizeof(TCHAR), m_clrBackground, m_clrText);
}

void SDisplay::DoPaint(HDC hDC, LPRECT lpRect)
{
	// It may be removed ??????
	UNREFERENCED_PARAMETER(lpRect);

	TCHAR szText[LONG_STR_LEN];
	TCHAR szDesc[LONG_STR_LEN];
	size_t len = 0;
	UINT id = SIMVAR_BASE_ID;

	static SimData* pVariables = SimData::GetSimData();

	for (int i = 0; i < m_listFavorites.getCount(); i++)
	{
		INT_PTR nFavorite;
		int dx = 64;
		int dy = 64;

		int y;	
		y = dy + m_yStep*i;
	
		nFavorite = m_listFavorites.getAt(i);
		if (MyLoadString(id + nFavorite, szDesc, LONG_STR_LEN))
		{
			TCHAR szUnitsName[SHORT_STR_LEN];
#ifndef _UNICODE
			StringCbCopy(szUnitsName, SHORT_STR_LEN, pVariables->GetVariable(nFavorite)->UnitsName);
#else
			_mbstowcsz(szUnitsName, pVariables->GetVariable(nFavorite)->UnitsName, SHORT_STR_LEN);
#endif

			Lock lock(&g_csSimData);
			double val = pVariables->GetVariable(nFavorite)->Value;
			lock.Release();

			StringCbPrintf(szText, LONG_STR_LEN, 
				_T("%s: %8.5f (%s)"),
				szDesc,
				val,
				szUnitsName);

			if (SUCCEEDED(StringCbLength(szText, LONG_STR_LEN, &len)))
				MyTextOut(hDC, dx, y, szText, len/sizeof(TCHAR), m_clrBackground, m_clrText);
		}


	}

	SimData::FreeSimData();

	if (m_listFavorites.getCount() == 0)
	{
		if (MyLoadString(IDS_FAVORITE_RIGHT_CLICK, szText, LONG_STR_LEN))
		{
			if (SUCCEEDED(StringCbLength(szText, LONG_STR_LEN, &len)))
			{
				MyTextOut(hDC, 50, 50,
					szText, len / sizeof(TCHAR), m_clrBackground, RGB(195, 195, 195));
			}

		}
	}
}

void SDisplay::AddToListBox(HWND hListBox)
{
	TCHAR szString[LONG_STR_LEN];
	UINT id = SIMVAR_BASE_ID;
	for (int i = 0; i < m_listFavorites.getCount(); i++)
	{
		int nFavorite = m_listFavorites.getAt(i);
		if (MyLoadString(id + nFavorite, 
			szString, LONG_STR_LEN))
		{
			::SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)szString);
			::SendMessage(hListBox, LB_SETITEMDATA, i, nFavorite);
		}
	}
}

void SDisplay::OnDestroy()	
{
	CRegistry Reg(HKEY_CURRENT_USER);
	int buffer[LONG_STR_LEN];

	int count = m_listFavorites.getCount();
	if (count > LONG_STR_LEN)
		count = LONG_STR_LEN;

	buffer[0] = count;
	for (int i = 1; i <= count; i++)
	{
		buffer[i] = m_listFavorites.getAt(i-1);
	}

	Reg.OpenKey(APP_MAIN_KEY);
	Reg.WriteBinary(_T("Favorites"),(CONST BYTE*)buffer, (SIZE_T)(count+1)*sizeof(int));
	Reg.CloseKey();	

}

VOID SDisplay::OnRightButtonUp(int xPos, int yPos)
{
	POINT ptMouse;
	ptMouse.x = xPos;
	ptMouse.y = yPos;

	// If the function succeeds, the return value is the nResult parameter 
	// in the call to the EndDialog function used to terminate the dialog 
	// box.
	INT_PTR nRes = DialogBox(AfxGetInstanceHandle(), 
		MAKEINTRESOURCE(IDD_CHART_PROPERTIES),
		m_hWnd, (DLGPROC)ChartOptionsDlgProc);
	if (nRes == IDOK)
	{
		InvalidateRect(NULL, FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////

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
#include "simclient.h"
#include "registry.h"

#include "SimVarIDS.h"
#include "resource.h"


// Debug version of CRT library uses this operator
#ifdef _DEBUG
#ifndef DEBUG_NEW
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new DEBUG_NEW
#endif
#endif  // _DEBUG
NDisplay::NDisplay()
{
	m_clrBackground = DISPLAY_BACKGROUND_COLOR;
	m_clrText = DISPLAY_TEXT_COLOR;

	m_hFont = CreateDefaultFont(); // don't remove it

	m_yNewPos = 0;
	m_xNewPos = 0;

	m_nLinesCount = 12;
	m_nMaxLength = SHORT_STR_LEN;
	
	m_nWidth = 200;
	m_nHeight = 200;
	
	m_yStep = 0;
	m_xStep = 0;	

}
NDisplay::~NDisplay()
{
	TRACE0("DTOR NDisplay\n");

	DeleteObject(m_hFont);
}

HFONT NDisplay::CreateDefaultFont()
{
	// Height = '11' for 'Segoe UI' font , 0xFFFFFFF2 in registry
	m_lf.lfHeight = -14;			
	m_lf.lfWeight = FW_SEMIBOLD;	
	m_lf.lfEscapement = 0;
	m_lf.lfOrientation = 0;
	m_lf.lfPitchAndFamily = FF_ROMAN;
	m_lf.lfItalic = 0;
	m_lf.lfUnderline = 0;
	m_lf.lfStrikeOut = 0;
	m_lf.lfCharSet = DEFAULT_CHARSET;

	// Find appropriate font
	if (SearchFont(_T("Segoe UI")))
		::StringCbCopy(m_lf.lfFaceName, LF_FACESIZE, _T("Segoe UI"));
	else
		::StringCbCopy(m_lf.lfFaceName, LF_FACESIZE, _T("Arial"));

	return ::CreateFontIndirect(&m_lf);
}



BOOL NDisplay::OnCreate()
{

	//-------------------------------------------------------------
	// m_yStep average height of characters in the current font
	// m_xStep average width of characters in the current font
	// 
	TEXTMETRIC tm;
	HDC hdc = GetDC(m_hWnd);
	GetTextMetrics(hdc, &tm);
	m_yStep = tm.tmHeight + tm.tmExternalLeading;
	m_xStep = tm.tmAveCharWidth;
	ReleaseDC(m_hWnd, hdc);
	//-------------------------------------------------------------

	if (LoadFontFromRegistry(&m_lf))
		m_hFont = ::CreateFontIndirect(&m_lf);
	else
		m_hFont = CreateDefaultFont();

	return TRUE;
}


void NDisplay::OnClose()	
{
	
}

void NDisplay::OnDestroy()	
{
	// Called when FrameWnd says 'Destroy'
	// see CFrameWnd::OnClose()
	if (!SaveFontToRegistry(&m_lf))
	{
		TRACE0("NDisplay::OnDestroy(): Failed to save font in registry\n");
	}
}


/// ----------------------------------------------------
// Fill a rectangular area with given color.
// It creates and delete GDI resources in one place.
// ----------------------------------------------------
void MyFillRect(HDC hDC, LPRECT lpRect,  COLORREF Color)
{
	ASSERT(lpRect);
	__analysis_assume(lpRect);

	HBRUSH hBrush = ::CreateSolidBrush(Color);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDC, hBrush);
	::FillRect(hDC, lpRect, hBrush);
	// Free GDI resources (!!!)
	::SelectObject(hDC, hOldBrush);
	::DeleteObject((HGDIOBJ)hBrush);

}


// -------------------------------------------------------------------
// My extension of GDI TextOut function to make it call more obvious.
// GDI resource for this function must be allocated and deleted 
// in another function (in caller).
// -------------------------------------------------------------------
void MyTextOut(HDC hDC, int xPos, int yPos,  LPCTSTR pText,  size_t nSize,  COLORREF BkColor,  COLORREF TextColor)
{
	::SetBkColor(hDC, BkColor);
	::SetTextColor(hDC, TextColor);
	::SetTextAlign(hDC, TA_TOP | TA_LEFT);
	::TextOut(hDC, xPos, yPos, pText, (int)nSize);
}



// ---------------------------------------------------
// Paint 3D shadow around given rectangle
// It creates and delete GDI resources in one place.
// ---------------------------------------------------
void Paint3DShadow(HDC hDC, LPRECT lpRect)
{
	ASSERT(lpRect);
	__analysis_assume(lpRect);

	// create GDI objects
	HPEN hPen = CreatePen(PS_SOLID, 2, GetSysColor(COLOR_3DSHADOW));
	HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);
	// upper line
	MoveToEx(hDC, lpRect->left, lpRect->top, NULL);
	LineTo(hDC, lpRect->right, lpRect->top);

	// left line
	MoveToEx(hDC, lpRect->left, lpRect->top, NULL);
	LineTo(hDC, lpRect->left, lpRect->bottom); 
	// free GDI objects
	SelectObject(hDC, hOldPen);
	DeleteObject((HGDIOBJ)hPen);

	hPen = CreatePen(PS_SOLID, 2, GetSysColor(COLOR_3DLIGHT));
	hOldPen = (HPEN)SelectObject(hDC, hPen);
	// right line
	MoveToEx(hDC, lpRect->right, lpRect->top, NULL);
	LineTo(hDC, lpRect->right, lpRect->bottom);
	// bottom line
	MoveToEx(hDC, lpRect->left, lpRect->bottom, NULL);
	LineTo(hDC, lpRect->right, lpRect->bottom);

	// free GDI objects
	SelectObject(hDC, hOldPen);
	DeleteObject((HGDIOBJ)hPen);
}

void NDisplay::OnPaint(HDC hDC)
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
		static SimData* pVars = SimData::GetSimData();
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

		// Check it again.
		// 'God helps those who help themselves'
		if (m_yNewPos < 0)
			m_yNewPos = 0;

		if (m_yNewPos + m_nLinesCount >= SIM_VARS_COUNT)
			m_yNewPos = SIM_VARS_COUNT - m_nLinesCount;

		
		Lock lock(&g_csSimData);

		// Current page shows a block of entries depending on vertical 
		// scroll position. Bound of the array already had been checked.
		for (intptr_t nEntry = 1 + m_yNewPos; 
			nEntry < (m_yNewPos + m_nLinesCount); 
			nEntry++)
		{

			TCHAR szText[LONG_STR_LEN];
			TCHAR szDesc[LONG_STR_LEN];
			size_t cbCount = 0;
			UINT id = SIMVAR_BASE_ID;
			int x, y;

			if (MyLoadString(id + nEntry, szDesc, LONG_STR_LEN))
			{

				TCHAR szUnitsName[SHORT_STR_LEN];
				const SIMVARIABLE* pSimVar = pVars->GetVariable(nEntry);

				// Convert 'UnitsName' into Unicode string if it is needed
#ifndef _UNICODE
				StringCbCopy(szUnitsName, SHORT_STR_LEN, pVars->GetVariable(nEntry)->UnitsName);
#else
				_mbstowcsz(szUnitsName, pSimVar->UnitsName, SHORT_STR_LEN);
#endif

				if ((nEntry == LAT) || (nEntry == LON))
				{
					// Special format for longitude and latitude
					StringCbPrintf(szText, LONG_STR_LEN,
						_T("%s: %8.5f (%s)"),
						szDesc,
						pSimVar->Value,
						szUnitsName);
				}
				else
				{
					StringCbPrintf(szText, LONG_STR_LEN,
						_T("%s: %5.2f (%s)"),
						szDesc,
						pSimVar->Value,
						szUnitsName);
				}


				//
				// cbCount is the current length in bytes of szText excluding 
				// the null terminator. This out parameter is equivalent to 
				// the return value of strlen(szText) * sizeof(TCHAR)
				//
				if (SUCCEEDED(StringCbLength((LPCTSTR)szText, LONG_STR_LEN, &cbCount)))
				{
					// Check if the string is the longest in this text
					if (cbCount > m_nMaxLength)
						m_nMaxLength = cbCount;

					if (m_nMaxLength > LONG_STR_LEN)
						m_nMaxLength = LONG_STR_LEN;

					// Set pointer to the begin of the string
					LPTSTR p = szText;
					// When the pointer move to the right, the displayed length
					// of the string is decreasing
					int cchCount = (int)cbCount / sizeof(TCHAR);
					for (int s = 0; s < min(m_xNewPos, cchCount); s++)
					{
						p++;
						cchCount--;
					}

					ASSERT(AfxIsValidAddress(p, cchCount));

					// Position of the string output
					x = rcWork.left;	// always the left border of the window
					y = rcWork.top + m_yStep*(nEntry - m_yNewPos);	// one line

					// Check bottom margin
					if (y >= (rcWork.bottom - m_yStep))
						y = rcWork.bottom - m_yStep;

					// Don't display strings that are not in the visible window frame
					// cchCount is how many characters are in the frame after scrolling
					//if (cchCount > m_xNewPos)
					{
						MyTextOut(hMemDC, x, y, p, cchCount, m_clrBackground, m_clrText);
					}
				} // if (SUCCEEDED(StringCbLengt)
			} // if (MyLoadString)
		} // Enumerate all entries in data array (for)
	

		lock.Release();

		::SelectObject(hMemDC, hOldFont);

		// Copies the source rectangle directly to the destination rectangle
		::BitBlt(hDC, 0, 0, nWidth, nHeight, hMemDC, 0, 0, SRCCOPY);


		SimData::FreeSimData();
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


// ----------------------------------------------------
// Display 'Font Select' dialog.
// The dialog has been initialized by current LOGFONT.
// ----------------------------------------------------
BOOL NDisplay::FontSelectDialog()
{
	CHOOSEFONT cf;

	LOGFONT lf;
	GetObject(m_hFont, sizeof(lf), &lf);

	// Initialize CHOOSEFONT
	ZeroMemory(&cf, sizeof(cf));
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = m_hWnd;
	cf.Flags =CF_FORCEFONTEXIST | CF_BOTH | CF_INITTOLOGFONTSTRUCT;
	cf.hInstance = AfxGetInstanceHandle();
	cf.lpLogFont = &lf;

	// If the user clicks the OK button of the dialog box, 
	// the return value is nonzero
	if (ChooseFont(&cf) != 0)
	{
		lf = *cf.lpLogFont;
		return TRUE;
	}
	m_hFont = CreateFontIndirect(&lf);
	return FALSE;
}


BOOL NDisplay::Create(const Window* pFrameWnd, LPCRECT lpRect)
{
	return Window::Create(0, _T("QARDisplayWndClass"), NULL, 
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | 
		WS_CLIPSIBLINGS| WS_HSCROLL | WS_VSCROLL,
		lpRect, pFrameWnd, 0, this);
}

void NDisplay::OnSize(UINT nWidth, UINT nHeight)
{
	SCROLLINFO si; 

	m_nWidth = nWidth;
	m_nHeight = nHeight; 

	ASSERT(m_yStep != 0);

	si.cbSize = sizeof(si); 
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
	si.nMin   = 0; 
	si.nMax   = SIM_VARS_COUNT-m_nLinesCount; 
	si.nPage  = m_nLinesCount/m_yStep;
	si.nPos   = m_yNewPos; 
	SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE); 

	si.cbSize = sizeof(si); 
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
	si.nMin   = 0; 
	si.nMax   = (int)(m_nMaxLength*m_xStep- m_nWidth); 
	si.nPage  = m_nWidth/(m_nMaxLength*m_xStep);
	si.nPos   = m_xNewPos; 
	SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE);

}


void NDisplay::OnHScroll(WPARAM wParam)
{
	SCROLLINFO si; 

	switch (LOWORD(wParam)) 
	{ 
	case SB_PAGEUP: 
		m_xNewPos -= 50; 
		// Check after decrement
		if (m_xNewPos < 0)
			m_xNewPos = 0;
		break; 

	case SB_PAGEDOWN: 
		// Check before increment
		if (m_xNewPos < abs((int)(m_nMaxLength*m_xStep - m_nWidth)))
			m_xNewPos += 50; 
		break; 

	case SB_LINEUP: 
		m_xNewPos--; //-= m_xStep; 
		// Check after decrement
		if (m_xNewPos < 0)
			m_xNewPos = 0;
		break; 

	case SB_LINEDOWN: 
		if (m_xNewPos < abs((int)(m_nMaxLength*m_xStep - m_nWidth)))
			m_xNewPos++;// += m_xStep; 
		break; 

	case SB_THUMBPOSITION: 
		m_xNewPos = HIWORD(wParam); 
		break; 

	default: 
		break;
	} 

	si.cbSize = sizeof(si); 
	si.fMask  = SIF_POS; 
	si.nPos   = m_xNewPos; 
	SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE); 

	// Do nothing without this ...
	InvalidateRect(NULL, FALSE);
}

void NDisplay::OnVScroll(WPARAM wParam)
{
	SCROLLINFO si; 

	switch (LOWORD(wParam)) 
	{ 
	case SB_PAGEUP: 
		m_yNewPos -= m_nLinesCount;
		// Check after decrement
		if (m_yNewPos < 0)
			m_yNewPos = 0;
		break; 

	case SB_PAGEDOWN: 
		// Check before increment
		if (m_yNewPos < SIM_VARS_COUNT - m_nLinesCount)
			m_yNewPos += m_nLinesCount; 
		break; 

	case SB_LINEUP: 
		--m_yNewPos;
		// Check after decrement
		if (m_yNewPos < 0)
			m_yNewPos = 0;
		break; 

	case SB_LINEDOWN: 
		// Check before increment
		if (m_yNewPos < SIM_VARS_COUNT - m_nLinesCount)
			++m_yNewPos; 
		break; 

	case SB_THUMBPOSITION: 
		m_yNewPos = HIWORD(wParam); 
		break; 

	default: 
		break;
	} 

	// Reset the scroll bar. 
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_POS; 
	si.nPos   = m_yNewPos; 
	SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE); 

	// Do nothing without this ...
	InvalidateRect(NULL, FALSE);
}

VOID NDisplay::OnRightButtonUp(int xPos, int yPos)
{
	POINT ptMouse;
	ptMouse.x = xPos;
	ptMouse.y = yPos;

	if (FontSelectDialog())
	{
		InvalidateRect(NULL, FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////

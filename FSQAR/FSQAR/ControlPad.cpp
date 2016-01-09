
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
#include "SimClient.h"


// Debug version of CRT library uses this operator
#ifdef _DEBUG
#ifndef DEBUG_NEW
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new DEBUG_NEW
#endif
#endif  // _DEBUG


CONTROLS_DEFLECTION g_CD;
double g_lever_pos = 0;

//----------------------------------------------------------------- 
// ControlPad implementation
//----------------------------------------------------------------- 


ControlPad::ControlPad()
{ 
	m_fAction = FALSE;
	m_ptCurrent.x = 0;
	m_ptCurrent.y = 0;
	m_nHeight = 200;
	m_nWidth = 200;


	SetDefaultFont();
} 


void ControlPad::SetDefaultFont()
{
	LOGFONT lf;

	// For the MM_TEXT mapping mode, you may use the following formula 
	// to specify a height for a font with a given point size.
	// lfHeight = -MulDiv(PointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	lf.lfHeight = -10;				
	lf.lfWidth = 0;
	lf.lfWeight = FW_NORMAL;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfPitchAndFamily = FF_ROMAN;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = DEFAULT_CHARSET;

	::StringCbCopy(lf.lfFaceName, LF_FACESIZE, _T("Arial"));

	m_hFont = ::CreateFontIndirect(&lf);
}

ControlPad::~ControlPad()
{ 
	TRACE0("DTOR ControlPad\n");

	DeleteObject(m_hFont);
} 


LRESULT CALLBACK ControlPadWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	if (Msg == WM_CREATE)
	{
		// Pointer to a value to be passed to the window through 
		// the CREATESTRUCT structure is out window object instance.
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		ControlPad* pCtrlPad = static_cast<ControlPad*>(pcs->lpCreateParams);
		// Store the pointer as private data of the window
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, PtrToUlong(pCtrlPad)); 
		pCtrlPad->Attach(hWnd);
		pCtrlPad->OnCreate();
		return 0;
	}

	// Extract the pointer to out window object
	ControlPad* pCtrlPad = static_cast<ControlPad*>(UlongToPtr(::GetWindowLongPtr(hWnd, GWLP_USERDATA)));

	if (pCtrlPad)
	{
		pCtrlPad->WindowProc(Msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);

}

LRESULT CALLBACK ControlPad::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_DISPLAYCHANGE:
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(m_hWnd, &ps);
			OnPaint(ps.hdc);
			EndPaint(m_hWnd, &ps);
		}
		return 0;

	case WM_ERASEBKGND:
		return 0;

	case WM_SIZE:
		OnSize(LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_RBUTTONUP:
		OnRightButtonUp(LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove(LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_MOUSEWHEEL:
		//pCtrlPad->OnMouseWheel(HIWORD(wParam));
		return 0;

	case WM_KEYDOWN: 
		OnKeyDown(wParam);
		return 0;

	case WM_CLOSE:
		DestroyWindow();
		return 0;

	default:
		return DefaultWindowProc(Msg, wParam, lParam);
	}

	return DefaultWindowProc(Msg, wParam, lParam);
}

BOOL ControlPad::Create(const Window* pParent, LPCRECT lpRect)
{
	return Window::Create(
		WS_EX_PALETTEWINDOW,
		szControlPadWndClass, 
		_T("Control your airplane"), 
		dwControlPadWndStyle, 
		lpRect,  
		pParent, 
		0, this);
}

void ControlPad::OnCreate()
{

	static SimData* pVars = SimData::GetSimData();

	Lock lock(&g_csSimData);
	g_lever_pos = (int)pVars->GetVariable(LEVER)->Value;

	g_CD.elevator = pVars->GetVariable(EDEF)->Value;
	g_CD.aileron = pVars->GetVariable(ADEF)->Value;
	g_CD.rudder = pVars->GetVariable(RDEF)->Value;

	g_CD.lever1 = g_lever_pos;
	g_CD.lever2 = g_lever_pos;
	g_CD.lever3 = g_lever_pos;
	g_CD.lever4 = g_lever_pos;
	
	lock.Release();

	SimData::FreeSimData();

	SendToSimulator();

}

void ControlPad::OnSize(UINT nWidth, UINT nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	//InvalidateRect(NULL, FALSE);
}

void ControlPad::DrawBackground(HDC hDC)
{
	RECT rc;
	SetRect(&rc, 0, 0, m_nWidth, m_nHeight);
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDC, hBrush);
	FillRect(hDC, &rc, hBrush);
	// Free GDI resources (!!!)
	SelectObject(hDC, hOldBrush);
	DeleteObject((HGDIOBJ)hBrush);
}

#define CIRCLE_SIZE	8

void ControlPad::DrawCross(HDC hDC)
{
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);
	// vertical line
	MoveToEx(hDC, m_nWidth/2, 0, NULL);
	LineTo(hDC, m_nWidth/2, m_nHeight);
	// horizontal line
	MoveToEx(hDC, 0, m_nHeight/2, NULL);
	LineTo(hDC, m_nWidth, m_nHeight/2);

	Ellipse(hDC, 
		m_nWidth/2-CIRCLE_SIZE, 
		m_nHeight/2-CIRCLE_SIZE, 
		m_nWidth/2+CIRCLE_SIZE, 
		m_nHeight/2+CIRCLE_SIZE);
	// free GDI objects
	SelectObject(hDC, hOldPen);
	DeleteObject((HGDIOBJ)hPen);
}

void ControlPad::DrawTextLabels(HDC hDC)
{

	size_t nSize;
	TCHAR szText[SHORT_STR_LEN];

	Lock lock(&g_csSimData);

	static SimData* pVariables = SimData::GetSimData();


	::SelectObject(hDC, m_hFont);
	::SetTextColor(hDC, RGB(255, 0, 0));

	//---------------- Elevator --------------------
	StringCbPrintf(szText, SHORT_STR_LEN, 
		_T("Elevator: %5.3f"), 
		pVariables->GetVariable(EDEF)->Value);
	if (SUCCEEDED(StringCbLength(szText, SHORT_STR_LEN, &nSize)))
	{
		TextOut(hDC,
			m_nWidth / 2 + 10,
			8,
			szText, (int)nSize / sizeof(TCHAR));
	}


	//------------------ Aileron --------------------
	StringCbPrintf(szText, SHORT_STR_LEN, 
		_T("Ailerons: %5.3f"), 
		pVariables->GetVariable(ADEF)->Value);

	if (SUCCEEDED(StringCbLength(szText, SHORT_STR_LEN, &nSize)))
	{
		TextOut(hDC,
			m_nWidth - 82,
			m_nHeight / 2 - 16,
			szText, (int)nSize / sizeof(TCHAR));
	}

	//------------------ Rudder ----------------------
	StringCbPrintf(szText, SHORT_STR_LEN, 
		_T("Rudder: %5.3f"), 
		pVariables->GetVariable(RDEF)->Value);

	if (SUCCEEDED(StringCbLength(szText, SHORT_STR_LEN, &nSize)))
	{
		TextOut(hDC,
			m_nWidth / 2 + 10,
			m_nHeight - 16,
			szText, (int)nSize / sizeof(TCHAR));
	}

	//------------------ Lever ----------------------
	StringCbPrintf(szText, SHORT_STR_LEN, 
		_T("Levers: %5.3f"), 
		pVariables->GetVariable(LEVER)->Value);

	if (SUCCEEDED(StringCbLength(szText, SHORT_STR_LEN, &nSize)))
	{
		TextOut(hDC,
			10,
			10,
			szText, (int)nSize / sizeof(TCHAR));
	}

	//--------------------------------------------

	SimData::FreeSimData();

	lock.Release();

}

#define CONTROL_SIZE	8

void ControlPad::DrawControl(HDC hDC, LPPOINT pptPos)
{
	HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 255));
	HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);
	// vertical line
	MoveToEx(hDC, pptPos->x, pptPos->y-CONTROL_SIZE, NULL);
	LineTo(hDC, pptPos->x, pptPos->y+CONTROL_SIZE);
	// horizontal line
	MoveToEx(hDC, pptPos->x-CONTROL_SIZE, pptPos->y, NULL);
	LineTo(hDC, pptPos->x+CONTROL_SIZE, pptPos->y);

	// free GDI objects
	SelectObject(hDC, hOldPen);
	DeleteObject((HGDIOBJ)hPen);
}


void ControlPad::SetControlDeflection()
{

	 

}

void ControlPad::OnPaint(HDC hDC)
{
	RECT rcClient;
	GetClientRect(&rcClient);

	// Standard Setup
	HDC hMemDC = ::CreateCompatibleDC(hDC);
	if (hMemDC)
	{
		HBITMAP hBmp   = ::CreateCompatibleBitmap(hDC, m_nWidth, m_nHeight);
		HGDIOBJ hOldBmp = ::SelectObject(hMemDC, hBmp);
	
		DrawBackground(hMemDC);
		DrawHint(hMemDC);
		DrawCross(hMemDC);
		DrawTextLabels(hMemDC);
		POINT ptDeflection;

		if (m_fAction)
		{
			ptDeflection.x = m_ptCurrent.x;
			ptDeflection.y = m_ptCurrent.y;
		}
		else
		{
			ptDeflection.x = m_nWidth/2;
			ptDeflection.y = m_nHeight/2;
		}

		DrawControl(hMemDC, &ptDeflection);

		// Copies the source rectangle directly to the destination rectangle
		::BitBlt(hDC, 0, 0, m_nWidth, m_nHeight, hMemDC, 0, 0, SRCCOPY);

		// Standard End
		::SelectObject(hMemDC, hOldBmp);
		::DeleteDC(hMemDC);
		::DeleteObject(hBmp); 
	}
	else
	{
		// Error handler here
		TRACE2("Memory DC failed: %#X, %d\n", hDC, GetLastError());
	}

}



void ControlPad::OnRightButtonUp(UINT xPos, UINT yPos)
{
	// Reserved
	UNREFERENCED_PARAMETER(xPos);
	UNREFERENCED_PARAMETER(yPos);

	if (m_fAction)
	{
		static SimData* pVars = SimData::GetSimData();
		m_fAction = FALSE;
		m_ptCurrent.x = m_nWidth/2;
		m_ptCurrent.y = m_nHeight/2;
		
		Lock lock(&g_csSimData);
		g_lever_pos = (int)pVars->GetVariable(LEVER)->Value;
		lock.Release();

		SimData::FreeSimData();
		

	} 
	else
	{
		POINT ptCursor;
		ptCursor.x = m_nWidth/2;
		ptCursor.y = m_nHeight/2;
		m_fAction = TRUE;
		ClientToScreen(m_hWnd, &ptCursor);
		SetCursorPos(ptCursor.x, ptCursor.y);

	}
	
}

void ControlPad::OnMouseMove(int xPos, int yPos)
{
	if (m_fAction)
	{
		m_ptCurrent.x = xPos;
		m_ptCurrent.y = yPos;

		int xScale = 32786/m_nWidth;
		int yScale = 32786/m_nHeight;
		int xDef = m_ptCurrent.x*xScale - 16384;
		int yDef = m_ptCurrent.y*yScale - 16384;

		g_CD.elevator = (double)yDef/16384;
		g_CD.aileron = (double)xDef/16384;
		g_CD.rudder = 0.0;
		g_CD.lever1 = g_lever_pos;
		g_CD.lever2 = g_lever_pos;
		g_CD.lever3 = g_lever_pos;
		g_CD.lever4 = g_lever_pos;

		InvalidateRect(NULL, FALSE);

		SendToSimulator();
	} 
	
}


void ControlPad::OnKeyDown(UINT_PTR nKeyCode)
{
	switch (nKeyCode) 
	{ 
	case VK_F1: 
		{

			g_lever_pos = 0;
		
		}
		break;

	case VK_F2: 
		{
			if (g_lever_pos >= 1.0f)
				g_lever_pos -=1.0f;
			
		}
		break; 

	case VK_F3: 
		{
			if (g_lever_pos <= 99.0f)
				g_lever_pos +=1.0f;
			SendToSimulator();
		}
		break; 

	case VK_F4: 
		{
			g_lever_pos =100.0f;

		}
		break; 

	default: 
		break; 
	}
	g_CD.lever1 = g_lever_pos;
	g_CD.lever2 = g_lever_pos;
	g_CD.lever3 = g_lever_pos;
	g_CD.lever4 = g_lever_pos;

SendToSimulator();
}

void ControlPad::SendToSimulator()
{

#ifndef TEST_WITHOUT_SIM
	if (m_fAction)
	{
		AfxGetApp()->GetSimClient()->SendDataToSimulator(
			DEFINITION_CONTROLS,
			sizeof(CONTROLS_DEFLECTION),
			&g_CD);
	}

#endif

}

void ControlPad::DrawHint(HDC hDC)
{
	LOGFONT lf;
	HFONT hFont, hOldFont;
	TCHAR szText[2*SHORT_STR_LEN];
	RECT rc;

	lf.lfHeight = -12;
	lf.lfWidth = 0;
	lf.lfWeight = FW_BOLD;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfPitchAndFamily = FF_ROMAN;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = DEFAULT_CHARSET;

	::StringCbCopy(lf.lfFaceName, LF_FACESIZE, _T("Courier"));

	hFont = ::CreateFontIndirect(&lf);
	hOldFont = (HFONT)::SelectObject(hDC, hFont);
	::SetTextColor(hDC, RGB(200, 200, 200));
	::SetRect(&rc, 10, 120, 220, 200);

	// TODO: Put this string into Resource
	StringCbPrintf(szText, 2*SHORT_STR_LEN, _T("%s"), _T("Mouse right click\ntoggles behavior"));

	// DrawText
	// If nCount is -1, then the lpchText parameter is assumed to be a pointer 
	// to a null-terminated string and DrawText computes the character count automatically.
	//if (SUCCEEDED(StringCbLength(szText, SHORT_STR_LEN, &nSize)))
	::DrawText(hDC,
		szText, -1,
		&rc,
		DT_LEFT);

	// Cleanup
	::SelectObject(hDC, hOldFont);
	::DeleteObject((HGDIOBJ)hFont);
}

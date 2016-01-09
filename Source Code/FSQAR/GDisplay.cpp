
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
#include "SimVarIDS.h"
#include "Registry.h"
#include "resource.h"

#include "..\SaveJpeg\jpge.h"


// Debug version of CRT library uses this operator
#ifdef _DEBUG
#ifndef DEBUG_NEW
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new DEBUG_NEW
#endif
#endif  // _DEBUG

GraphData g_GraphData;

#ifdef TEST_WITHOUT_SIM
#define ID_TEST_TIMER	345
#endif

// if TEST_WITHOUT_SIM it generates warning C4100
#pragma warning(push)
#pragma warning(disable:4100)	// unreferenced formal parameter
VOID CALLBACK GDisplayTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent,DWORD dwTime)
{
#ifdef TEST_WITHOUT_SIM

	// Add random test data for g-force
	int rand_value = rand()%100;
	double dbl_value = 1.0 + (80 - rand_value)/100.0;
	g_GraphData.AddValue(0, dbl_value);

	GDisplay* pDisplay = AfxGetApp()->GetGDisplay();

	if (g_GraphData.GetSize() >= 512)
		pDisplay->ResetBuffer(INIT_BUFFER_VALUE);
	
	pDisplay->Update(&g_GraphData);

#else

	// do nothing

#endif // TEST_WITHOUT_SIM
}
#pragma warning(pop)

void GraphData::ResetBuffer(double InitValue)
{
	Lock lock(&g_csSimData);
	for (int i =0; i < MAX_BUFFER_SIZE; i++)
	{
		for (int k = 0; k < MAX_CHART_COUNT; k++)
			SetBufferValue(k, i, InitValue);
	}

	SetSize(0);
	lock.Release();
}

GraphData::GraphData()
{
	m_nSize = 0;

	m_nSelectedItem[0] = GFORCE;
	m_nSelectedItem[1] = VSPEED;
	m_nSelectedItem[2] = ALPHA;

	//ResetBuffer(1.0);
	// Initialization !!!
	g_GraphData.m_nSelectedItem[0] = m_nSelectedItem[0];
	g_GraphData.m_nSelectedItem[1] = m_nSelectedItem[1];
	g_GraphData.m_nSelectedItem[2] = m_nSelectedItem[2];
}

#ifdef TEST_WITHOUT_SIM
void GraphData::AddValue(int nLine, double Value)
{
	m_dBuffer[nLine][m_nSize] = Value;
	m_nSize++;	
}
#endif



//------------------------------------------------------------------------
// BorderRect
// 
// It draws a rectangle which is not filled by using the current brush. 
// This is difference from GDI API 'Rectangle'
// 
// Remarks:
// 
// 1) The LineTo function draws a line from the current position up to, 
// but not including, the specified point !!! This is the reason why +1
// should be added to end position.
// 2) If LineTo succeeds, the current position is set to the specified 
// ending point. Then it is not necessary to move position after each
// drawing.
//------------------------------------------------------------------------
void BorderRect(HDC hDC, LPRECT lpRect, COLORREF clrPenColor)
{
	ASSERT(lpRect);
	__analysis_assume(lpRect);

	// create GDI objects
	HPEN hPen = CreatePen(PS_SOLID, 1, clrPenColor);
	HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);
	// upper line
	MoveToEx(hDC, lpRect->left, lpRect->top, NULL);
	LineTo(hDC, lpRect->right, lpRect->top);
	// right line
	LineTo(hDC, lpRect->right, lpRect->bottom);
	// bottom line
	LineTo(hDC, lpRect->left, lpRect->bottom);
	// left line
	LineTo(hDC, lpRect->left, lpRect->top-1); // +1 see above !!!
	// free GDI objects
	SelectObject(hDC, hOldPen);
	DeleteObject((HGDIOBJ)hPen);
}



//-----------------------------------------------------------------------
// Maps a value of 'double' type to value of 'long' type
// 
// when d = d1 then l = l1
// when d = d2 then l = l2
// any intermediate values are calculate according to the expression 
// for linear function: l=l(d)=k(d-d1)+l1, where k = (l2 - l1)/(d2 - d1)
//
// TODO: You must add error handler here !!!
//-----------------------------------------------------------------------
BOOL dtoi(long iMin, long iMax,  
		 double dMin, double dMax, 
		 double val, long* pResult)
{
	// interpolates linear function l=l(d)=k(d-d1)+l1
	double k;
	double denominator = dMax - dMin;

	if (denominator < DBL_MIN)
	{
		// TODO: Fixme
		TRACE0("An invalid argument is passed to 'dtoi' function.\n");
		return FALSE;
	}
	k = (iMax - iMin)/denominator;
	*pResult = (long)(k*(val - dMin)) + iMin;
	return TRUE;
}


LRESULT CALLBACK GDisplayWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_CREATE)
	{
		// Pointer to a value to be passed to the window through 
		// the CREATESTRUCT structure is out window object instance.
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		GDisplay* pGDisplay = static_cast<GDisplay*>(pcs->lpCreateParams);
		// Store the pointer as private data of the window
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, PtrToUlong(pGDisplay)); 
		pGDisplay->Attach(hWnd);
		pGDisplay->OnCreate();
		return 0;
	}

	// Extract the pointer to out window object
	GDisplay* pGDisplay = static_cast<GDisplay*>(UlongToPtr(::GetWindowLongPtr(hWnd, GWLP_USERDATA)));

	if (pGDisplay)
	{
		PAINTSTRUCT ps;
		switch (Msg)
		{
		case WM_SIZE:
			pGDisplay->OnSize(wParam, lParam);
			return 0;

		case WM_ERASEBKGND:
			return 0;

		case WM_DISPLAYCHANGE:
		case WM_PAINT:
			{
				BeginPaint(hWnd, &ps);
				pGDisplay->OnPaint(ps.hdc);
				EndPaint(hWnd, &ps);
			}
			return 0;

		case WM_DESTROY:
			pGDisplay->OnDestroy();
			break;

		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
		}
	}

	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

LRESULT CALLBACK GDisplay::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return DefaultWindowProc(Msg, wParam, lParam);
}


GDisplay::GDisplay()
{
	m_clrBorder = RGB(239, 238, 176);
	m_clrGrid = RGB(127, 127, 127);
	m_clrText = RGB(210, 255, 134);

	m_xUpdatePosition = 0;

	m_dxText = 48;
	m_dyText = 8;

		
	m_clrBackDark = RGB(28, 24, 78);
	m_clrBackLight = RGB(32, 52, 90);

	// --------------------------------------------------------------------
	//m_clrBackDark = RGB(137, 204, 235);	// This is the logo color (!!!)
	//m_clrBackLight = RGB(140, 200, 240);
	// --------------------------------------------------------------------

	m_clrUpdateLine = RGB(181, 239, 29);
	m_xMinStep = DEFAULT_XSTEP;	
	//m_nScreenBufferSize = DEF_SCREEN_BUFFER_SIZE;
	m_nClientWidth = 200;
	m_nClientHeight = 200;

	m_fManoeuverable = FALSE;
	m_nChartType = CHART_TYPE_SIMPLE;

	// Set default value
	ResetBuffer(INIT_BUFFER_VALUE);


	m_xCount = 16;
	m_yCount = 16;

	LOGFONT lf;

	lf.lfHeight = -12;			
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

	m_hFont = ::CreateFontIndirect(&lf);

	m_nChartCount = MAX_CHART_COUNT;

	m_clrMarker[0] = RED;
	m_clrMarker[1] = GREEN;
	m_clrMarker[2] = YELLOW;
}

GDisplay::~GDisplay()
{
	TRACE0("DTOR GDisplay\n");
	DeleteObject(m_hFont);
}


BOOL GDisplay::Create(const Window* pFrameWnd, LPCRECT lpRect)
{
	return Window::Create(0, szGDisplayWndClass, NULL, 
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		lpRect, pFrameWnd, 0, this);
}


void GDisplay::OnCreate()
{
	ReadOptions();
	SetChartType(CHART_TYPE_SIMPLE);
}

void GDisplay::OnDestroy()
{
	SaveOptions();
}

void GDisplay::OnSize(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);	

	m_nClientWidth = LOWORD(lParam);
	m_nClientHeight = HIWORD(lParam);

	// Use this call instead InvalidateRect()
	Update(&g_GraphData);	// update chart
}



// -----------------------------------------------------------------
// It just draws a simple line between two given point but moreover
// it validates margins of the drawing
// 
// It is important to remove 'CreatePen' from this function's body
// to improve performance speed
// -----------------------------------------------------------------
void GDisplay::DrawMarker(HDC hDC, LPPOINT ppOld, LPPOINT ppNew, int nRightBorder, COLORREF clrMarker)
{
	if ((ppOld->x <= nRightBorder) && (ppNew->x <= nRightBorder))
	{
		// TODO: Think about the width of the pen
		HPEN hPen = CreatePen(PS_SOLID, 2, clrMarker);
		HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);

		MoveToEx(hDC, ppOld->x, ppOld->y, NULL);
		LineTo(hDC, ppNew->x, ppNew->y);
		 
		SelectObject(hDC, hOldPen);
		DeleteObject((HGDIOBJ)hPen);
	}
}


// ------------------------------------------------------------
// Draw a vertical line that separates new values from old one.
// Current position of the line is xUpdatePosition.
// The lpRect parameter is rectangle of chart. This parameter 
// prevents going of the line out of bound.
// -------------------------------------------------------------
void GDisplay::DrawUpdateLine(HDC hDC,  UINT xUpdatePosition,  LPRECT lpRect)
{
	HPEN hPen = CreatePen(PS_SOLID, 1, m_clrUpdateLine);
	HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);
	int x = lpRect->left + xUpdatePosition;
	if (x > lpRect->right)
	{
		x = lpRect->right;
	}
	MoveToEx(hDC, x, lpRect->bottom, NULL);
	LineTo(hDC, x, lpRect->top);

	SelectObject(hDC, hOldPen);
	DeleteObject((HGDIOBJ)hPen);
}


// Fill display buffer of default values
void GDisplay::ResetBuffer(double dInitValue)
{
	for (int i = 0; i < MAX_BUFFER_SIZE; i++)
	{
		for (int k = 0; k < MAX_CHART_COUNT; k++)
			m_dScreenBuffer[k][i] = dInitValue;
	}
}



// HERE: lpRect is available rect for painting
void GDisplay::DrawGrid(HDC hDC, LPRECT lpRect)
{
	// They cannot be zero
	ASSERT(m_xCount);
	ASSERT(m_yCount);

	// Calculate dynamic step on X and Y axis
	div_t dx = div((int)(lpRect->right - lpRect->left), (int)m_xCount);
	div_t dy = div((int)(lpRect->bottom - lpRect->top), (int)m_yCount);

	int xStep = dx.quot;
	int yStep = dy.quot;

	// Calculate margins of drawing
	m_nMarginTop = lpRect->top;
	m_nMarginRight = lpRect->left + m_dxText + xStep*(m_xCount-2);
	m_nMarginBottom = lpRect->top + yStep*(m_yCount-1);

	// Set appropriate rectangle
	RECT m_rcWork;
	::SetRect(&m_rcWork, 
		lpRect->left+m_dxText,  
		lpRect->top, 
		m_nMarginRight, 
		m_nMarginBottom);

	// Fill old values area by dark color
	RECT rcUpdate;

	SetRect(&rcUpdate,
		m_rcWork.left,
		m_rcWork.top, 
		m_rcWork.left+m_xUpdatePosition, 
		m_rcWork.bottom);

	MyFillRect(hDC, &rcUpdate, m_clrBackDark);

	// Fill new values area by light color
	SetRect(&rcUpdate,
		m_rcWork.left+m_xUpdatePosition,
		m_rcWork.top, 
		m_rcWork.right, 
		m_rcWork.bottom);

	MyFillRect(hDC, &rcUpdate, m_clrBackLight);

	//
	// Draw proper grid
	// 

	HPEN hPen = CreatePen(PS_SOLID, 1, m_clrGrid);
	HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);

	// draw lines along X axis (prefix 'y')
	for (int a = 0; a < m_yCount; a++)
	{
		MoveToEx(hDC, m_rcWork.left, m_rcWork.top + yStep*a, NULL);
		LineTo(hDC, m_rcWork.right, m_rcWork.top + yStep*a);
	}

	// draw lines along Y axis (prefix 'x')
	for (int a = 0; a < m_xCount-1; a++)
	{
		MoveToEx(hDC, m_rcWork.left + xStep*a, m_rcWork.top, NULL);
		LineTo(hDC, m_rcWork.left + xStep*a, m_rcWork.bottom);
	}

	SelectObject(hDC, hOldPen);
	DeleteObject((HGDIOBJ)hPen);

	if (m_nChartType == CHART_TYPE_SIMPLE)
	{
		long y;
		dtoi(m_rcWork.bottom, m_rcWork.top, 0.75, 2.00, 1.0, &y);

		hPen = CreatePen(PS_SOLID, 1, RGB(43,123,89));
		hOldPen = (HPEN)SelectObject(hDC, hPen);

		MoveToEx(hDC, m_rcWork.left, y, NULL);
		LineTo(hDC, m_rcWork.right, y);

		SelectObject(hDC, hOldPen);
		DeleteObject((HGDIOBJ)hPen);
	}

	//
	// Output text labels
	//
	
	HFONT hOldFont = (HFONT)SelectObject(hDC, m_hFont);

	SetBkColor(hDC, m_clrBackDark);

	// ------------ first ----------------------
	SetTextColor(hDC, m_clrMarker[0]);
	for (int a = 0; a < m_yCount; a++)
	{
		size_t nSize;
		if (SUCCEEDED(StringCbLength(m_szLabel1[a], MAX_LABEL_LENGTH, &nSize)))
		{
			TextOut(hDC,
				m_rcWork.left - m_dxText,
				m_rcWork.top + yStep*a - m_dyText,
				m_szLabel1[a], (int)nSize / sizeof(TCHAR));
		}

	}
		
	if (m_nChartType != CHART_TYPE_SIMPLE)
	{
		// ------------ second ----------------------
		SetTextColor(hDC, m_clrMarker[1]);
		for (int a = 0; a < m_yCount; a++)
		{
			size_t nSize;
			if (SUCCEEDED(StringCbLength(m_szLabel2[a], MAX_LABEL_LENGTH, &nSize)))
			{
				TextOut(hDC,
					m_rcWork.left + 6,
					m_rcWork.top + yStep*a + 2/* + m_dyText*/,
					m_szLabel2[a], (int)nSize / sizeof(TCHAR));
			}


		}

		// -------------- third ------------------------
		SetTextColor(hDC, m_clrMarker[2]);
		for (int a = 0; a < m_yCount; a++)
		{
			size_t nSize;
			if (SUCCEEDED(StringCbLength(m_szLabel3[a], MAX_LABEL_LENGTH, &nSize)))
			{

				TextOut(hDC,
					m_rcWork.right + 6,
					m_rcWork.top + yStep*a - m_dyText,
					m_szLabel3[a], (int)nSize / sizeof(TCHAR));
			}

		}
	}
	// -------------------------------------------------
	SelectObject(hDC, hOldFont);

	//
	// Draw light border
	// 

	hPen = CreatePen(PS_SOLID, 1, m_clrBorder);
	hOldPen = (HPEN)SelectObject(hDC, hPen);

	// top border
	MoveToEx(hDC, m_rcWork.left, m_rcWork.top, NULL);
	LineTo(hDC, m_nMarginRight, m_rcWork.top);
	// right border
	LineTo(hDC, m_nMarginRight, m_nMarginBottom);
	// bottom border
	LineTo(hDC, m_rcWork.left, m_nMarginBottom);
	// left border
	LineTo(hDC, m_rcWork.left, m_rcWork.top);

	SelectObject(hDC, hOldPen);
	DeleteObject((HGDIOBJ)hPen);

}


// 
// lpRect This is the owner window client area
// xShift shift on X axis from the client area rectangle
// yShift shift on Y axis from the client area rectangle
// 
//
// Call by OnPaint as DrawChart(hMemDC, &rcClient, dx, dy);
//
void GDisplay::DrawChart( HDC hDC,  LPRECT lpRect, INT xShift, INT yShift)
{
	// At this point lpRect is owner window client rectangle
	MyFillRect(hDC, lpRect, m_clrBackDark);
	InflateRect(lpRect, -xShift, -yShift);

	// Uncomment the next line to see proper chart rectangle
	//BorderRect(hDC, lpRect, RGB(255,255,255));

	DrawGrid(hDC, lpRect);


	// Form rectangle for painting

	SetRect(&m_rcChart, 
		xShift + m_dxText,
		lpRect->top, 
		m_nMarginRight,		// These margings came from DrawGrid() 
		m_nMarginBottom);	// where they are calculated

	// How many items the current screen can show ?
	// Note: m_xMinStep forced to be set to DEFAULT_XSTEP ('4' pixels)
	m_nScreenBufferSize = (m_rcChart.right - m_rcChart.left)/m_xMinStep;

	if (m_nScreenBufferSize < MIN_SCREEN_BUFFER_SIZE)
		m_nScreenBufferSize = MIN_SCREEN_BUFFER_SIZE;

	if (m_nScreenBufferSize > MAX_SCREEN_BUFFER_SIZE)
		m_nScreenBufferSize = MAX_SCREEN_BUFFER_SIZE;


	POINT ptOld[MAX_CHART_COUNT];	// Start point
	POINT ptNew[MAX_CHART_COUNT];	// End point

	double new_value[MAX_CHART_COUNT];
	double old_value[MAX_CHART_COUNT];

	for (int i = 0; i < m_nScreenBufferSize; i++)
	{
		for (int k = 0; k < m_nChartCount; k++)
		{
			new_value[k] = m_dScreenBuffer[k][i+1];
			old_value[k] = m_dScreenBuffer[k][i];


			ptOld[k].x = m_xMinStep*i + m_rcChart.left;
			ptNew[k].x = m_xMinStep*(i+1)+ m_rcChart.left;

			// Convert actual value to screen units
			if (!dtoi(m_rcChart.bottom, 
				m_rcChart.top, 
				m_dblMin[k],
				m_dblMax[k], 
				old_value[k], &(ptOld[k].y)))
			{
				ptOld[k].y = m_rcChart.bottom;
			}

			if (!dtoi(m_rcChart.bottom, 
				m_rcChart.top, 
				m_dblMin[k], 
				m_dblMax[k],  
				new_value[k], &(ptNew[k].y)))
			{
				ptNew[k].y = m_rcChart.bottom;	
			}


			if (ptOld[k].y <= m_rcChart.top)
				ptOld[k].y = m_rcChart.top;

			if (ptOld[k].y >= m_rcChart.bottom)
				ptOld[k].y = m_rcChart.bottom;

			if (ptNew[k].y <= m_rcChart.top)
				ptNew[k].y = m_rcChart.top;

			if (ptNew[k].y >= m_rcChart.bottom)
				ptNew[k].y = m_rcChart.bottom;

			// we out of the right border
			if (ptNew[k].x >= m_rcChart.right)
			{
				ResetBuffer(INIT_BUFFER_VALUE);
			}

			DrawMarker(hDC, &ptOld[k], &ptNew[k], m_rcChart.right, m_clrMarker[k]);
			DrawUpdateLine(hDC, m_xUpdatePosition, &m_rcChart);
			DrawDescription(hDC, &m_rcChart);			


		} // for (k < m_nChartCount)
	} // for (i < m_nScreenBufferSize)
}

void GDisplay::DrawDescription(HDC hDC, LPRECT lpRect)
{
	TCHAR szTitle[LONG_STR_LEN];
	size_t nSize;

	HFONT hOldFont = (HFONT)::SelectObject(hDC, m_hFont);

	for (int k = 0; k < m_nChartCount; k++)
	{
		UINT id = SIMVAR_BASE_ID + m_nSimVarID[k];
		if(MyLoadString(id, szTitle, LONG_STR_LEN))
		{
			if(SUCCEEDED(StringCbLength(szTitle, LONG_STR_LEN, &nSize)))
			{
				SetTextColor(hDC, m_clrMarker[k]);

				TextOut(hDC, 
					lpRect->left - m_dxText, 
					lpRect->bottom + 16 + 18*k, 
					szTitle, (int)nSize/sizeof(TCHAR));	
			}
		}
	}

	::SelectObject(hDC, hOldFont);

}

void GDisplay::OnPaint(HDC hDC)
{

	// Standard Setup
	HDC hMemDC = ::CreateCompatibleDC(hDC);
	if (hMemDC)
	{
		UINT dx = X_SHIFT_TO_CHART;
		UINT dy = Y_SHIFT_TO_CHART;
		RECT rcClient;
		::SetRect(&rcClient, 0, 0, m_nClientWidth, m_nClientHeight);
		HBITMAP hBmp   = ::CreateCompatibleBitmap(hDC, m_nClientWidth, m_nClientHeight);
		HGDIOBJ hOldBmp = ::SelectObject(hMemDC, hBmp);

		// Draw only on hMemDC
		// Delegate main work to the DrawChart function
		// 

		Lock lock(&g_csSimData);
		DrawChart(hMemDC, &rcClient, dx, dy);
		lock.Release();

		// My Logo !!!
		//DrawLogo(hMemDC, rcClient.right - 120, rcClient.bottom - 30);

		// Copies the source rectangle directly to the destination rectangle
		::BitBlt(hDC, 0, 0, m_nClientWidth, m_nClientHeight, hMemDC, 0, 0, 
			SRCCOPY);

		// Standard End
		::SelectObject(hMemDC, hOldBmp);
		::DeleteDC(hMemDC);
		::DeleteObject(hBmp); 
	}
	else
	{
		TRACE2("Memory DC was not created: %#X, Error code: %d\n", 
			hDC, GetLastError());
	}
}

// WARNING: This function is called in another thread
BOOL GDisplay::Update( GraphData* pGraphData)
{

#ifndef _NO_INTEL_ADVISOR
	ANNOTATE_SITE_BEGIN(UpdateGDisplayData);
#endif


	Lock lock(&g_csSimData);

	if (pGraphData->GetSize() >= m_nScreenBufferSize)
	{
		ResetBuffer(INIT_BUFFER_VALUE);
		pGraphData->SetSize(0);
	}


	for (int k = 0; k < MAX_CHART_COUNT; k++)
	{

#ifndef _NO_INTEL_ADVISOR
		ANNOTATE_ITERATION_TASK(ExternalLoop);
#endif
		for (int i = 0; i < MAX_BUFFER_SIZE; i++)
		{

#ifndef _NO_INTEL_ADVISOR
			ANNOTATE_ITERATION_TASK(InternalLoop);
#endif
			m_dScreenBuffer[k][i] = pGraphData->GetBufferValue(k, i);

		}
	}

	m_xUpdatePosition = pGraphData->GetSize()*m_xMinStep;
	lock.Release();

#ifndef _NO_INTEL_ADVISOR
	ANNOTATE_SITE_END();
#endif

	// TODO: This call failed when m_hWnd == NULL
	// before window is created
	InvalidateRect(NULL, FALSE);

	return TRUE;
}

void GDisplay::SetChartType(enum CHART_TYPE nType)
{
	if (m_fManoeuverable)
	{
		ChartForManoeuverable(nType);
	} 
	else
	{
		ChartForNonManoeuverable(nType);
	}
}

void GDisplay::UpdateChartType()
{
	SetChartType(m_nChartType);
}


void GDisplay::ChartForNonManoeuverable(__in enum CHART_TYPE nType)
{
	switch (nType)
	{
	case CHART_TYPE_LANDING:
		{
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_LANDING;
			g_GraphData.ResetBuffer(0);
	

			m_nSimVarID[0] = GFORCE;
			m_nSimVarID[1] = VSPEED;
			m_nSimVarID[2] = ALPHA;

			m_dblMin[0] = 0.50;
			m_dblMax[0] = 2.25;

			m_dblMin[1] = -2000;
			m_dblMax[1] = 2000;

			m_dblMin[2] = 0;
			m_dblMax[2] = 8;
		}
		break;

	case CHART_TYPE_ANGLES:
		{
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_ANGLES;
			g_GraphData.ResetBuffer(0);

			m_nSimVarID[0] = GFORCE;
			m_nSimVarID[1] = ALPHA;
			m_nSimVarID[2] = BETA;

			m_dblMin[0] = 0.50;
			m_dblMax[0] = 2.25;

			m_dblMin[1] = 0;
			m_dblMax[1] = 8;

			m_dblMin[2] = -8;
			m_dblMax[2] = 8;
		}
		break;

	case CHART_TYPE_PITCH:
		{
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_PITCH;
			g_GraphData.ResetBuffer(0);

			m_nSimVarID[0] = EPCT;
			m_nSimVarID[1] = PITCH;
			m_nSimVarID[2] = ALPHA;

			m_dblMin[0] = -100.0;
			m_dblMax[0] = 100.0;

			m_dblMin[1] = -20;
			m_dblMax[1] = 20;

			m_dblMin[2] = 0;
			m_dblMax[2] = 12;
		}
		break;

	case CHART_TYPE_ROLL:
		{
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_ROLL;
			g_GraphData.ResetBuffer(0);

			m_nSimVarID[0] = APCT;
			m_nSimVarID[1] = BANK;
			m_nSimVarID[2] = BETA;

			m_dblMin[0] = -100.0;
			m_dblMax[0] = 100.0;

			m_dblMin[1] = -30;
			m_dblMax[1] = 30;

			m_dblMin[2] = -8;
			m_dblMax[2] = 8;
		}
		break;

	case CHART_TYPE_YAW:
		{		
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_YAW;
			g_GraphData.ResetBuffer(0);

			m_nSimVarID[0] = RPCT;
			m_nSimVarID[1] = HMAG;
			m_nSimVarID[2] = BETA;

			m_dblMin[0] = -100;
			m_dblMax[0] = 100;

			m_dblMin[1] = 0;
			m_dblMax[1] = 365;

			m_dblMin[2] = 0;
			m_dblMax[2] = 8;
			
		}
		break;

	case CHART_TYPE_THRUST:
		{		
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_THRUST;
			g_GraphData.ResetBuffer(0);

			m_nSimVarID[0] = LEVER;
			m_nSimVarID[1] = ALPHA;
			m_nSimVarID[2] = IAS;

			m_dblMin[0] = 0;
			m_dblMax[0] = 100;

			m_dblMin[1] = 0;
			m_dblMax[1] = 12;

			m_dblMin[2] = 50;
			m_dblMax[2] = 250;
			
		}
		break;

		case CHART_TYPE_SIMPLE:
			{
				m_nChartCount = 1;
				m_nChartType = CHART_TYPE_SIMPLE;
				g_GraphData.ResetBuffer(1.0);

				m_nSimVarID[0] = GFORCE;
				m_nSimVarID[1] = 0;
				m_nSimVarID[2] = 0;

				m_dblMin[0] = 0.75;
				m_dblMax[0] = 2.00;

				m_dblMin[1] = 0;
				m_dblMax[1] = 12;

				m_dblMin[2] = 50;
				m_dblMax[2] = 250;

			}
			break;
	}

	double dblDataStep[MAX_CHART_COUNT];

	dblDataStep[0] = (m_dblMax[0] - m_dblMin[0])/(m_yCount-1);
	dblDataStep[1] = (m_dblMax[1] - m_dblMin[1])/(m_yCount-1);
	dblDataStep[2] = (m_dblMax[2] - m_dblMin[2])/(m_yCount-1);


	for (int i = 0; i < m_yCount; i++)
	{
		StringCbPrintf((LPTSTR)m_szLabel1[i], MAX_LABEL_LENGTH, 
			_T("%3.2f"), (m_dblMax[0]-dblDataStep[0]*i));
	}

	if (nType != CHART_TYPE_SIMPLE)
	{
		for (int i = 0; i < m_yCount; i++)
		{
			StringCbPrintf((LPTSTR)m_szLabel2[i], MAX_LABEL_LENGTH, 
				_T("%3.2f"), (m_dblMax[1]-dblDataStep[1]*i));
		}

		for (int i = 0; i < m_yCount; i++)
		{
			StringCbPrintf((LPTSTR)m_szLabel3[i], MAX_LABEL_LENGTH, 
				_T("%3.2f"), (m_dblMax[2]-dblDataStep[2]*i));
		}
	}

	// NOTE: Elements GraphData::m_nSelectedItem[] are used 
	// in working thread to select needed flight data,
	// i.e. as an argument of SimData::GetFlightData()

	Lock lock(&g_csSimData);

	g_GraphData.SetSelectedChartValue(0, m_nSimVarID[0]);
	g_GraphData.SetSelectedChartValue(1, m_nSimVarID[1]);
	g_GraphData.SetSelectedChartValue(2, m_nSimVarID[2]);

	lock.Release();

}


void GDisplay::SaveAsBitmap(LPCTSTR pszFileName)
{
	HDC hdcClient = GetDC(m_hWnd); 
	HDC hdcCompatible = CreateCompatibleDC(hdcClient); 
 

	HBITMAP hBmp = CreateCompatibleBitmap(hdcClient, m_nClientWidth, m_nClientHeight);
	HGDIOBJ hOldBmp = SelectObject(hdcCompatible, hBmp); 

	BitBlt(hdcCompatible, 
		0,0, 
		m_nClientWidth, m_nClientHeight, 
		hdcClient, 
		0,0, 
		SRCCOPY);
	
	
	PBITMAPINFO pbi = CreateBitmapInfoStruct(hBmp);
	if (!CreateBMPFile(pszFileName, pbi, hBmp, hdcCompatible))
	{
		AfxGetApp()->Error(IDS_ERROR_CREATE_BMP);
	}

	SelectObject(hdcCompatible, hOldBmp);
	DeleteDC(hdcCompatible);
	DeleteObject(hBmp); 

}


#ifdef TEST_WITHOUT_SIM

void GDisplay::StartTimer()
{
	m_nTimer = SetTimer(m_hWnd, ID_TEST_TIMER, 400, (TIMERPROC)GDisplayTimerProc);
	m_xMinStep = 4;
}

void GDisplay::StopTimer()
{
	KillTimer(m_hWnd, ID_TEST_TIMER);
}

#endif // TEST_WITHOUT_SIM


// ==========================================================================
// BMP files are encoded with the pixel colors in BGR order, and the JPEG 
// library expects RGB order. You'll have to reverse the red and blue bytes 
// out of each group of 3. BMP files are also organized with the bottom 
// line at the top of the file, you'll want to reverse that too.
//
// Author: Andreas Hartl
// http://tipsandtricks.runicsoft.com/Cpp/BitmapTutorial.html
// 
// ==========================================================================
BYTE* ConvertBMPToRGBBuffer (BYTE* Buffer, INT_PTR width, INT_PTR height )
{
	// first make sure the parameters are valid
	if ( ( NULL == Buffer ) || ( width == 0 ) || ( height == 0 ) )
		return NULL;

	// find the number of padding bytes

	int padding = 0;
	int scanlinebytes = width * 3;
	while ( ( scanlinebytes + padding ) % 4 != 0 )     // DWORD = 4 bytes
		padding++;
	// get the padded scanline width
	int psw = scanlinebytes + padding;

	// create new buffer
	BYTE* newbuf = new BYTE[width*height*3];

	// now we loop trough all bytes of the original buffer, 
	// swap the R and B bytes and the scanlines
	intptr_t bufpos = 0;   
	intptr_t newpos = 0;
	for ( intptr_t y = 0; y < height; y++ )
		for ( intptr_t x = 0; x < 3 * width; x+=3 )
		{
			newpos = y * 3 * width + x;     
			bufpos = ( height - y - 1 ) * psw + x;

			newbuf[newpos] = Buffer[bufpos + 2];       
			newbuf[newpos + 1] = Buffer[bufpos+1]; 
			newbuf[newpos + 2] = Buffer[bufpos];     
		}

		return newbuf;
}



void GDisplay::SaveAsJpeg(LPCTSTR pszFileName)
{
	const int req_comps = 3; // request RGB image

	HDC hdcClient = GetDC(m_hWnd); 
	HDC hdcCompatible = CreateCompatibleDC(hdcClient); 
	HBITMAP hBmp = CreateCompatibleBitmap(hdcClient, m_nClientWidth, m_nClientHeight);
	HGDIOBJ hOldBmp = SelectObject(hdcCompatible, hBmp); 

	BitBlt(hdcCompatible, 
		0,0, 
		m_nClientWidth, m_nClientHeight, 
		hdcClient, 
		0,0, 
		SRCCOPY);


	BITMAPINFOHEADER   bi;

	bi.biSize = sizeof(BITMAPINFOHEADER);    
	bi.biWidth = m_nClientWidth;    
	bi.biHeight = m_nClientHeight;  
	bi.biPlanes = 1;    
	bi.biBitCount = 24;				// RGB mode is 24 bits
	bi.biCompression = BI_RGB;    
	bi.biSizeImage = 0;				// can be 0 for 24 bit images
	bi.biXPelsPerMeter = 0x0ec4;	// paint and PSP use this values 
	bi.biYPelsPerMeter = 0x0ec4;    
	bi.biClrUsed = 0;				// we are in RGB mode and have no palette
	bi.biClrImportant = 0;			// all colors are important

	SIZE_T dwBmpSize = ((m_nClientWidth*bi.biBitCount + 31)/24)*4*m_nClientHeight;

	HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize); 
	if (hDIB)
	{
		PUCHAR lpBits = (PUCHAR)GlobalLock(hDIB);

		if (!GetDIBits(hdcClient, hBmp, 0, (WORD)bi.biHeight,
			lpBits, (LPBITMAPINFO)&bi,
			DIB_RGB_COLORS))
		{
			// error handler here...
		}

		// MUST be free later !!!
		PUCHAR pRGBBits = ConvertBMPToRGBBuffer(lpBits,
			m_nClientWidth,
			m_nClientHeight);

		// Fill in the compression parameter structure.
		jpge::params params;
		params.m_quality = 80;
		params.m_subsampling = jpge::H2V2;
		params.m_two_pass_flag = false;

		CHAR szAnsiFileName[MAX_PATH];

#ifndef _UNICODE
		StringCbCopy(szAnsiFileName, MAX_PATH, pszFileName);
#else
		_wcstombsz(szAnsiFileName, pszFileName, MAX_PATH);
#endif

		if (!jpge::compress_image_to_jpeg_file(szAnsiFileName,
			m_nClientWidth, m_nClientHeight,
			req_comps, pRGBBits, params))
		{
			// error handler here...
		}


		//Unlock and Free the DIB from the heap
		GlobalUnlock(hDIB);

		// DON'T forget free this !!!
		delete pRGBBits;
	}
   
	GlobalFree(hDIB);



	SelectObject(hdcCompatible, hOldBmp);
	DeleteDC(hdcCompatible);
	DeleteObject(hBmp);

}

void GDisplay::DrawLogo(HDC hDC, int x, int y)
{
	HDC hMemDC = ::CreateCompatibleDC(hDC);
	if (hMemDC)
	{
		HBITMAP hBmp = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LOGO));
		HGDIOBJ hOldBmp = ::SelectObject(hMemDC, hBmp);
		//::TransparentBlt(hDC, 0, 0, 100, 100, hMemDC, 0, 0, 100, 100, RGB(120,20,20));
		//PatBlt()
		//MaskBlt(hDC, 0, 0, 100, 100, hMemDC, 0, 0, hBmp1, 0, 0, SRCCOPY );
		BitBlt(hDC, x, y, 200, 200, hMemDC, 0, 0, SRCCOPY );
		::SelectObject(hMemDC, hOldBmp);
		::DeleteDC(hMemDC);
		::DeleteObject(hBmp); 
	}
}


void GDisplay::ChartForManoeuverable(__in enum CHART_TYPE nType)
{
	switch (nType)
	{
	case CHART_TYPE_LANDING:
		{
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_LANDING;
			g_GraphData.ResetBuffer(0);


			m_nSimVarID[0] = GFORCE;
			m_nSimVarID[1] = VSPEED;
			m_nSimVarID[2] = ALPHA;

			m_dblMin[0] = 0.25;
			m_dblMax[0] = 6.25;

			m_dblMin[1] = -4000;
			m_dblMax[1] = 4000;

			m_dblMin[2] = 0;
			m_dblMax[2] = 16;
		}
		break;

	case CHART_TYPE_ANGLES:
		{
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_ANGLES;
			g_GraphData.ResetBuffer(0);

			m_nSimVarID[0] = GFORCE;
			m_nSimVarID[1] = ALPHA;
			m_nSimVarID[2] = BETA;

			m_dblMin[0] = 0.25;
			m_dblMax[0] = 6.25;

			m_dblMin[1] = 0;
			m_dblMax[1] = 16;

			m_dblMin[2] = -12;
			m_dblMax[2] = 12;
		}
		break;

	case CHART_TYPE_PITCH:
		{
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_PITCH;
			g_GraphData.ResetBuffer(0);

			m_nSimVarID[0] = EPCT;
			m_nSimVarID[1] = PITCH;
			m_nSimVarID[2] = ALPHA;

			m_dblMin[0] = -100.0;
			m_dblMax[0] = 100.0;

			m_dblMin[1] = -90;
			m_dblMax[1] = 90;

			m_dblMin[2] = 0;
			m_dblMax[2] = 16;
		}
		break;

	case CHART_TYPE_ROLL:
		{
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_ROLL;
			g_GraphData.ResetBuffer(0);

			m_nSimVarID[0] = APCT;
			m_nSimVarID[1] = BANK;
			m_nSimVarID[2] = BETA;

			m_dblMin[0] = -100.0;
			m_dblMax[0] = 100.0;

			m_dblMin[1] = -90;
			m_dblMax[1] = 90;

			m_dblMin[2] = -12;
			m_dblMax[2] = 12;
		}
		break;

	case CHART_TYPE_YAW:
		{		
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_YAW;
			g_GraphData.ResetBuffer(0);

			m_nSimVarID[0] = RPCT;
			m_nSimVarID[1] = HMAG;
			m_nSimVarID[2] = BETA;

			m_dblMin[0] = -100;
			m_dblMax[0] = 100;

			m_dblMin[1] = 0;
			m_dblMax[1] = 365;

			m_dblMin[2] = 0;
			m_dblMax[2] = 12;

		}
		break;

	case CHART_TYPE_THRUST:
		{		
			m_nChartCount = 3;
			m_nChartType = CHART_TYPE_THRUST;
			g_GraphData.ResetBuffer(0);

			m_nSimVarID[0] = LEVER;
			m_nSimVarID[1] = ALPHA;
			m_nSimVarID[2] = IAS;

			m_dblMin[0] = 0;
			m_dblMax[0] = 100;

			m_dblMin[1] = 0;
			m_dblMax[1] = 16;

			m_dblMin[2] = 50;
			m_dblMax[2] = 350;

		}
		break;

	case CHART_TYPE_SIMPLE:
		{
			m_nChartCount = 1;
			m_nChartType = CHART_TYPE_SIMPLE;
			g_GraphData.ResetBuffer(1.0);

			m_nSimVarID[0] = GFORCE;
			m_nSimVarID[1] = 0;
			m_nSimVarID[2] = 0;

			m_dblMin[0] = 0.25;
			m_dblMax[0] = 6.25;

			m_dblMin[1] = 0;
			m_dblMax[1] = 12;

			m_dblMin[2] = 50;
			m_dblMax[2] = 250;

		}
		break;
	}

	double dblDataStep[MAX_CHART_COUNT];

	dblDataStep[0] = (m_dblMax[0] - m_dblMin[0])/(m_yCount-1);
	dblDataStep[1] = (m_dblMax[1] - m_dblMin[1])/(m_yCount-1);
	dblDataStep[2] = (m_dblMax[2] - m_dblMin[2])/(m_yCount-1);


	for (int i = 0; i < m_yCount; i++)
	{
		StringCbPrintf((LPTSTR)m_szLabel1[i], MAX_LABEL_LENGTH, 
			_T("%3.2f"), (m_dblMax[0]-dblDataStep[0]*i));
	}

	if (nType != CHART_TYPE_SIMPLE)
	{
		for (int i = 0; i < m_yCount; i++)
		{
			StringCbPrintf((LPTSTR)m_szLabel2[i], MAX_LABEL_LENGTH, 
				_T("%3.2f"), (m_dblMax[1]-dblDataStep[1]*i));
		}

		for (int i = 0; i < m_yCount; i++)
		{
			StringCbPrintf((LPTSTR)m_szLabel3[i], MAX_LABEL_LENGTH, 
				_T("%3.2f"), (m_dblMax[2]-dblDataStep[2]*i));
		}
	}

	// NOTE: Elements GraphData::m_nSelectedItem[] are used 
	// in working thread to select needed flight data,
	// i.e. as an argument of SimData::GetFlightData()

	Lock lock(&g_csSimData);

	g_GraphData.SetSelectedChartValue(0, m_nSimVarID[0]);
	g_GraphData.SetSelectedChartValue(1, m_nSimVarID[1]);
	g_GraphData.SetSelectedChartValue(2, m_nSimVarID[2]);

	lock.Release();

}

void GDisplay::SaveOptions()
{
	CRegistry Reg(HKEY_CURRENT_USER);
	DWORD dw;

	Reg.OpenKey(APP_MAIN_KEY);
	dw = m_fManoeuverable;
	Reg.WriteInteger(_T("Manoeuverable"), &dw);
	Reg.CloseKey();

}

// Read user interface options from registry
void GDisplay::ReadOptions()
{
	CRegistry Reg(HKEY_CURRENT_USER);
	DWORD dw;

	Reg.OpenKey(APP_MAIN_KEY);
	if (Reg.ReadInteger(_T("Manoeuverable"), &dw))
		m_fManoeuverable = dw;

	Reg.CloseKey();
}

//////////////////////////////////////////////////////////////////////////

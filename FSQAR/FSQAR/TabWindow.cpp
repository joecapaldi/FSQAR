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
#include "resource.h"

#define CX_ICON  16 
#define CY_ICON  16 
#define NUM_ICONS 3 

TabWindow::TabWindow()
{
	m_hImageList = NULL;
}

TabWindow::~TabWindow()
{
	if (m_hImageList)
		ImageList_Destroy(m_hImageList);
}

void TabWindow::CreateImageList()
{
	m_hImageList = ImageList_Create(CX_ICON, CY_ICON, 
		ILC_MASK, NUM_ICONS, 0);

	ASSERT(m_hImageList != NULL);

	if (m_hImageList)
	{
		HICON hIcon = MyLoadIcon(MAKEINTRESOURCE(IDI_TAB1)); 
		if (hIcon)
		{
			ImageList_AddIcon(m_hImageList, hIcon); 
		}
		hIcon = NULL;

		hIcon = MyLoadIcon(MAKEINTRESOURCE(IDI_TAB2));
		if (hIcon)
		{
			ImageList_AddIcon(m_hImageList, hIcon); 
		}
		hIcon = NULL;

		hIcon = MyLoadIcon(MAKEINTRESOURCE(IDI_TAB3)); 
		if (hIcon)
		{
			ImageList_AddIcon(m_hImageList, hIcon); 
		}

		/*
		hIcon = MyLoadIcon(MAKEINTRESOURCE(IDI_TAB4)); 
		if (hIcon)
		{
			ImageList_AddIcon(m_hImageList, hIcon); 
		}
		*/
	}
}


BOOL TabWindow::InsertItem(__in_opt UINT nResID, 
						   __in_opt INT nImage)
{
	TCHAR szTitle[SHORT_STR_LEN];
	if (!MyLoadString(nResID, szTitle, SHORT_STR_LEN))
		return FALSE;

	TCITEM tie;
	tie.mask = TCIF_TEXT | TCIF_IMAGE; 
	tie.iImage = nImage; 
	tie.pszText = szTitle; 
	tie.cchTextMax = SHORT_STR_LEN;

	ASSERT(::IsWindow(m_hWnd));

	if (TabCtrl_InsertItem(m_hWnd, 0, &tie) == -1) 
	{ 
		DestroyWindow(); 
		return FALSE; 
	}

	return TRUE;
}

BOOL TabWindow::Create(__in const Window* pFrameWnd, 
					   __in LPCRECT lpRect)
{
	Window::Create(0, WC_TABCONTROL, NULL,
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE
		|TCS_EX_FLATSEPARATORS|TCS_BUTTONS|TCS_FLATBUTTONS, 
		lpRect, 
		pFrameWnd, 0, NULL); 
	Attach(m_hWnd);

	//InsertItem(IDS_TAB4_TITLE, 3);
	InsertItem(IDS_TAB3_TITLE, 2);
	InsertItem(IDS_TAB2_TITLE, 1);
	InsertItem(IDS_TAB1_TITLE, 0);
	
	TabCtrl_SetExtendedStyle(m_hWnd, TCS_BUTTONS|TCS_FLATBUTTONS);

	CreateImageList();
	if (m_hImageList)
		TabCtrl_SetImageList(m_hWnd, m_hImageList);

	TabCtrl_SetCurFocus(m_hWnd, 0);
	TabCtrl_SetCurSel(m_hWnd, 0);

	SetDefaultFont();
	return TRUE;
}

void TabWindow::SetDefaultFont()
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

void TabWindow::AdjustRect(__in_opt LPCRECT lpRect)
{
	TabCtrl_AdjustRect(m_hWnd, FALSE, lpRect);
}
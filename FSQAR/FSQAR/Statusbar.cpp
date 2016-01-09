
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


static const UINT nParts = SBPARTS_NUMBER;
static UINT aWidths[SBPARTS_NUMBER] =
{
	DEFAULT_SBPART1_WIDTH, 
	DEFAULT_SBPART2_WIDTH,
	DEFAULT_SBPART3_WIDTH
};

StatusBar::StatusBar()
{

}

BOOL StatusBar::Create(const Window* pFrameWnd)
{
	RECT rc;
	::SetRect(&rc, 10, 10, 10, 10);
	Window::Create(0, STATUSCLASSNAME, NULL, 
		WS_CHILD | WS_VISIBLE, 
		&rc, 
		pFrameWnd, (HMENU)ID_STATUSBAR, NULL);

	Attach(m_hWnd);
	SendMessage(SB_SETPARTS, (WPARAM)nParts, (LPARAM)(LPUINT)&aWidths);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

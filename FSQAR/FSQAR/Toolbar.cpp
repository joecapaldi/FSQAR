
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
//#include "rebar.h"
#include "resource.h"

//----------------------------------------------------------------- 
// Toolbar implementation
//----------------------------------------------------------------- 

Toolbar::Toolbar()
{ 
	m_hImgList = NULL;
} 

Toolbar::~Toolbar()
{ 
	TRACE0("DTOR Toolbar\n");

	if (m_hImgList)
	{
		ImageList_Destroy(m_hImgList);
	}
} 



#define MIN_COMBOCX			128
#define MIN_COMBOCY			32
#define NUM_LINKS			3


#define ID_STD_TOOL_BAR		2
#define NUM_IMAGES			14
#define NUM_BUTTONS 		14

// File commands
#define IDM_FILE_NEW                    40001
#define IDM_FILE_OPEN                   40002
#define IDM_FILE_SAVE                   40003
#define IDM_FILE_PRINT                  40006

// Edit commands
#define IDM_EDIT_CUT                    40018
#define IDM_EDIT_COPY                   40019
#define IDM_EDIT_PASTE                  40020
#define IDM_EDIT_DATETIME               40023

// Search command
#define IDM_SEARCH_FIND                 40025

// This definition fixes the size of array and important 
// to check if out of the array bounds
#define MAX_BUTTON_COUNT	14	
static const TBBUTTON tbStdBtns[MAX_BUTTON_COUNT] =
{
	{0, ID_SIM_CONTINUE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{1, ID_SIM_PAUSE,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{2, ID_SIM_QUIT,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{0, 0,					TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
	{3, ID_CONTROL_PAD,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{0, 0,					TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
	{4, ID_CHART_SIMPLE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{5, ID_CHART_LANDING,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{6, ID_CHART_ANGLES,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{7, ID_CHART_PITCH,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{8, ID_CHART_ROLL,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{9, ID_CHART_YAW,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{10, ID_CHART_THRUST,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	{11, ID_SCREENSHOOT,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
};

// TBSTYLE_TRANSPARENT - creates a transparent toolbar. In a transparent 
// toolbar, the toolbar is transparent but the buttons are not.
//static const DWORD dwStyle = WS_VISIBLE | WS_CHILD;

static HWND CreateStandartToolbar(HWND hWndParent)
{
	HWND hwnd;
	UINT uNumButtons;
	TBADDBITMAP tbBmp;
	UINT nImageNumber;
	DWORD dxButton, dyButton;
	RECT rcClient;


	// The toolbar window procedure automatically sets the size and position
	// of the toolbar window. The height is based on the height of the buttons
	// in the toolbar. The width is the same as the width of the parent window's
	// client area. To change the automatic size settings, send a
	// TB_SETBUTTONSIZE message.
	// BUT HERE:
	// The toolbar default sizing and positioning behaviors can be turned off
	// by setting the CCS_NORESIZE and CCS_NOPARENTALIGN common control styles.
	// Toolbar controls that are hosted by rebar controls must set these styles because
	// the rebar control sizes and positions the toolbar.
	// 

	GetClientRect(hWndParent, &rcClient);
	hwnd = CreateWindowEx(0, 
		TOOLBARCLASSNAME, 
		NULL, 
		WS_VISIBLE | WS_CHILD | TBSTYLE_TOOLTIPS, 
		rcClient.left, rcClient.top,
		rcClient.right - rcClient.left,
		TOOLBAR_HEIGHT, 
		hWndParent, 
		(HMENU) ID_TOOLBAR, AfxGetInstanceHandle(), NULL); 

	if (!hwnd)
	{
		// Error handler here...
		return NULL;
	}

	dxButton = dyButton = BUTTON_SIZE;

	::SendMessage(hwnd, TB_SETBUTTONSIZE, 0, (LPARAM)MAKELONG(dxButton, dyButton));

	// If the toolbar was created by using the CreateWindowEx function,
	// you must send the TB_BUTTONSTRUCTSIZE message to the toolbar before
	// sending TB_ADDBUTTONS and TB_ADDBITMAP.
	::SendMessage(hwnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);


	tbBmp.hInst = AfxGetInstanceHandle();
	tbBmp.nID = IDR_QAR_TOOL;//IDB_STD_SMALL;
	nImageNumber = NUM_IMAGES;

	::SendMessage(hwnd, TB_ADDBITMAP, (WPARAM)nImageNumber, (LPARAM) &tbBmp);

	uNumButtons = NUM_BUTTONS;
	// Check if out of the array bounds
	ASSERT((uNumButtons <= MAX_BUTTON_COUNT));
	::SendMessage(hwnd, TB_ADDBUTTONS, (WPARAM)uNumButtons, (LPARAM) &tbStdBtns);

	// you must send the TB_AUTOSIZE message after all the items and strings have been
	// inserted into the control to cause the toolbar to recalculate its size based on
	// its content.
	::SendMessage(hwnd, TB_AUTOSIZE, 0, 0);

	return hwnd;
}


BOOL Toolbar::Create(const Window* pParent)
{
	// Create the toolbar control to be added.
	HWND hWnd = CreateStandartToolbar(pParent->GetSafeHwnd());
	Attach(hWnd);
	return TRUE;
}
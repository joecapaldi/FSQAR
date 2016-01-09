
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


Window::Window()
{
	m_hWnd = NULL;
	m_hFont = NULL;
	m_hMenu = NULL;
}

Window::~Window()
{
	DeleteObject(m_hFont);
}


BOOL Window::CreateEx(DWORD dwExStyle,
					LPCTSTR lpClassName,
					LPCTSTR lpWindowName,
					DWORD dwStyle,
					int x,
					int y,
					int nWidth,
					int nHeight,
					HWND hWndParent,
					HMENU hMenu,
					Window* lpWndObject)
{

	// It is taken from MFC source and edited by me
	m_hWnd = ::CreateWindowEx(
		dwExStyle,
		lpClassName,
		lpWindowName,
		dwStyle,
		x,
		y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		AfxGetInstanceHandle(),
		lpWndObject
		);

	if (!m_hWnd)
		return FALSE;

	m_hMenu = ::GetMenu(m_hWnd);
	return TRUE;
}

BOOL Window::Create(DWORD dwExStyle,
				  LPCTSTR lpszClassName, 
				  LPCTSTR lpszWindowName, 
				  DWORD dwStyle, 
				  LPCRECT lpRect, 
				  const Window* pParentWnd,
				  HMENU hMenu,
				  const Window* lpWndObject)
{
	// can't use for desktop or pop-up windows (use CreateEx instead)
	ASSERT(pParentWnd != NULL);
	__analysis_assume(pParentWnd);

	
	// lpWndObject is out window object instance that to be passed to 
	// the window through the CREATESTRUCT structure.
	m_hWnd = CreateWindowEx(
		dwExStyle,
		lpszClassName,
		lpszWindowName,
		dwStyle | WS_CHILD,
		lpRect->left, 
		lpRect->top, 
		lpRect->right - lpRect->left, 
		lpRect->bottom - lpRect->top,
		pParentWnd->m_hWnd,
		hMenu,
		AfxGetInstanceHandle(),
		(LPVOID)lpWndObject);

	if (!m_hWnd)
		return FALSE;

	m_hMenu = ::GetMenu(m_hWnd);
	return TRUE;
}


LRESULT CALLBACK Window::DefaultWindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::DefWindowProc(m_hWnd, Msg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////
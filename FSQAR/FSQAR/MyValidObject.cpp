
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

//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
BOOL  AfxAssertFailedLine(LPCSTR lpszFileName, int nLine)
{

#ifdef _ALX_USE_CRT_ASSERT

	// we remove WM_QUIT because if it is in the queue then the message box
	// won't display
	MSG msg;
	BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
	BOOL bResult = _CrtDbgReport(_CRT_ASSERT, lpszFileName, nLine, NULL, NULL);
	if (bQuit)
		PostQuitMessage((int)msg.wParam);
	return bResult;

#else

	TCHAR szBuffer[MAX_PATH];
	if (SUCCEEDED(StringCbPrintf(szBuffer, MAX_PATH,
		_T("Assertion failed!!!\n%S (%d)\n"),
		lpszFileName, nLine)))
	{
		int nRes = MessageBox(0, szBuffer, 
			_T("ASSERTION FAILED"), MB_ABORTRETRYIGNORE|MB_ICONERROR|MB_APPLMODAL);

		if (nRes == IDABORT)
		{
			return TRUE;
		}
	}

	return FALSE;

#endif // _ALX_USE_CRT_ASSERT

}

#endif // _DEBUG

//////////////////////////////////////////////////////////////////////////

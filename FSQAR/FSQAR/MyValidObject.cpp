
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


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const struct CRuntimeClass CObject::classCObject =
{ "CObject", sizeof(CObject), NULL };

CObject::CObject()
{
}

CObject::~CObject()
{
}

CRuntimeClass* CObject::GetRuntimeClass() const
{
	return RUNTIME_CLASS(CObject);
}

#ifdef _DEBUG
void AFXAPI AfxAssertValidObject(const CObject* pOb, LPCSTR lpszFileName, int nLine)
{
	if (pOb == NULL)
	{
		TRACE0("ASSERT_VALID fails with NULL pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}
	if (!AfxIsValidAddress(pOb, sizeof(CObject)))
	{
		TRACE0("ASSERT_VALID fails with illegal pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}

	// check to make sure the VTable pointer is valid
	ASSERT(sizeof(CObject) == sizeof(void*));
	if (!AfxIsValidAddress(*(void**)pOb, sizeof(void*), FALSE))
	{
		TRACE0("ASSERT_VALID fails with illegal vtable pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}

	if (!AfxIsValidAddress(pOb, pOb->GetRuntimeClass()->m_nObjectSize, FALSE))
	{
		TRACE0("ASSERT_VALID fails with illegal pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}
	pOb->AssertValid();
}


void CObject::AssertValid() const
{
	ASSERT(this != NULL);
}
#endif //_DEBUG



//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
BOOL AFXAPI AfxAssertFailedLine(LPCSTR lpszFileName, int nLine)
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

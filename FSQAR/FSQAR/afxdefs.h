
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


#ifndef __AFXDEFS_H__
#define __AFXDEFS_H__

//========================================================================
//				We emulate MFC names for our GUI framework
//========================================================================

// This macro is used to reduce size requirements of some classes
#ifndef AFX_ALWAYS_VTABLE
#ifndef AFX_NOVTABLE
#if _MSC_VER >= 1100 && !defined(_DEBUG)
#define AFX_NOVTABLE __declspec(novtable)
#else
#define AFX_NOVTABLE
#endif
#endif
#endif


/////////////////////////////////////////////////////////////////////////////
// Special AfxDebugBreak: used to break into debugger at critical times

#ifndef AfxDebugBreak
#ifdef _AFX_NO_DEBUG_CRT
// by default, debug break is asm int 3, or a call to DebugBreak, or nothing
#if defined(_M_IX86) && !defined(_AFX_PORTABLE)
#define AfxDebugBreak() _asm { int 3 }
#else
#define AfxDebugBreak() DebugBreak()
#endif
#else
#define AfxDebugBreak() _CrtDbgBreak()
#endif
#endif

#ifndef _DEBUG
#ifdef AfxDebugBreak
#undef AfxDebugBreak
#endif
#define AfxDebugBreak()
#endif  // _DEBUG



/////////////////////////////////////////////////////////////////////////////
// Diagnostic support

#ifdef _DEBUG

BOOL  AfxAssertFailedLine(LPCSTR lpszFileName, int nLine);

void __cdecl AfxTrace(LPCTSTR lpszFormat, ...);



#define THIS_FILE          __FILE__
#define VERIFY(f)          ASSERT(f)
#define DEBUG_ONLY(f)      (f)

void __cdecl MyTrace(LPCTSTR lpszFormat, ...);

#define TRACE0(sz)              ::MyTrace(_T("%s"), _T(sz))
#define TRACE1(sz, p1)          ::MyTrace(_T(sz), p1)
#define TRACE2(sz, p1, p2)      ::MyTrace(_T(sz), p1, p2)
#define TRACE3(sz, p1, p2, p3)  ::MyTrace(_T(sz), p1, p2, p3)
#define TRACE4(sz, p1, p2, p3, p4)  ::MyTrace(_T(sz), p1, p2, p3, p4)


#else   // _DEBUG

#define VERIFY(f)          ((void)(f))
#define DEBUG_ONLY(f)      ((void)0)
#pragma warning(push)
#pragma warning(disable : 4793)
inline void MyTrace(LPCTSTR, ...) { }
#pragma warning(pop)
#define TRACE              __noop
#define TRACE0(sz)
#define TRACE1(sz, p1)
#define TRACE2(sz, p1, p2)
#define TRACE3(sz, p1, p2, p3)
#define TRACE4(sz, p1, p2, p3, p4)

#endif // !_DEBUG

#ifndef _ALX_USE_CRT_ASSERT

#define ASSERT(f)          DEBUG_ONLY((void) ((f) || \
		!::AfxAssertFailedLine(THIS_FILE, __LINE__) || (AfxDebugBreak(), 0))) \

#endif

#ifdef _ALX_USE_CRT_ASSERT
#define ASSERT(f)	_ASSERTE(f);
#endif


//////////////////////////////////////////////////////////////////////////
// Validation must be here

BOOL AfxIsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite = TRUE);
BOOL AfxIsValidString(LPCWSTR lpsz, UINT_PTR nLength = -1);
BOOL AfxIsValidString(LPCSTR lpsz, UINT_PTR nLength = -1);


#endif // __AFXDEFS_H__

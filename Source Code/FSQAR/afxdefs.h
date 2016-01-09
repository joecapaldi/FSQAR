
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

#ifndef AFXAPI
#define AFXAPI __stdcall
#endif

#ifndef AFX_CDECL
#define AFX_CDECL __cdecl
#endif


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


//////////////////////////////////////////////////////////////////////////
// Root object must be exists

struct CRuntimeClass
{
	LPCSTR m_lpszClassName;
	int m_nObjectSize;
	// CRuntimeClass objects linked together in simple list
	CRuntimeClass* m_pNextClass;       // linked list of registered classes
};

class AFX_NOVTABLE CObject
{
public:
	static const CRuntimeClass classCObject;
	virtual CRuntimeClass* GetRuntimeClass() const;
	virtual ~CObject() = 0;  // virtual destructor is necessary
	void AssertValid() const;

	// Disable the copy constructor and assignment by default so you will get
	// compiler errors instead of unexpected behavior if you pass objects
	// by value or assign objects.
protected:
	CObject();
private:
	CObject(const CObject& objectSrc);              // no implementation
	void operator=(const CObject& objectSrc);       // no implementation
};

#define RUNTIME_CLASS(class_name) ((CRuntimeClass*)(&class_name::class##class_name))

void AFXAPI AfxClassInit(CRuntimeClass* pNewClass);

/////////////////////////////////////////////////////////////////////////////
// Diagnostic support

#ifdef _DEBUG

BOOL AFXAPI AfxAssertFailedLine(LPCSTR lpszFileName, int nLine);

void AFX_CDECL AfxTrace(LPCTSTR lpszFormat, ...);
// Note: file names are still ANSI strings (filenames rarely need UNICODE)
void AFXAPI AfxAssertValidObject(const CObject* pOb, LPCSTR lpszFileName, 
								 int nLine);
void AFXAPI AfxDump(const CObject* pOb); // Dump an object from CodeView


#define THIS_FILE          __FILE__
#define VERIFY(f)          ASSERT(f)
#define DEBUG_ONLY(f)      (f)

void AFX_CDECL MyTrace(LPCTSTR lpszFormat, ...);

#define TRACE0(sz)              ::MyTrace(_T("%s"), _T(sz))
#define TRACE1(sz, p1)          ::MyTrace(_T(sz), p1)
#define TRACE2(sz, p1, p2)      ::MyTrace(_T(sz), p1, p2)
#define TRACE3(sz, p1, p2, p3)  ::MyTrace(_T(sz), p1, p2, p3)
#define TRACE4(sz, p1, p2, p3, p4)  ::MyTrace(_T(sz), p1, p2, p3, p4)

// These AFX_DUMP macros also provided for backward compatibility
#define AFX_DUMP0(dc, sz)   dc << _T(sz)
#define AFX_DUMP1(dc, sz, p1) dc << _T(sz) << p1

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


/* see ATL headers for commentary on this */
/* We use the name AFXASSUME to avoid name clashes */
#define AFXASSUME(cond)       do { bool __afx_condVal=!!(cond); \
	ASSERT(__afx_condVal); __analysis_assume(__afx_condVal); } while(0) 
#define ASSERT_VALID(pOb)  DEBUG_ONLY((::AfxAssertValidObject(pOb, THIS_FILE, __LINE__)))

// Debug ASSERTs then throws. Retail throws if condition not met
#define ENSURE_THROW(cond, exception)	\
	do { int __afx_condVal=!!(cond); ASSERT(__afx_condVal); \
	if (!(__afx_condVal)){exception;} } while (false)
#define ENSURE(cond)		ENSURE_THROW(cond, ::AfxThrowInvalidArgException() )
#define ENSURE_ARG(cond)	ENSURE_THROW(cond, ::AfxThrowInvalidArgException() )

// Debug ASSERT_VALIDs then throws. Retail throws if pOb is NULL
#define ENSURE_VALID_THROW(pOb, exception)	\
	do { ASSERT_VALID(pOb); if (!(pOb)){exception;} } while (false)
#define ENSURE_VALID(pOb)	ENSURE_VALID_THROW(pOb, ::AfxThrowInvalidArgException() )

#define ASSERT_POINTER(p, type) \
	ASSERT(((p) != NULL) && AfxIsValidAddress((p), sizeof(type), FALSE))

#define ASSERT_NULL_OR_POINTER(p, type) \
	ASSERT(((p) == NULL) || AfxIsValidAddress((p), sizeof(type), FALSE))

#ifdef _DEBUG
#define UNUSED(x)
#else
#define UNUSED(x) x
#endif
#define UNUSED_ALWAYS(x) x

#ifdef _DEBUG
#define REPORT_EXCEPTION(pException, szMsg) \
	do { \
	TCHAR szErrorMessage[512]; \
	if (pException->GetErrorMessage(szErrorMessage, sizeof(szErrorMessage)/sizeof(*szErrorMessage), 0)) \
	TRACE(traceAppMsg, 0, _T("%s (%s:%d)\n%s\n"), szMsg, __FILE__, __LINE__, szErrorMessage); \
		else \
		TRACE(traceAppMsg, 0, _T("%s (%s:%d)\n"), szMsg, __FILE__, __LINE__); \
		ASSERT(FALSE); \
	} while (0)
#else
#define REPORT_EXCEPTION(pException, szMsg) \
	do { \
	CString strMsg; \
	TCHAR  szErrorMessage[512]; \
	if (pException->GetErrorMessage(szErrorMessage, sizeof(szErrorMessage)/sizeof(*szErrorMessage), 0)) \
	strMsg.Format(_T("%s (%s:%d)\n%s"), szMsg, __FILE__, __LINE__, szErrorMessage); \
		else \
		strMsg.Format(_T("%s (%s:%d)"), szMsg, __FILE__, __LINE__); \
		AfxMessageBox(strMsg); \
	} while (0)
#endif

#define EXCEPTION_IN_DTOR(pException) \
	do { \
	REPORT_EXCEPTION((pException), "Exception thrown in destructor"); \
	delete pException; \
	} while (0)

#define AFX_BEGIN_DESTRUCTOR try {
#define AFX_END_DESTRUCTOR   } catch (CException *pException) { EXCEPTION_IN_DTOR(pException); }

//////////////////////////////////////////////////////////////////////////
// Validation must be here

BOOL AFXAPI AfxIsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite = TRUE);
BOOL AFXAPI AfxIsValidString(LPCWSTR lpsz, UINT_PTR nLength = -1);
BOOL AFXAPI AfxIsValidString(LPCSTR lpsz, UINT_PTR nLength = -1);



#endif // __AFXDEFS_H__

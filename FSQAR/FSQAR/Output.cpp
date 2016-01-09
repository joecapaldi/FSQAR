
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
#include "SimClient.h"


//////////////////////////////////////////////////////////////////////////
// THIS ARE CORRECT FUNCTIONS THAT DOES'T PRODUCE ANALYZE WARNINGS
// -----------------------------------------------------------------------
#pragma warning(suppress: 6103)
intptr_t _wcstombsz(__out_bcount(count) char* mbstr, 
						  __in const wchar_t* wcstr, 
						  __in size_t count)
{
	// count is number of bytes
	if (count == 0 && mbstr != NULL)
		return 0;

	intptr_t result = (intptr_t)::WideCharToMultiByte(CP_ACP, 0, wcstr, -1,
		mbstr, (int)count, NULL, NULL);
	ASSERT(mbstr == NULL || result <= (intptr_t)count);
	return result;
}

#pragma warning(suppress: 6103)
intptr_t _mbstowcsz(__out_ecount_z(count) wchar_t* wcstr, 
						  __in const char* mbstr, 
						  __in size_t count)
{
	// count is number of wchar_t's
	if (count == 0 && wcstr != NULL)
		return 0;

	intptr_t result = (intptr_t)::MultiByteToWideChar(CP_ACP, 0, mbstr, -1,
		wcstr, (int)count);
	ASSERT(wcstr == NULL || result <= (intptr_t)count);
	if ((result > 0) && (wcstr != NULL) && ((result-1) < (intptr_t)count*2))
	{
		wcstr[result-1] = 0;
	}

	return result;
}



COutput::COutput()
{
}

COutput::~COutput()
{
}

COutput& COutput::operator<<(__in_z LPCTSTR lpsz)
{
	TCHAR szBuffer[512];
	LPTSTR lpBuf = szBuffer;
	
	ASSERT(AfxIsValidString(lpsz));

	while (*lpsz != '\0')
	{
		if (lpBuf > szBuffer + _countof(szBuffer) - 3)
		{
			*lpBuf = '\0';
			OutputString(szBuffer);
			lpBuf = szBuffer;
		}
		
		if (*lpsz == '\n')
			*lpBuf++ = '\r';
			
		*lpBuf++ = *lpsz++;
	}
	
	*lpBuf = '\0';
	OutputString(szBuffer);
	return *this;
}


#ifdef _UNICODE
// special version for ANSI characters
COutput& COutput::operator<<(__in_z LPCSTR lpsz)
{
	ASSERT(AfxIsValidString(lpsz));
	// limited length
	TCHAR szBuffer[512];
	_mbstowcsz(szBuffer, lpsz, _countof(szBuffer));
	szBuffer[511] = 0;
	return *this << szBuffer;
}
#else   //_UNICODE
// special version for WIDE characters
COutput& COutput::operator<<(__in_z LPCWSTR lpsz)
{
	// limited length
	char szBuffer[LONG_STR_LEN];
	_wcstombsz(szBuffer, lpsz, _countof(szBuffer));
	szBuffer[LONG_STR_LEN-1] = 0;
	return *this << szBuffer;
}
#endif  //!_UNICODE

#ifdef _UNICODE
COutput& COutput::operator<<(char ch)
{
	TCHAR szBuffer[2];

	wsprintf(szBuffer, _T("%c"), ch);
	OutputString(szBuffer);

	return *this;
}
#else   //_UNICODE
COutput& COutput::operator<<(wchar_t wc)
{
	TCHAR szBuffer[2];

	wsprintf(szBuffer, _T("%C"), wc);
	OutputString(szBuffer);

	return *this;
}
#endif  //!_UNICODE

COutput& COutput::operator<<(int n)
{
	TCHAR szBuffer[SHORT_STR_LEN];
	::StringCbPrintf(szBuffer, SHORT_STR_LEN, _T("%d"), n);
	OutputString(szBuffer);
	return *this;
}

COutput& COutput::operator<<(float f)
{
	TCHAR szBuffer[SHORT_STR_LEN];
	::StringCbPrintf(szBuffer, SHORT_STR_LEN, _T("%5.3f"), f);
	*this << szBuffer;
	return *this;
}

COutput& COutput::operator<<(double d)
{
	TCHAR szBuffer[SHORT_STR_LEN];
	::StringCbPrintf(szBuffer, SHORT_STR_LEN, _T("%5.3f"), d);
	*this << szBuffer;
	return *this;
}


/////////////////////////////////////////////////////////////////////////////


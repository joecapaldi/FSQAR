
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


void MyOutputDebugString(LPCTSTR lpsz)
{
	TCHAR szBuffer[512];
	LPTSTR lpBuf = szBuffer;

	if (lpsz == NULL)
	{
		OutputDebugString(_T("(NULL)"));
		return;
	}

	while (*lpsz != '\0')
	{
		if (lpBuf > szBuffer + _countof(szBuffer) - 3)
		{
			*lpBuf = '\0';
			OutputDebugString(szBuffer);
			lpBuf = szBuffer;
		}
		if (*lpsz == '\n')
			*lpBuf++ = '\r';
		*lpBuf++ = * lpsz++;
	}
	*lpBuf = '\0';
	OutputDebugString(szBuffer);
	return;

}

#ifdef _DEBUG
void MyTrace(LPCTSTR lpszFormat,...)
{
	TCHAR szBuffer[512];	// it may be long buffer

	va_list args;
	va_start(args, lpszFormat);

	StringCbVPrintf(szBuffer, _countof(szBuffer), lpszFormat, args);

	MyOutputDebugString((LPCTSTR)szBuffer);

	va_end(args);
}
#endif


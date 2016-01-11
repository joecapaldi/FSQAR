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




// AfxIsValidString() returns TRUE if the passed pointer
// references a string of at least the given length in characters.
// A length of -1 (the default parameter) means that the string
// buffer's minimum length isn't known, and the function will
// return TRUE no matter how long the string is. The memory
// used by the string can be read-only.

BOOL AfxIsValidString(LPCWSTR lpsz, UINT_PTR nLength /* = -1 */)
{
	if (lpsz == NULL)
		return FALSE;
	return ::IsBadStringPtrW(lpsz, nLength) == 0;
}

// As above, but for ANSI strings.

BOOL AfxIsValidString(LPCSTR lpsz, UINT_PTR nLength /* = -1 */)
{
	if (lpsz == NULL)
		return FALSE;
	return ::IsBadStringPtrA(lpsz, nLength) == 0;
}

// AfxIsValidAddress() returns TRUE if the passed parameter points
// to at least nBytes of accessible memory. If bReadWrite is TRUE,
// the memory must be writeable; if bReadWrite is FALSE, the memory
// may be const.

BOOL AfxIsValidAddress(const void* lp, UINT nBytes,
	BOOL bReadWrite /* = TRUE */)
{
	// simple version using Win-32 APIs for pointer validation.
	return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
		(!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}

/////////////////////////////////////////////////////////////////////////////

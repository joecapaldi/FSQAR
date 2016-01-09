
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


#include "StdAfx.h"
#include "fsqar.h"


//----------------------------------------------------------------------------
// Returns the current length in bytes of szText excluding 
// the null terminator. This out parameter is equivalent to 
// the return value of strlen(szText) * sizeof(TCHAR)
//----------------------------------------------------------------------------
size_t MyStrLength(LPCTSTR pString, size_t cbMax)
{
	size_t cbCount = 0;
	if (FAILED(StringCbLength(pString, cbMax, &cbCount)))
	{
		ASSERT(FALSE);
		return 0;
	}
	return cbCount;
}

LPVOID _AllocateMemory(SIZE_T nSize, LPCTSTR pszFile, UINT nLine)
{
	LPVOID lpPtr =::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, nSize);

	if (lpPtr == NULL)
	{
#ifdef _DEBUG

		TRACE3("_AllocateMemory failed. Code: 0x%X, %s (%d)\n", 
			GetLastError(), pszFile, nLine);

#else

		UNREFERENCED_PARAMETER(pszFile);
		UNREFERENCED_PARAMETER(nLine);

#endif // _DEBUG

		return NULL;
	}
	else
		return lpPtr;
}

BOOL _FreeMemory(LPVOID lpAddress)
{
	return ::HeapFree(::GetProcessHeap(), 0, lpAddress);
}

wchar_t* _newstringW(SIZE_T nSize, LPCTSTR pszFile, UINT nLine)
{
	return static_cast<wchar_t*>(::_AllocateMemory(nSize, pszFile, nLine));

}


void _freestringW(wchar_t* string)
{
	_FreeMemory(string);
}

char* _newstringA(SIZE_T nSize, LPCTSTR pszFile, UINT nLine)
{
	return static_cast<char*>(::_AllocateMemory(nSize, pszFile, nLine));
}


void _freestringA(char* string)
{
	_FreeMemory(string);
}


BOOL IsFolderExists(LPCTSTR pszFolderName)
{
	DWORD dwAttrib = GetFileAttributes(pszFolderName);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

DWORD GetThisProcessID(LPCTSTR pszExeName)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		TRACE0("CreateToolhelp32Snapshot failed.\n");
		return 0;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		TRACE0("Process32First failed.\n");
		CloseHandle(hProcessSnap);
		return 0;
	}

	// Walk the snapshot of processes
	do
	{
		if (_tcscmp(pe32.szExeFile, pszExeName) == 0)
		{
			return pe32.th32ProcessID;
		}
	}
	while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return 0;
}

// ===========================================================
// Print a list of all threads for the given process to debug
// output.
//
// dwOwnerPID is thew process ID, that may be current process
// or any other process in the system
//
// tlhelp32.h should be included to use this fucntion
//
// ===========================================================
BOOL ListProcessThreads(DWORD dwOwnerPID)
{
	THREADENTRY32 te32;
	int i = 0;
	// Take a snapshot of all running threads
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return(FALSE);

	te32.dwSize = sizeof(THREADENTRY32);

	if (!Thread32First(hThreadSnap, &te32))
	{
		CloseHandle(hThreadSnap);
		return(FALSE);
	}

	do
	{
		if (te32.th32OwnerProcessID == dwOwnerPID)
		{
			i++;
			// TODO: Put here another action with the list of threads...
			TRACE4("  (%d) ThreadID: %#x (%d),\tOwnerProcessID: %x\n", i,
				te32.th32ThreadID, te32.th32ThreadID, te32.th32OwnerProcessID);
		}
	}
	while (Thread32Next(hThreadSnap, &te32));

	//  Don't forget to clean up the snapshot object.
	CloseHandle(hThreadSnap);
	return(TRUE);
}


HICON MyLoadIcon(LPCTSTR lpIconName)
{
	return ::LoadIcon(AfxGetInstanceHandle(), lpIconName);
}


int MyLoadString(UINT uID, LPTSTR lpBuffer, int cchBufferMax)
{
	return ::LoadString(AfxGetInstanceHandle(), uID, lpBuffer, cchBufferMax);
}


static BOOL fFontFound = FALSE;

int CALLBACK EnumFontFamProc(ENUMLOGFONTEX * lpelf, NEWTEXTMETRICEX * lpntm, 
							 int FontType, LPARAM lParam)
{
	LOGFONT * lf;

	UNREFERENCED_PARAMETER(lpntm);
	UNREFERENCED_PARAMETER(FontType);

	lf = & (lpelf)->elfLogFont;

	//if (lstrcmp(TEXT("Consolas"), (LPTSTR)lf->lfFaceName) == 0)
	if (lstrcmp((LPCTSTR)lParam, (LPTSTR)lf->lfFaceName) == 0)
	{
		fFontFound = TRUE;
		return 0;
	}
	else
		return 1;
}


BOOL SearchFont(LPCTSTR pszFontName)
{
	HDC hdc;
	LOGFONT lf;

	hdc = GetDC(NULL);
	if (hdc != NULL)
	{
		ZeroMemory(& lf, sizeof(LOGFONT));

		lf.lfCharSet = DEFAULT_CHARSET;

		EnumFontFamiliesEx(hdc, & lf, 
			(FONTENUMPROC)EnumFontFamProc, (LPARAM)pszFontName, 0);
		ReleaseDC(NULL, hdc);
	}
	else
		return FALSE;

	return fFontFound;
}


LPTSTR GetFileExt(LPTSTR pFile)
{
	intptr_t i;

	for (i = (intptr_t)lstrlen(pFile) - 1; i >= 0; --i)
	{
		if (pFile[i] == '.') 
			return (pFile + i + 1);
		else 
			if (pFile[i] == '\\') 
				break;
	}

	return _T("");
}


LPCTSTR GetFileName(LPCTSTR pFile)
{
	size_t cbSize;

	if (SUCCEEDED(StringCbLength(pFile, MAX_PATH, &cbSize)))
	{
		for (intptr_t i = cbSize - 1; i >= 0; --i)
		{
			if (pFile[i] == '\\')
				return (pFile + i + 1);
		}
	}

	return pFile;
}

__checkReturn
INT_PTR GetFileDir(__in_z LPCTSTR pFile, __out_z LPTSTR szFileDir, 
				   __in_opt intptr_t nFileDirLen)
{
	size_t cbSize;

	if (!nFileDirLen)
	{
		szFileDir[0] = '\0';
		return 0;
	}

	szFileDir[0] = '\0';
	if (SUCCEEDED(StringCbLength(pFile, MAX_PATH, &cbSize)))
	{
		size_t nCount;
		for (nCount = cbSize - 1; nCount > 0; --nCount)
		{
			if (pFile[nCount] == '\\')
			{
				nCount = min(nFileDirLen, (intptr_t)nCount + 1);
				if (SUCCEEDED(StringCchCopy(szFileDir, nCount, pFile)))
					return (nCount - 1);
			}
		}
	}
	return 0;
}



BOOL CenterWindow(HWND hwndChild, HWND hwndParent)
{
	RECT rcChild, rcParent;
	int cxChild, cyChild, cxParent, cyParent;
	int cxScreen, cyScreen, xNew, yNew;
	HDC hdc;

	// Get the Height and Width of the child window
	GetWindowRect(hwndChild, & rcChild);
	cxChild = rcChild.right - rcChild.left;
	cyChild = rcChild.bottom - rcChild.top;

	// Get the Height and Width of the parent window
	GetWindowRect(hwndParent, & rcParent);
	cxParent = rcParent.right - rcParent.left;
	cyParent = rcParent.bottom - rcParent.top;

	// Get the display limits
	hdc = GetDC(hwndChild);
	cxScreen = GetDeviceCaps(hdc, HORZRES);
	cyScreen = GetDeviceCaps(hdc, VERTRES);
	ReleaseDC(hwndChild, hdc);

	// Calculate new X position, then adjust for screen
	xNew = rcParent.left + ((cxParent - cxChild) / 2);
	if (xNew < 0)
	{
		xNew = 0;
	}
	else if ((xNew + cxChild) > cxScreen)
	{
		xNew = cxScreen - cxChild;
	}

	// Calculate new Y position, then adjust for screen
	yNew = rcParent.top + ((cyParent - cyChild) / 2);
	if (yNew < 0)
	{
		yNew = 0;
	}
	else if ((yNew + cyChild) > cyScreen)
	{
		yNew = cyScreen - cyChild;
	}

	// Set it, and return
	return SetWindowPos(hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

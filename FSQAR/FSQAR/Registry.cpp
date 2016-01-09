
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
#include "Registry.h"

CRegistry::CRegistry() : 
	m_hRoot(NULL), 
	m_hCurrentKey(NULL)
{}


CRegistry::CRegistry(HKEY hRootKey)
{
	m_hRoot = hRootKey;
	m_hCurrentKey;
}

CRegistry::~CRegistry()
{

}

BOOL CRegistry::IsKeyExist(LPCTSTR pszKeyName)
{

	ASSERT(m_hRoot);

	if (::RegOpenKeyEx(m_hRoot, pszKeyName, 0, 
		KEY_ALL_ACCESS, &m_hCurrentKey) != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL CRegistry::CreateKey(LPCTSTR pszKeyName)
{

	DWORD dwDisposition;

	ASSERT(m_hRoot);

	if(::RegCreateKeyEx(m_hRoot, pszKeyName, 0, NULL, 
		REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, 
		&m_hCurrentKey, &dwDisposition) != ERROR_SUCCESS)
	{
		TRACE1("CRegistry::CreateKey failed: %d\n", 
			GetLastError());
		return FALSE;
	}
	return TRUE;
}


HKEY CRegistry::OpenKey(LPCTSTR pszKeyName)
{
	ASSERT(m_hRoot);

	if (::RegOpenKeyEx(m_hRoot, pszKeyName, 0, KEY_ALL_ACCESS, 
		&m_hCurrentKey) != ERROR_SUCCESS)
	{
		TRACE1("CRegistry::OpenKey failed: %d\n", 
			GetLastError());
		return NULL;
	}
	return m_hCurrentKey;
}

void CRegistry::CloseKey()
{
	ASSERT(m_hRoot);
	ASSERT(m_hCurrentKey);
	::RegCloseKey(m_hCurrentKey);
}

void CRegistry::CloseKey(HKEY hKey)
{
	ASSERT(m_hRoot);
	ASSERT(hKey);
	::RegCloseKey(hKey);
}

BOOL CRegistry::WriteInteger(LPCTSTR pszValueName, LPDWORD lpdwValue)
{
	ASSERT(m_hRoot);
	ASSERT(m_hCurrentKey);

	if (::RegSetValueEx(m_hCurrentKey, pszValueName, 0, REG_DWORD, 
		(CONST BYTE *)lpdwValue, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		TRACE1("CRegistry::WriteInteger failed:  %d\n", 
			GetLastError());
		return FALSE;
	}
	return TRUE;
}

BOOL CRegistry::ReadInteger(LPCTSTR pszValueName, LPDWORD lpdwResult)
{
	DWORD dwType;
	LPBYTE lpbData;
	SIZE_T cbData;

	ASSERT(m_hRoot);
	ASSERT(m_hCurrentKey);

	if (::RegQueryValueEx(m_hCurrentKey, pszValueName, 0, 
		&dwType, NULL, &cbData) != ERROR_SUCCESS)
	{
		TRACE1("CRegistry::ReadInteger failed: %d\n", 
			GetLastError());
		return FALSE;
	}

	lpbData = (LPBYTE)::GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, cbData);
	if (!lpbData)
	{
		TRACE1("CRegistry::ReadInteger  failed: %d\n", 
			GetLastError());
		return FALSE;
	}

	if (::RegQueryValueEx(m_hCurrentKey, pszValueName, 0, 
		&dwType, lpbData, &cbData) != ERROR_SUCCESS)
	{
		TRACE1("CRegistry::ReadInteger  failed: %d\n", 
			GetLastError());
		return FALSE;
	}

	if (dwType == REG_DWORD)
		CopyMemory(lpdwResult, (LPDWORD)lpbData, sizeof(lpbData));

	GlobalFree((HGLOBAL)lpbData);

	return TRUE;
}


BOOL CRegistry::WriteString(LPCTSTR pszValueName, LPCTSTR pszString)
{
	size_t cbData;

	ASSERT(m_hRoot);
	ASSERT(m_hCurrentKey);

	if (SUCCEEDED(StringCbLength(pszString, MAX_PATH, &cbData)))
	{
		if (::RegSetValueEx(m_hCurrentKey, (LPTSTR)pszValueName, 0, REG_SZ,
			(CONST BYTE *) pszString, (DWORD)cbData) != ERROR_SUCCESS)
		{
			TRACE1("CRegistry::WriteString failed: %d\n",
				GetLastError());
			return FALSE;
		}
	}




	return TRUE;
}

BOOL CRegistry::WriteString(LPCTSTR pszValueName, LPCTSTR pszString, SIZE_T nLength)
{

	ASSERT(m_hRoot);
	ASSERT(m_hCurrentKey);

	if (::RegSetValueEx(m_hCurrentKey, (LPTSTR)pszValueName, 0, REG_SZ, 
		(CONST BYTE *) pszString, (DWORD)nLength) != ERROR_SUCCESS)
	{
		TRACE1("CRegistry::WriteString failed: %d\n", 
			GetLastError());
		return FALSE;
	}


	return TRUE;
}

BOOL CRegistry::ReadString(LPCTSTR pszValueName, LPTSTR pszResult)
{

	DWORD dwType;
	LPBYTE lpbData;
	SIZE_T cbData;

	ASSERT(m_hRoot);
	ASSERT(m_hCurrentKey);


	if (::RegQueryValueEx(m_hCurrentKey, pszValueName, 0, 
		&dwType, NULL, &cbData)!= ERROR_SUCCESS)
	{
		//
		return FALSE;
	}

	lpbData = (LPBYTE)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, cbData);
	if (!lpbData)
	{
		//
		return FALSE;
	}

	if (::RegQueryValueEx(m_hCurrentKey, pszValueName, 0, 
		&dwType, lpbData, &cbData)!= ERROR_SUCCESS)
	{
		//
		return FALSE;
	}

	if (dwType == REG_SZ)
	{
		StringCbCopy(pszResult, MAX_PATH, (LPTSTR)lpbData);
	}

	GlobalFree((HGLOBAL)lpbData);

	return TRUE;
}

BOOL CRegistry::WriteBinary(LPCTSTR pszValueName,  const BYTE* lpBuffer, SIZE_T nSize)
{
	ASSERT(m_hRoot);
	ASSERT(m_hCurrentKey);

	if (::RegSetValueEx(m_hCurrentKey, (LPTSTR)pszValueName, 0, REG_BINARY, 
		lpBuffer, (DWORD)nSize) != ERROR_SUCCESS)
	{
		TRACE1("CRegistry::WriteBinary failed: %d\n", 
			GetLastError());
		return FALSE;
	}


	return TRUE;
}

BOOL CRegistry::ReadBinary(__in_z LPCTSTR pszValueName, 
						   __out_bcount_full(nSize) LPBYTE lpBuffer, 
						   __in SIZE_T nSize)
{

	DWORD dwType;
	LPBYTE lpbData;
	SIZE_T cbData;

	ASSERT(m_hRoot);
	ASSERT(m_hCurrentKey);


	if (::RegQueryValueEx(m_hCurrentKey, pszValueName, 0, 
		&dwType, NULL, &cbData)!= ERROR_SUCCESS)
	{
		//
		return FALSE;
	}

	lpbData = (LPBYTE)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, cbData);
	if (!lpbData)
	{
		//
		return FALSE;
	}

	if (::RegQueryValueEx(m_hCurrentKey, pszValueName, 0, 
		&dwType, lpbData, &cbData)!= ERROR_SUCCESS)
	{
		//
		return FALSE;
	}

	if (dwType == REG_BINARY)
	{
		CopyMemory(lpBuffer, lpbData, nSize);
	}

	GlobalFree((HGLOBAL)lpbData);

	return TRUE;
}


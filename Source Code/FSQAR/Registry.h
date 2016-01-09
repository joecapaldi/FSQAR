
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


#ifndef __REGISTRY_H__
#define __REGISTRY_H__


class CRegistry
{
public:
	CRegistry();
	CRegistry(HKEY hRootKey);
	~CRegistry();

	HKEY GetRootKey() { return m_hRoot; }
	void SetRootKey(HKEY hKey) { m_hRoot = hKey; }

	BOOL IsKeyExist(LPCTSTR pszKeyName);
	BOOL CreateKey(LPCTSTR pszKeyName);
	HKEY OpenKey(LPCTSTR pszKeyName);
	void CloseKey();
	void CloseKey(HKEY hKey);

	// Integer
	BOOL WriteInteger(LPCTSTR pszValueName, LPDWORD lpdwValue);
	BOOL ReadInteger(LPCTSTR pszValueName, LPDWORD lpdwResult);

	// String
	BOOL WriteString(LPCTSTR pszValueName, LPCTSTR pszString);
	BOOL WriteString(LPCTSTR pszValueName, LPCTSTR pszString, SIZE_T nLength);
	BOOL ReadString(LPCTSTR pszValueName, LPTSTR pszResult);

	// Binary
	BOOL WriteBinary(LPCTSTR pszValueName, const BYTE* lpBuffer, SIZE_T nSize);
	BOOL ReadBinary(__in_z LPCTSTR pszValueName,
		__out_bcount_full(nSize) LPBYTE lpBuffer,
		__in SIZE_T nSize);

private:
	HKEY m_hRoot;
	HKEY m_hCurrentKey;

protected:
	DISABLE_COPY_AND_ASSIGN(CRegistry);

};

#endif // __REGISTRY_H__

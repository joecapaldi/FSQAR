
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
#include "simclient.h"
#include "resource.h"


CFile::CFile()
{
	m_hFile = INVALID_HANDLE_VALUE;
}

CFile::~CFile()
{

}

// open for write only !!!!!!!!!!!!!!!!!!!!!!!!!
// if bFileExist == TRUE open an existing file and returns
// an error if the file doesn't exist. 
// if bFileExist == FALSE open the file in any case.
// Create the file if it doesn't exist.
//
BOOL CFile::Open(LPCTSTR pszFileName, BOOL bFileExist)
{
	ASSERT(AfxIsValidString(pszFileName));

	m_hFile = ::CreateFile(pszFileName, 
		GENERIC_WRITE, 
		FILE_SHARE_READ,
		NULL, 
		bFileExist ? OPEN_EXISTING : OPEN_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);

	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		// error handler here
		TRACE1("CODE: %d\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL CFile::Write(LPCVOID lpBuffer, size_t dwBufferSize)
{
	DWORD dwBytesWritten = 0;

	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	::WriteFile(m_hFile, lpBuffer, (DWORD)dwBufferSize, &dwBytesWritten, NULL);
	if (dwBytesWritten != dwBufferSize)
	{
		//DWORD dwErrCode = ::GetLastError();
		return FALSE;
	}
	return TRUE;
}

BOOL CFile::Read(LPVOID lpBuffer, DWORD dwBufferSize)
{
	DWORD dwBytesRead = 0;

	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (::ReadFile(m_hFile, lpBuffer, dwBufferSize, &dwBytesRead, NULL))
	{
		if (dwBytesRead != dwBufferSize)
			return FALSE;
	}

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
// Implementation of CFileOutput::PrintFile

void CFileOutput::PrintFile(const SimData* pSimData, SIZE_T nDataSize)
{
	ASSERT(pSimData);
	__analysis_assume(pSimData);

	if (m_fFileOpened)
	{
		// It is necessary a reference on the object 
		// for using of stream operation <<
		CFileOutput& f = *this;
		for (uintptr_t i = 0; i < nDataSize; i++)
		{
			f << pSimData->GetVariable(i)->Value;
			if (i < nDataSize - 1) f << m_chSeparator;

			// End of line
			if (i == nDataSize - 1) f << "\n"; // ANCII \r\n 0x10 0x13

		} // for (... print ...)
	}
}


//////////////////////////////////////////////////////////////////////////
// Implementation of CFileOutput::PrintHeader
// 
void CFileOutput::PrintHeader(const SimData* pSimData, SIZE_T nDataSize)
{
	ASSERT(pSimData);
	__analysis_assume(pSimData);

	if (m_fFileOpened)
	{
		// It is necessary a reference on the object 
		// for using of stream operation <<
		CFileOutput& f = *this;

		for (uintptr_t i = 0; i < nDataSize; i++)
		{

			f << pSimData->GetVariable(i)->DatumName;
			if (i < nDataSize - 1) f << m_chSeparator;
		}
		
		// End of line
		f << "\n";
	}
}


BOOL CFileOutput::OpenFile(LPCTSTR pszFileName)
{
	if (!m_fFileOpened)
	{
		// Open output file
		if (Open(pszFileName, FALSE))
		{			
			m_fFileOpened = TRUE;
			TRACE1("Open file: %s\n", pszFileName);
			return TRUE;
		}
		else
		{
			TRACE1("Cannot open file: %s\n", pszFileName);
			// Top level function will process this error
			return FALSE;
		}
	}
	return TRUE;
}


void CFileOutput::SetSeparator(INT nSeparator)
{
	m_nDataSeparator = nSeparator;
	switch (nSeparator)
	{
	case DATAFILE_SEPARATOR_COMMA:
		m_chSeparator = ','; // 0x44;
		break;

	case DATAFILE_SEPARATOR_SEMICOLON:
		m_chSeparator = ';'; // 0x59;
		break;

	case DATAFILE_SEPARATOR_SPACE:
		m_chSeparator = ' '; // 0x32;
		break;

	case DATAFILE_SEPARATOR_TAB:
		m_chSeparator = '\t'; // 0x09;
		break;

	default:
		m_chSeparator = ';'; // 0x59;
		break;
	}


}
//////////////////////////////////////////////////////////////////////////
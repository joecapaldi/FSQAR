
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

#ifdef _ALX_USE_LITEZIP
#include "..\litezip\litezip.h"
#endif

#ifdef _ALX_USE_ZIPLIB
#include "..\ziplib\zlib.h"
#endif

#include "zipimpl.h"
#include "resource.h"

// Debug version of CRT library uses this operator
#ifdef _DEBUG
#ifndef DEBUG_NEW
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new DEBUG_NEW
#endif
#endif  // _DEBUG

CSimConnectOutput::CSimConnectOutput()
{
	m_fDefaultFileName = FALSE;
	m_fZipOutputFile = TRUE;
	m_nRecordsCount = 0;
	m_pFileOutput = new CFileOutput();
	m_pszOutputFolder = _newstring(MAX_PATH);
	m_pszCurrentFileName = _newstring(SHORT_STR_LEN);
}

CSimConnectOutput::~CSimConnectOutput()
{
	delete m_pFileOutput;
	_freestring(m_pszOutputFolder);
	_freestring(m_pszCurrentFileName);
}



void CSimConnectOutput::MakeFullPath(LPTSTR pOutputFileName,  size_t nSize, LPCTSTR pAircraftName)
{
	ASSERT(AfxIsValidString(pOutputFileName, nSize));

	if (!m_fDefaultFileName)
	{
		// Without extension !!!
		SYSTEMTIME st;
		GetLocalTime(&st);
		StringCbPrintf(m_pszCurrentFileName, SHORT_STR_LEN, 
			_T("%u_(%u_%u) %s"),
			st.wDay, st.wHour, st.wMinute, pAircraftName);
	}

	// Add file extension (*.csv) here
	TRACE1("Current file name: %s\n", m_pszCurrentFileName);
	StringCbPrintf(pOutputFileName, nSize, _T("%s\\%s.csv"), 
		m_pszOutputFolder, m_pszCurrentFileName);
	TRACE1("Full path: %s\n", pOutputFileName);
}



BOOL CSimConnectOutput::DoOpenFile(const char* pAircraftName)
{
	TCHAR szFullPath[MAX_PATH] = {'\0'};
	TCHAR szAircraftName[SHORT_STR_LEN];

#ifndef _UNICODE
	StringCbCopy(szAircraftName, SHORT_STR_LEN, pAircraftName);
#else
	_mbstowcsz(szAircraftName, pAircraftName, SHORT_STR_LEN);
#endif

	MakeFullPath(szFullPath, MAX_PATH, szAircraftName);
	return m_pFileOutput->OpenFile(szFullPath);
}

void CSimConnectOutput::CreateOutputFile(const char* pszFileName,
										 const SimData* pSimData)
{
	if (m_pFileOutput->IsFileOpened())
	{
		// FIXME: Invalid Handle error
		m_pFileOutput->Close();
		ZipOutputFile();

		// Open new file
		if (DoOpenFile(pszFileName))
			m_pFileOutput->PrintHeader(pSimData, pSimData->GetArraySize());
	}
	else
	{
		// Open new file
		if (DoOpenFile(pszFileName))
			m_pFileOutput->PrintHeader(pSimData, pSimData->GetArraySize());
	}
}

void CSimConnectOutput::PrintData(const SimData* pSimData)
{
	if (m_pFileOutput->IsFileOpened())
	{
		m_pFileOutput->PrintFile(pSimData, pSimData->GetArraySize());
		m_nRecordsCount++;
	}
}

void CSimConnectOutput::CloseFile()
{
	if (m_pFileOutput->IsFileOpened())
		m_pFileOutput->Close();

	m_nRecordsCount = 0;
	ZipOutputFile();
}


BOOL CSimConnectOutput::CreateOutputFolder(LPCTSTR pszSimFolder)
{
	LPTSTR pszMonth = _T("Jan");
	SYSTEMTIME st;
	GetLocalTime(&st);
	TCHAR szMyDocuments[MAX_PATH];

	// Get 'My Documents' folder location for current user
	if (FAILED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
		NULL, 0, szMyDocuments)))
		return FALSE;

	LPTSTR pszNewFolder = _newstring(2 * MAX_PATH);

	if (pszNewFolder == NULL)
		return FALSE;


	// Add UNC prefix for long path
	StringCbPrintf(pszNewFolder, MAX_PATH,
		_T("\\\\?\\%s\\%s"),
		szMyDocuments,
		pszSimFolder);


	//
	// 'CreateDirectory' is not recursive function.
	// You have to create the first folder and then the second
	//
	// At this point 'pszNewFolder' should be <My Documents>\<Sim folder>
	//
	if (!IsFolderExists(pszNewFolder))
	{
		// <My Documents>\<Sim folder>
		if (!::CreateDirectory(pszNewFolder, NULL))
		{
			TRACE1("MyCreateDirectory failed. Error code: %d\n", GetLastError());
			return FALSE;
		}
		else
		{
			StringCbCat(pszNewFolder, MAX_PATH, _T("\\QAR"));
			//
			// At this point 'pszNewFolder' should be <My Documents>\<Sim folder>\QAR
			//
			if (!IsFolderExists(pszNewFolder))
			{
				if (!::CreateDirectory(pszNewFolder, NULL))
				{
					TRACE1("MyCreateDirectory failed. Error code: %d\n", GetLastError());
					return FALSE;
				}
			}
		}
	}
	else
	{
		StringCbCat(pszNewFolder, MAX_PATH, _T("\\QAR"));
		if (!IsFolderExists(pszNewFolder))
		{
			if (!::CreateDirectory(pszNewFolder, NULL))
				return FALSE;
		}
	}


	switch (st.wMonth)
	{
	case 1:
		pszMonth = TEXT("Jan");
		break;
	case 2:
		pszMonth = TEXT("Feb");
		break;
	case 3:
		pszMonth = TEXT("March");
		break;
	case 4:
		pszMonth = TEXT("Apr");
		break;
	case 5:
		pszMonth = TEXT("May");
		break;
	case 6:
		pszMonth = TEXT("Jun");
		break;
	case 7:
		pszMonth = TEXT("Jul");
		break;
	case 8:
		pszMonth = TEXT("Aug");
		break;
	case 9:
		pszMonth = TEXT("Sept");
		break;
	case 10:
		pszMonth = TEXT("Oct");
		break;
	case 11:
		pszMonth = TEXT("Nov");
		break;
	case 12:
		pszMonth = TEXT("Dec");
		break;
	}

	//
	// At this point 'pszNewFolder' should be <My Documents>\<Sim folder>\QAR\
	//
	StringCbPrintf(m_pszOutputFolder, MAX_PATH, 
		_T("%s\\%s_%u"),
		pszNewFolder, pszMonth, st.wYear);

	//
	// At this point 'pszNewFolder' should be 
	// <My Documents>\<Sim folder>\QAR\Month_Year
	//
	if (!IsFolderExists(m_pszOutputFolder))
	{
		if (!::CreateDirectory(m_pszOutputFolder, NULL))
			return FALSE;
	}
	_freestring(pszNewFolder);

	TRACE1("Output Folder: %s\n", m_pszOutputFolder);

	return TRUE;
}

void status1(LPCTSTR lpszMessage)
{
	TRACE1("%s\n", lpszMessage);

#ifndef _DEBUG
	// Reserved
	UNREFERENCED_PARAMETER(lpszMessage);
#endif

}

// !!!!! Makes two identical strings ????????
BOOL CSimConnectOutput::PrepareArchive( LPTSTR pszArchivePath, LPTSTR pszFilePath)
{
	if (FAILED(StringCbPrintf(pszArchivePath, MAX_PATH,
		_T("%s\\%s.gz"),
		m_pszOutputFolder, m_pszCurrentFileName)))
	{
		return FALSE;
	}

	if (FAILED(StringCbPrintf(pszFilePath, MAX_PATH,
		_T("%s\\%s.csv"),
		m_pszOutputFolder, m_pszCurrentFileName)))
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CSimConnectOutput::AnsiFileName(LPSTR pszAnsiName)
{

#ifndef _UNICODE
	StringCbCopy(pszAnsiName, SHORT_STR_LEN, szFileName);
#else
	_wcstombsz(pszAnsiName, m_pszCurrentFileName, SHORT_STR_LEN);
#endif

	return TRUE;
}

#pragma warning(suppress: 6262)
BOOL CSimConnectOutput::DoCompression(LPCTSTR pszFilePath,  LPCTSTR pszArchivePath, LPCSTR pszANSIName)
{
	HANDLE in, out;
	ZLIB zs;
#pragma warning(suppress: 6001)
	InitZipLib(&zs, &status1, NULL);
	in = FileOpen(pszArchivePath, FO_READ);
	if (in == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	out = FileOpen(pszFilePath, FO_WRITE);
	if (out == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	if (CompressFile(&zs, in, out, pszANSIName) != Z_OK)
	{
		AfxGetApp()->Error(IDS_ERROR_COMPRESSION);
		TRACE0("CSimConnectOutput::ZipOutputFile(): Failed compress file\n");
	}

	CloseHandle(in);
	CloseHandle(out);

	return TRUE;
}

BOOL CSimConnectOutput::ZipOutputFile()
{

#ifdef _ALX_USE_LITEZIP

	if (m_fZipOutputFile) 
	{
		LPTSTR pszArchivePathName = _newstring(MAX_PATH);
		StringCbPrintf(pszArchivePathName, MAX_PATH,
			_T("%s\\%s.zip"),
			m_pszOutputFolder, m_pszCurrentFileName);

		HZIP hz = NULL;
		ZipCreateFile(&hz, pszArchivePathName, 0);
		if (!hz)
		{
			TRACE0("Create zip-file error\n");
			TRACE1("Cannot compress file %s\n", m_pszCurrentFileName);
			return FALSE;
		}
		_freestring(pszArchivePathName);
		pszArchivePathName = NULL;


		LPTSTR pszFileName = _newstring(SHORT_STR_LEN);
		StringCbPrintf(pszFileName, SHORT_STR_LEN,
			_T("%s.csv"), m_pszCurrentFileName);


		LPTSTR pszFilePath = _newstring(MAX_PATH);
		StringCbPrintf(pszFilePath, MAX_PATH,
			_T("%s\\%s.csv"),
			m_pszOutputFolder, m_pszCurrentFileName);

		DWORD dwRes = ZipAddFile(hz, pszFileName, pszFilePath);
		if (dwRes)
		{
			TRACE1("ZipAddFile() Error: %d\n", dwRes);
			TRACE1("Cannot compress file %s\n", m_pszCurrentFileName);
			return FALSE;
		}

		_freestring(pszFileName);
		pszFileName = NULL;


		ZipClose(hz);
		

		TRACE1("Create archive file %s.zip\n", m_pszCurrentFileName);
		if (DeleteFile(pszFilePath))
		{
			TRACE1("File %s.csv deleted\n", m_pszCurrentFileName);
		}

		_freestring(pszFilePath);
		pszFilePath = NULL;
	}

#endif // _ALX_USE_LITEZIP

#ifdef _ALX_USE_ZIPLIB

	if (m_fZipOutputFile) 
	{

		TCHAR szFilePath[MAX_PATH] = {'\0'};
		TCHAR szArchivePath[MAX_PATH] = {'\0'};
		char szANSIName[SHORT_STR_LEN] = {'\0'};

		if (!PrepareArchive(szArchivePath, szFilePath))
			return FALSE;

		if (!AnsiFileName(szANSIName))
			return FALSE;

		TRACE0("Now ZipLib is working: ");
		if (!DoCompression(szArchivePath, szFilePath, szANSIName))
			return FALSE;

		TRACE1("Create archive file %s\n", szArchivePath);
		if (DeleteFile(szFilePath))
		{
			TRACE1("File %s was deleted\n", szFilePath);
		}

		ClearOutputFolder();

	}
#endif // _ALX_USE_ZIPLIB

	return TRUE;
}

void CSimConnectOutput::SetDatafileSeparator(INT nSeparator)
{
	m_pFileOutput->SetSeparator(nSeparator);
}


BOOL CSimConnectOutput::ClearOutputFolder()
{
	ASSERT(m_pszOutputFolder);

	if (m_pszOutputFolder)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		TCHAR szTemplate[2 * MAX_PATH];

		if (FAILED(StringCbPrintf(szTemplate, 2 * MAX_PATH, _T("%s\\*.csv"), m_pszOutputFolder)))
			return FALSE;

		hFind = FindFirstFile(szTemplate, &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			TRACE1("Invalid File Handle. GetLastError reports %d\n", GetLastError());
			return FALSE;
		}
		else
		{
			TCHAR szFilePath[2 * MAX_PATH];
			StringCbPrintf(szFilePath, 2 * MAX_PATH, _T("%s\\%s"), m_pszOutputFolder, FindFileData.cFileName);
			if (DeleteFile(szFilePath))
			{
				TRACE1("File %s was deleted\n", szFilePath);
			}

			while (FindNextFile(hFind, &FindFileData) != 0)
			{
				StringCbPrintf(szFilePath, 2 * MAX_PATH, _T("%s\\%s"), m_pszOutputFolder, FindFileData.cFileName);
				if (DeleteFile(szFilePath))
				{
					TRACE1("File %s was deleted\n", szFilePath);
				}

			}

			DWORD dwError = GetLastError();
			FindClose(hFind);
			if (dwError != ERROR_NO_MORE_FILES)
			{
				TRACE1("FindNextFile error. Error is %u\n", dwError);
				return FALSE;
			}

			return TRUE;
		}
	}

	return TRUE;
}
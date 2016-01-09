
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

#ifdef _ALX_USE_LITEZIP
#include "..\litezip\litezip.h"
#endif

#ifdef _ALX_USE_ZIPLIB
#include "..\ziplib\zlib.h"
#include "..\ziplib\zutil.h"
#endif

#include "zipimpl.h"
#include "resource.h"

//
// The  next functions are used in CompressFile()
//
// deflateEnd
// deflate
// deflateInit2_ 
// crc32
//


//////////////////////////////////////////////////////////////////////////

#define ZIP_HEADER_SIZE	16

// Application defined error code
#define Z_USER_ABORT (-7)


// Default handler
static void InternalStatusMessage(LPCTSTR pszText)
{
	UNREFERENCED_PARAMETER(pszText);
}

// Default handler
static void InternalStatusProgress(int nPercent)
{
	UNREFERENCED_PARAMETER(nPercent);
}

void InitZipLib(__in ZLIB* pZlib,
	__in_opt STATUS_MESSAGE pfnStatusMessage,
	__in_opt STATUS_PROGRESS pfnStatusProgress)
{
	pZlib->zstream.zalloc = 0;
	pZlib->zstream.zfree = 0;
	pZlib->zstream.opaque = 0;
	pZlib->zstream.next_in = Z_NULL;
	pZlib->zstream.next_out = Z_NULL;
	pZlib->zstream.avail_in = 0;
	pZlib->zstream.avail_out = 0;
	pZlib->zstream.msg = NULL;
	pZlib->crc = crc32(0L, Z_NULL, 0);

	if (pfnStatusMessage != NULL)
		pZlib->pfnStatusMessage = pfnStatusMessage;
	else
		pZlib->pfnStatusMessage = &InternalStatusMessage;

	if (pfnStatusProgress != NULL)
		pZlib->pfnStatusProgress = pfnStatusProgress;
	else
		pZlib->pfnStatusProgress = &InternalStatusProgress;
}

//
// Outputs a long in LSB (Least Significant Byte) order
// to the given file
//
static void putLSBLong(HANDLE hFile, uLong x)
{
	int n;
	DWORD dwBytesWritten;
	unsigned char buf[6];
	for (n = 0; n < 4; n++)
	{
		buf[0] = (unsigned char)(x & (unsigned char) 0xff);
		WriteFile(hFile, buf, 1, &dwBytesWritten, NULL);
		//f.Write(buf,1);
		x >>= 8;
	}
}


//
// File operations
//

#define FO_READ  1
#define FO_WRITE 2

// Specifies the type of access to the file. This parameter can be any combination 
// of the following values.
//
// FO_READ Specifies read access to the file.
// FO_WRITE Specifies write access to the file.


// dwCreationDistribution

// Specifies which action to take on files that exist, and which action to take when 
// files do not exist. This parameter must be one of the following values: 

// Value	Meaning
// CREATE_NEW	Creates a new file. The function fails if the specified file already exists.
// CREATE_ALWAYS	Creates a new file. The function overwrites the file if it exists.
// OPEN_EXISTING	Opens the file. The function fails if the file does not exist.
// OPEN_ALWAYS	Opens the file, if it exists. If the file does not exist, the function creates the file as if dwCreationDistribution were CREATE_NEW.

__checkReturn
HANDLE FileOpen(__in_z LPCTSTR pszFileName,
				__in UINT mode)
{
	//DWORD dwAccess;

	if (mode == FO_READ)
	{
		return CreateFile(pszFileName,
			GENERIC_READ,
			/*FILE_SHARE_READ|*/FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	}
	else if (mode == FO_WRITE)
	{
		return CreateFile(pszFileName,
			GENERIC_WRITE,
			FILE_SHARE_READ/*|FILE_SHARE_WRITE*/,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	}
	return NULL;
}

static LONG FileSeek(HANDLE hFile, LONG lOff, UINT nFrom)
{
	return SetFilePointer((HANDLE)hFile, lOff, NULL, (DWORD)nFrom);
}

static INT Write(HANDLE hFile, LPVOID lpBuffer, DWORD dwBytes)
{
	DWORD dw;
	return WriteFile(hFile, lpBuffer, dwBytes, &dw, NULL);
}

static ULONG Read(HANDLE hFile, LPBYTE lpBuffer, DWORD dwBuffSize)
{
	ULONG dw;
	if (!ReadFile(hFile, lpBuffer,  dwBuffSize, &dw, NULL))
	{
		TRACE2("ZipImpl.cpp Read() failed. hFile: %#x, Bytes Read: %ul\n",
			hFile, dw);
	}
	return dw;
}

static DWORD GetStrLength(const char* lpszString)
{
	return (DWORD)lstrlenA(lpszString);
}

static LONG GetFileLength(HANDLE hFile)
{
	DWORD dwSize;
	::GetFileSize(hFile, &dwSize);
	return (LONG)dwSize;
}



//  =========================================================
//  Every time Zlib consumes all of the data in the
//  input buffer, this function gets called to reload.
//  The avail_in member is part of z_stream, and is
//  used to keep track of how much input is available.
//  I churn the Windows message loop to ensure that
//  the process can be aborted by a button press or
//  other Windows event.
//  =========================================================
int load_block(ZLIB* pZlib,  HANDLE inFile)
{
	if (pZlib->zstream.avail_in == 0)
	{
		pZlib->zstream.next_in = pZlib->input_buffer;
		pZlib->zstream.avail_in = Read(inFile, 
			(LPBYTE)&(pZlib->input_buffer), 
			INPUT_LENGTH);
	}
	return pZlib->zstream.avail_in;
}

//  =========================================================
//  Every time Zlib fills the output buffer with data,
//  this function gets called.  Its job is to write
//  that data out to the output file, then update
//  the z_stream member avail_out to indicate that more
//  space is now available.  I churn the Windows message
//  loop to ensure that the process can be aborted by a
//  button press or other Windows event.
//  =========================================================

int flush_block(ZLIB* pZlib, HANDLE outFile)
{
	unsigned int count = OUTPUT_LENGTH - pZlib->zstream.avail_out;
	if (count)
	{
		Write(outFile, pZlib->output_buffer, count);
		pZlib->zstream.next_out = pZlib->output_buffer;
		pZlib->zstream.avail_out = OUTPUT_LENGTH;
	}
	return count;
}


//  =========================================================
//  This function is called so as to provide the progress()
//  virtual function with a reasonable figure to indicate
//  how much processing has been done.  Note that the length
//  member is initialized when the input file is opened.
//  =========================================================
int percent(ZLIB* pZlib)
{
	if (pZlib->length == 0)
		return 100;
	else if (pZlib->length > 10000000L)
		return (pZlib->zstream.total_in /(pZlib->length / 100));
	else
		return (pZlib->zstream.total_in * 100 / pZlib->length);
}

//
// Returns Zlib error code
//
__checkReturn
int CompressFile(__in ZLIB* pZlib,
				__in HANDLE hInputFile,
				__in HANDLE hOutputFile,
				__in_z LPCSTR pNameInArchive)
{
	int gz_magic[2] = {0x1f, 0x8b};
	char header[ZIP_HEADER_SIZE];	// ANSI (!!!)
	char* desc;

	pZlib->err = Z_OK;
	pZlib->zstream.avail_in = 0;
	pZlib->zstream.avail_out = OUTPUT_LENGTH;
	pZlib->zstream.next_out = pZlib->output_buffer;
	pZlib->nAbortFlag = 0;
	FileSeek(hInputFile, 0, FILE_BEGIN);

	// Write a gzip magic header to the file
	memset(header, 0, sizeof(header));

	// ANSI (!!!)
	StringCchPrintfA(header, ZIP_HEADER_SIZE,
		"%c%c%c%c%c%c%c%c%c%c",
		gz_magic[0],
		gz_magic[1],
		Z_DEFLATED,
		24,				// flags - Bit 3:filename 4:comment present
		0,0,0,0,		// time
		2,				// xflags - 02 = better compression
		OS_CODE);
	Write(hOutputFile, header, 10L);

	// Write a filename
	Write(hOutputFile, (LPVOID)pNameInArchive, GetStrLength(pNameInArchive));
	Write(hOutputFile, "\0", 1);

	//  Write a comment
	desc = "Created by FSQAR (compressed by ZLib)";
	Write(hOutputFile, (LPVOID)desc, GetStrLength(desc));
	Write(hOutputFile, "\0", 1);

	// Init checksum for running total
	pZlib->crc = crc32(0L, Z_NULL, 0);

	// Determine total bytes to compress
	pZlib->length = GetFileLength(hInputFile);

	//
	// Init deflate library with a negative window value. A neg value
	// will suppress creation of the zlib header.
	//
	pZlib->err = deflateInit2(&(pZlib->zstream), 9, Z_DEFLATED,
		-MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
	//
	// Loop until entire file is compressed
	for (; ;)
	{
		if (pZlib->nAbortFlag)
			break;
		if (!load_block(pZlib, hInputFile))
			break;
		pZlib->crc = crc32(pZlib->crc, (const Bytef *)pZlib->input_buffer, 
			pZlib->zstream.avail_in);
		pZlib->err = deflate(&(pZlib->zstream), Z_NO_FLUSH);
		flush_block(pZlib, hOutputFile);
		if (pZlib->err != Z_OK)
			break;
		// TODO: progress bar
		pZlib->pfnStatusProgress(percent(pZlib));
	}

	for (; ;)
	{
		if (pZlib->nAbortFlag)
			break;
		pZlib->err = deflate(&(pZlib->zstream), Z_FINISH);
		if (!flush_block(pZlib, hOutputFile))
			break;
		if (pZlib->err != Z_OK)
			break;
	}

	pZlib->pfnStatusProgress(percent(pZlib));
	deflateEnd(&(pZlib->zstream));


	if (pZlib->nAbortFlag)
	{
		// TODO: status message
		pZlib->pfnStatusMessage(_T("User Abort"));
	}
	else if (pZlib->err != Z_OK && pZlib->err != Z_STREAM_END)
	{
		pZlib->pfnStatusMessage(_T("Zlib Error"));
	}
	else
	{
		pZlib->pfnStatusMessage(_T("Success"));
		pZlib->err = Z_OK;
	}

	// Write the file terminator
	putLSBLong(hOutputFile, pZlib->crc);
	putLSBLong(hOutputFile, pZlib->zstream.total_in);

	if (pZlib->nAbortFlag)
		return Z_USER_ABORT;
	else
		return pZlib->err;
}

//////////////////////////////////////////////////////////////////////////
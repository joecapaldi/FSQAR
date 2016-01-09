

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

#ifndef __ZIPIMPL_H__
#define __ZIPIMPL_H__

#ifdef _ALX_USE_LITEZIP
#include "..\litezip\litezip.h"
#endif

#ifdef _ALX_USE_ZIPLIB
#include "..\ziplib\zlib.h"
#endif

// Original size is 16k = 16384	but in this case a big allocation on stack
// appears in function call 
// In-out buffer size
#define	IO_BUFFER_SIZE		16384	
#define INPUT_LENGTH		IO_BUFFER_SIZE
#define OUTPUT_LENGTH		IO_BUFFER_SIZE

// Redefinitions
#define FO_READ				1
#define FO_WRITE			2

// Callback function types
typedef void (*STATUS_MESSAGE)(LPCTSTR pszMessage);
typedef void (*STATUS_PROGRESS)(INT nPercent);

typedef struct _ZLIB
{
	z_stream zstream;
	uLong crc;
	int err;
	long length;
	int nAbortFlag;

	unsigned char input_buffer[INPUT_LENGTH];
	unsigned char output_buffer[OUTPUT_LENGTH];

	STATUS_MESSAGE pfnStatusMessage;
	STATUS_PROGRESS pfnStatusProgress;
}ZLIB;



__checkReturn
HANDLE FileOpen(__in_z LPCTSTR pszFileName, 
				__in UINT mode);

void InitZipLib(__in ZLIB* pZlib, 
				__in_opt STATUS_MESSAGE pfnStatusMessage, 
				__in_opt STATUS_PROGRESS pfnStatusProgress);

__checkReturn
int CompressFile(__in ZLIB* pZlib, 
				  __in HANDLE hInputFile, 
				  __in HANDLE hOutputFile, 
				  __in_z LPCSTR pNameInArchive);

//////////////////////////////////////////////////////////////////////////

#endif // __ZIPIMPL_H__

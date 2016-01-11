
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

#ifndef __STDAFX_H__
#define  __STDAFX_H__

#define _WIN32_WINNT	0x0601	// Windows 7
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <process.h>
#include <shellapi.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <Commdlg.h>
#include <commctrl.h>
#include <htmlhelp.h>


// we use source annotation and analyze tool
#include <sal.h>

// ======================================================================
// These definitions drive compilation process
//
//   TEST_WITHOUT_SIM  it does not call SimConnect API and may not be linked
//                     with SimConnect.lib, no simulator required for this
// ======================================================================
//#define TEST_WITHOUT_SIM
//#define SIMCONNECT_DYNAMIC_LOAD
#define _NO_INTEL_ADVISOR
// Comment this macro for using Windows Tray
#define _ALX_NO_TRAYICON

// Uncomment this macro for building without toolbar
//#define _ALX_NO_TOOLBAR

// Uncomment this macro for building with LiteZip library instead zlib
//#define _ALX_USE_LITEZIP

// Comment this macro for using LiteZip library instead zlib
#define _ALX_USE_ZIPLIB

// Comment this macro for using your own AfxAssertFailedLine()
#define _ALX_USE_CRT_ASSERT
// -----------------------------------------------------------------------

#ifndef TEST_WITHOUT_SIM
#ifdef SIMCONNECT_DYNAMIC_LOAD
#error "SimConnect dynamic loader must be implemented!!!"
#endif
#endif // TEST_WITHOUT_SIM

#include <SimConnect.h>			// use SDK header
//#include "simconnect_p3d_v3.h"		// use internel header

// WARNING: The #include statements must be in the order shown here. 
// If you change the order, the functions you use may not work properly.
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef _MT
#error This program must be compiled witn _MT directive
#endif

//
// From yvals.h
//
#define __STR2WSTR(str)    L##str
#define _STR2WSTR(str)     __STR2WSTR(str)

#if _MSC_VER < 1900
#define __FILEW__          _STR2WSTR(__FILE__)
#define __FUNCTIONW__      _STR2WSTR(__FUNCTION__)
#endif


// Intel Advisor Build Options:
// 
// $(ADVISOR_XE_2013_DIR)\include 
// $(ADVISOR_XE_2013_DIR)\lib32
// 
// Linker-> add libadvisor.lib
// 

#ifndef _NO_INTEL_ADVISOR

#pragma warning(push)
#pragma warning(disable:4100)	// unreferenced formal parameter
#include <advisor-annotate.h>
#pragma warning(pop)

#endif


// TODO: Include yours header here
#include "afxdefs.h"
//#include "objmodel.h"


#endif // __STDAFX_H__

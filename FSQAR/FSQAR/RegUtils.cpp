
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
#include "registry.h"
#include "resource.h"



BOOL LoadWindowPos(LPINT lpX,
				LPINT lpY,
				LPINT lpDX,
				LPINT lpDY)
{
	ASSERT(lpX);
	ASSERT(lpY);
	ASSERT(lpDX);
	ASSERT(lpDY);

#if _MSC_VER > 1700
	*lpX = 0;
	*lpY = 0;
	*lpDX = 0;
	*lpDY = 0;
#else
	__analysis_assume(lpX);
	__analysis_assume(lpY);
	__analysis_assume(lpDX);
	__analysis_assume(lpDY);
#endif

	DWORD x, y, dx, dy;

	CRegistry Reg(HKEY_CURRENT_USER);

	Reg.OpenKey(APP_MAIN_KEY);

	// quick exit on error
	if (!Reg.ReadInteger(_T("nWindowPosX"), &x))
		return FALSE;

	Reg.ReadInteger(_T("nWindowPosY"), &y);
	Reg.ReadInteger(_T("nWindowPosDX"), &dx);
	Reg.ReadInteger(_T("nWindowPosDY"), &dy);

	Reg.CloseKey();

	if ((dx == 0) && (dy == 0))
		return FALSE;

	*lpX = x;
	*lpY = y;
	*lpDX = dx;
	*lpDY = dy;

	return TRUE;
}


BOOL SaveWindowPos(HWND hWnd, int nWindowPosX, int nWindowPosY, int nWindowPosDX, int nWindowPosDY)
{
	CRegistry Reg(HKEY_CURRENT_USER);
	
	if (Reg.OpenKey(APP_MAIN_KEY) == NULL)
		return FALSE;

	if ((!IsIconic(hWnd)) && (!IsZoomed(hWnd)))
	{
		RECT rc;
		GetWindowRect(hWnd, & rc);
		nWindowPosX = rc.left;
		nWindowPosY = rc.top;
		nWindowPosDX = rc.right - rc.left;
		nWindowPosDY = rc.bottom - rc.top;
	}
	else
	{
		nWindowPosX = DEFAULT_WINDOWS_X;
		nWindowPosY = DEFAULT_WINDOWS_Y;
		nWindowPosDX = DEFAULT_WINDOWS_DX;
		nWindowPosDY = DEFAULT_WINDOWS_DY;
	}


	Reg.WriteInteger(_T("nWindowPosX"), (LPDWORD)&nWindowPosX);
	Reg.WriteInteger(_T("nWindowPosY"), (LPDWORD)&nWindowPosY);
	Reg.WriteInteger(_T("nWindowPosDX"), (LPDWORD)&nWindowPosDX);
	Reg.WriteInteger(_T("nWindowPosDY"), (LPDWORD)&nWindowPosDY);


	Reg.CloseKey();

	return TRUE;
}


BOOL SaveFontToRegistry(const LOGFONT* lpLogFont)
{
	ASSERT(lpLogFont);
	CRegistry Reg(HKEY_CURRENT_USER);

	if (!Reg.IsKeyExist(APP_MAIN_KEY))
		Reg.CreateKey(APP_MAIN_KEY);

	if (!Reg.OpenKey(APP_MAIN_KEY))
		return FALSE;

	if (!Reg.WriteString(_T("FontName"), (LPTSTR)lpLogFont->lfFaceName))
		return FALSE;

	Reg.WriteInteger(_T("Italic"), (LPDWORD)&lpLogFont->lfItalic);
	Reg.WriteInteger(_T("Height"), (LPDWORD)&lpLogFont->lfHeight);
	Reg.WriteInteger(_T("Weight"),  (LPDWORD)&lpLogFont->lfWeight);
	Reg.WriteInteger(_T("Underline"), (LPDWORD)&lpLogFont->lfUnderline);
	Reg.WriteInteger(_T("CharSet"), (LPDWORD)&lpLogFont->lfCharSet);
	Reg.WriteInteger(_T("StrikeOut"), (LPDWORD)&lpLogFont->lfStrikeOut);

	Reg.CloseKey();

	return TRUE;
}

BOOL LoadFontFromRegistry(LOGFONT* lpLogFont)
{
	TCHAR szBuffer[LF_FACESIZE];
	CRegistry Reg(HKEY_CURRENT_USER);
	LOGFONT defaultFont;

	// This returned on error
	defaultFont.lfItalic = 0;
	defaultFont.lfCharSet = DEFAULT_CHARSET;
	defaultFont.lfHeight = 12;
	defaultFont.lfUnderline = 0;
	defaultFont.lfWeight = 0;
	StringCbCopy(defaultFont.lfFaceName, LF_FACESIZE, _T("Arial"));

	if (!Reg.IsKeyExist(APP_MAIN_KEY))
	{
		lpLogFont = &defaultFont;
		return FALSE;
	}
		
	if (!Reg.OpenKey(APP_MAIN_KEY))
	{
		lpLogFont = &defaultFont;
		return FALSE;
	}

	if (Reg.ReadString(_T("FontName"), (LPTSTR)szBuffer))
		StringCbCopy(lpLogFont->lfFaceName, LF_FACESIZE, szBuffer);

	Reg.ReadInteger(_T("Italic"), (LPDWORD)&lpLogFont->lfItalic);
	Reg.ReadInteger(_T("Height"), (LPDWORD)&lpLogFont->lfHeight);
	Reg.ReadInteger(_T("Weight"), (LPDWORD)&lpLogFont->lfWeight);
	Reg.ReadInteger(_T("Underline"), (LPDWORD)&lpLogFont->lfUnderline);
	Reg.ReadInteger(_T("CharSet"), (LPDWORD)&lpLogFont->lfCharSet);
	Reg.ReadInteger(_T("StrikeOut"), (LPDWORD)&lpLogFont->lfStrikeOut);

	Reg.CloseKey();

	return TRUE;

}

BOOL ReadSimClientOptions(SIMCLIENT_OPTIONS* pSimClientOpt)
{
	CRegistry Reg(HKEY_CURRENT_USER);

	SIMCLIENT_OPTIONS defaultOptions;
	defaultOptions.fPrintFile = FALSE;
	defaultOptions.fUseSchedule = TRUE;
	defaultOptions.fUseTimer = FALSE;

	if (!Reg.IsKeyExist(APP_MAIN_KEY))
	{
		pSimClientOpt = &defaultOptions;
		return FALSE;
	}

	if (!Reg.OpenKey(APP_MAIN_KEY))
	{
		pSimClientOpt = &defaultOptions;
		return FALSE;
	}

	Reg.ReadInteger(_T("PrintFile"), (LPDWORD)&pSimClientOpt->fPrintFile);
	Reg.ReadInteger(_T("UseSchedule"), (LPDWORD)&pSimClientOpt->fUseSchedule);
	Reg.ReadInteger(_T("UseTimer"), (LPDWORD)&pSimClientOpt->fUseTimer);
	Reg.ReadInteger(_T("Separator"), (LPDWORD)&pSimClientOpt->nSeparator);

	Reg.CloseKey();

	return TRUE;
}


BOOL SaveSimClientOptions(const SIMCLIENT_OPTIONS* pSimOptions)
{
	ASSERT(pSimOptions);
	CRegistry Reg(HKEY_CURRENT_USER);

	if (!Reg.IsKeyExist(APP_MAIN_KEY))
		Reg.CreateKey(APP_MAIN_KEY);

	if (!Reg.OpenKey(APP_MAIN_KEY))
		return FALSE;

	if (!Reg.WriteInteger(_T("UseSchedule"), (LPDWORD)&pSimOptions->fUseSchedule))
		return FALSE;

	Reg.WriteInteger(_T("PrintFile"), (LPDWORD)&pSimOptions->fPrintFile);
	Reg.WriteInteger(_T("UseTimer"), (LPDWORD)&pSimOptions->fUseTimer);
	Reg.WriteInteger(_T("Separator"), (LPDWORD)&pSimOptions->nSeparator);

	Reg.CloseKey();

	return TRUE;

}

//////////////////////////////////////////////////////////////////////////

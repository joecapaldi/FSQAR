
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

void CSimClientOptions::SetDefaultOptions()
{
	m_Options.fUseTimer = FALSE;
	m_Options.fPrintFile = TRUE;
	m_Options.fUseSchedule = TRUE;
	m_Options.nSeparator = DATAFILE_SEPARATOR_COMMA;
}

void CSimClientOptions::SetOptions(__in const SIMCLIENT_OPTIONS* pSimOptions)
{
	ASSERT(pSimOptions);

	// Write 'MainThread' (UI thread)
	Lock lock(&g_csAppData);
	m_Options.fUseTimer = pSimOptions->fUseTimer;
	m_Options.fPrintFile = pSimOptions->fPrintFile;
	m_Options.fUseSchedule = pSimOptions->fUseSchedule;
	m_Options.nSeparator = pSimOptions->nSeparator;
	lock.Release();
}

void CSimClientOptions::GetOptions(__inout SIMCLIENT_OPTIONS* pSimOptions)
{
	ASSERT(pSimOptions);
	(*pSimOptions).fPrintFile = m_Options.fPrintFile;
	(*pSimOptions).fUseSchedule = m_Options.fUseSchedule;
	(*pSimOptions).fUseTimer = m_Options.fUseTimer;
	(*pSimOptions).nSeparator = m_Options.nSeparator;
}

void CSimClientOptions::SaveOptions()
{

	if (!SaveSimClientOptions(&m_Options))
	{
		TRACE0("CSimClientOptions::SaveOptions(): Failed to save SimClient options\n");
	}
}

BOOL CSimClientOptions::ReadOptions()
{
	BOOL fRead;
	Lock lock(&g_csAppData);
	fRead = ::ReadSimClientOptions(&m_Options);
	lock.Release();

	if (fRead)
	{
		SetOptions(&m_Options);
		return TRUE;
	}
	else
	{
		SetDefaultOptions();
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////////


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



// Get the ID of the last packet sent to the SimConnect server and save the ID
// and sender name into the record
void CSimConnect::AddSender(const char* pSenderName)
{
#ifndef SIMCONNECT_DYNAMIC_LOAD
	DWORD id;
	
	if (m_SenderID < MAX_EXCEPTION_RECORDS)
	{
		m_hCurrentResult = ::SimConnect_GetLastSentPacketID(m_hSimConnect, &id);
		strncpy_s(aDataSender[m_SenderID].szSenderName, 255, pSenderName, 255);
		aDataSender[m_SenderID ].dwSenderID = id;
		++m_SenderID;
	}
#endif // SIMCONNECT_DYNAMIC_LOAD
}

// Given the ID of an erroneous packet, find the identification string of the call
char* CSimConnect::FindSender(DWORD dwSenderID)
{
	BOOL found  = FALSE;
	int count   = 0;
	
	while ((!found) && (count < m_SenderID))
	{
		if (dwSenderID == aDataSender[count].dwSenderID)
		{
			found = TRUE;
			return aDataSender[count].szSenderName;
		}
			
		++count;
	}
	
	return "Exception Record not found";
}


void CSimConnect::EventException(SIMCONNECT_RECV_EXCEPTION* pException)
{
	{
		TCHAR sz[LONG_STR_LEN];
		StringCbPrintf(sz, LONG_STR_LEN,
		          _T("SimConnect Exception: dwException = %u, dwSendID = %u, dwIndex = %u\n"),
		          pException->dwException,
		          pException->dwSendID,
		          pException->dwIndex);
		TRACE1("%s\n", sz);

	}
	TRACE0("----------------------------------------------------------------------\n");

#if 1

	switch (pException->dwException)
	{
	case SIMCONNECT_EXCEPTION_NONE:
	case SIMCONNECT_EXCEPTION_ERROR:
		TRACE0("An unspecific error has occurred. This can be from incorrect flag settings, null or incorrect parameters, the need to have at least one up or down event with an input event, failed calls from the SimConnect server to the operating system, among other reasons.");
		break;
	case SIMCONNECT_EXCEPTION_SIZE_MISMATCH:
		TRACE0("Specifies the size of the data provided does not match the size required. This typically occurs when the wrong string length, fixed or variable, is involved.");
		break;
	case SIMCONNECT_EXCEPTION_UNRECOGNIZED_ID:
		TRACE0("Client event, request ID, data definition ID, or object ID was not recognized.\n");
		break;
	case SIMCONNECT_EXCEPTION_VERSION_MISMATCH:
	{
		if (pException->dwIndex == 4)
		{
			// The original RTM version of FSX here
		}
		else
		{
			// FSX SP1 and subsequent releases here
			WORD wLo, wHi;
			wLo = LOWORD(pException->dwIndex);
			wHi = HIWORD(pException->dwIndex);
		}
	}
	break;
	case SIMCONNECT_EXCEPTION_TOO_MANY_GROUPS:
	case SIMCONNECT_EXCEPTION_NAME_UNRECOGNIZED:
	{
		TCHAR sz[LONG_STR_LEN];
		StringCbPrintf(sz, LONG_STR_LEN,
		          _T("The exception details: dwException = %u, dwSendID = %u, dwIndex = %u\n\n"),
		          pException->dwException,
		          pException->dwSendID,
		          pException->dwIndex);
		//m_EditOutput << sz;
	}
	break;
	case SIMCONNECT_EXCEPTION_TOO_MANY_EVENT_NAMES:
	case SIMCONNECT_EXCEPTION_EVENT_ID_DUPLICATE:
	case SIMCONNECT_EXCEPTION_TOO_MANY_MAPS:
	case SIMCONNECT_EXCEPTION_TOO_MANY_OBJECTS:
	case SIMCONNECT_EXCEPTION_TOO_MANY_REQUESTS:
	case SIMCONNECT_EXCEPTION_WEATHER_INVALID_PORT:
	case SIMCONNECT_EXCEPTION_WEATHER_INVALID_METAR:
	case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_GET_OBSERVATION:
	case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_CREATE_STATION:
	case SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_REMOVE_STATION:
	case SIMCONNECT_EXCEPTION_INVALID_DATA_TYPE:
	case SIMCONNECT_EXCEPTION_INVALID_DATA_SIZE:
	case SIMCONNECT_EXCEPTION_DATA_ERROR:
	case SIMCONNECT_EXCEPTION_INVALID_ARRAY:
	case SIMCONNECT_EXCEPTION_CREATE_OBJECT_FAILED:
	case SIMCONNECT_EXCEPTION_LOAD_FLIGHTPLAN_FAILED:
	case SIMCONNECT_EXCEPTION_OPERATION_INVALID_FOR_OBJECT_TYPE:
	case SIMCONNECT_EXCEPTION_ILLEGAL_OPERATION:
	case SIMCONNECT_EXCEPTION_ALREADY_SUBSCRIBED:
	case SIMCONNECT_EXCEPTION_INVALID_ENUM:
	case SIMCONNECT_EXCEPTION_DEFINITION_ERROR:
	case SIMCONNECT_EXCEPTION_DUPLICATE_ID:
	case SIMCONNECT_EXCEPTION_DATUM_ID:
	case SIMCONNECT_EXCEPTION_OUT_OF_BOUNDS:

#if TARGET_SIMVERSION > FSXSP2
	case SIMCONNECT_EXCEPTION_ALREADY_CREATED:
	case SIMCONNECT_EXCEPTION_OBJECT_OUTSIDE_REALITY_BUBBLE:
	case SIMCONNECT_EXCEPTION_OBJECT_CONTAINER:
	case SIMCONNECT_EXCEPTION_OBJECT_AI:
	case SIMCONNECT_EXCEPTION_OBJECT_ATC:
	case SIMCONNECT_EXCEPTION_OBJECT_SCHEDULE:
#endif
		break;
	default:
	{
		TCHAR sz[LONG_STR_LEN];
		StringCbPrintf(sz, LONG_STR_LEN,
		          _T("Unknown exception: dwException = %u, dwSendID = %u, dwIndex = %u\n"),
		          pException->dwException,
		          pException->dwSendID,
		          pException->dwIndex);
		//m_EditOutput << sz;
	}
	break;
	}
	
#endif
}

//////////////////////////////////////////////////////////////////////////

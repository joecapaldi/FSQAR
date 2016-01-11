
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

// Debug version of CRT library uses this operator
#ifdef _DEBUG
#ifndef DEBUG_NEW
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new DEBUG_NEW
#endif
#endif  // _DEBUG

CRITICAL_SECTION g_csSimData;


//
// WARNING: This function works in the main application thread
//
VOID CALLBACK SimClientTimerProc( HWND hWnd, UINT uMsg, UINT_PTR idEvent,  DWORD dwTime)
{
	CSimClient* pSimClient = AfxGetApp()->GetSimClient();
	CSimConnect* pSimConnect = pSimClient->GetSimConnect();

	if (pSimConnect->IsUseTimer())
	{
		if (!pSimConnect->IsPaused())
		{
			pSimConnect->RequestDataOnUserObject(REQUEST_FLIGHT_DATA, 
				DEFINITION_FLIGHT_DATA);
		}
	}


	// All parameters are unreferenced
	UNREFERENCED_PARAMETER(dwTime);
	UNREFERENCED_PARAMETER(idEvent);
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(hWnd);
}

//////////////////////////////////////////////////////////////////////////

CSimConnect::CSimConnect()
{
	m_hCurrentResult = S_OK;
	m_fEndSession = 0;
	m_hSimConnect = NULL;
	m_fSimPause = TRUE;


	m_SimType = P3D30;
	// send_record
	m_SenderID = 0;
	StringCbCopyA(m_pAircraft.szName, SHORT_STR_LEN, "Demo"); // ANSI !!!


	m_pOutput = new CSimConnectOutput();
	m_pTimer = new CTimer();

	m_InternalData.onground = 0;
	m_InternalData.radio_height = 0;
	m_uFrequency = 250;
}



CSimConnect::~CSimConnect()
{
	TRACE0("DTOR CSimConnect\n");
	delete m_pOutput;
	delete m_pTimer;

}


BOOL CSimConnect::OpenConnection(LPCSTR pSessionName)
{
	ASSERT(AfxIsValidString(pSessionName));

#ifndef SIMCONNECT_DYNAMIC_LOAD
	m_hCurrentResult = SimConnect_Open(&m_hSimConnect, pSessionName, NULL, 0, 0, 0);
	CHECK_RESULT(m_hCurrentResult);
#endif // SIMCONNECT_DYNAMIC_LOAD

	if (m_hSimConnect == NULL)
	{
		return FALSE;
	}

		
	TRACE0("-------- Connection was opened ---------\n");
	TRACE1("SimConnect Handle: %#x\n", m_hSimConnect);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// You MUST fill this initializer
//
//////////////////////////////////////////////////////////////////////////

void CSimConnect::Initialize()
{
#ifndef SIMCONNECT_DYNAMIC_LOAD

	// NOTE: This function may be overloaded in the future
	// The first of all
	Subscribe(EVENT_START,		"SimStart");
	Subscribe(EVENT_STOP,		"SimStop");
	Subscribe(EVENT_PAUSE,		"Pause");
	Subscribe(EVENT_PAUSED,		"Paused");
	Subscribe(EVENT_UNPAUSED,	"Unpaused");
	// Internal timers of the simulator
	Subscribe(EVENT_ONE_SECOND,		"1sec");
	Subscribe(EVENT_FOUR_SECONDS,	"4sec");
	Subscribe(EVENT_6HZ,			"6Hz");
	// Request a notification when the aircraft flight dynamics file was changed.
	Subscribe(EVENT_AIR_LOADED, "AircraftLoaded");
	// Add user variables to the data definition
	//---------------------------------------------
	AddToData("SIM ON GROUND", "Bool");
	AddToData("Incidence Alpha", "Degrees");
	AddToData("Incidence Beta", "Degrees");
	AddToData("BETA DOT", "Degrees per second");
	AddToData("Airspeed True", "Knots");
	AddToData("Airspeed Mach", "Mach");
	AddToData("Vertical Speed", "ft/min");
	AddToData("G Force", "GForce");
	AddToData("Total Weight", "Pounds");
	AddToData("DYNAMIC PRESSURE", "psf");
	//---------------------------------------------
	// Aircraft Position and Speed Data
	//---------------------------------------------
	AddToData("Airspeed Indicated", "Knots");
	AddToData("PLANE LATITUDE", "degree latitude");
	AddToData("PLANE LONGITUDE", "degree longitude");
	AddToData("PLANE ALTITUDE", "Feet");
	AddToData("PLANE PITCH DEGREES", "Degrees");
	AddToData("PLANE BANK DEGREES", "Degrees");
	AddToData("PLANE HEADING DEGREES TRUE", "Degrees");
	AddToData("PLANE HEADING DEGREES MAGNETIC", "Degrees");
	AddToData("MAGVAR", "Degrees");
	AddToData("RADIO HEIGHT", "Feet");
	AddToData("GROUND VELOCITY", "Knots");
	AddToData("CG PERCENT", "Percent");
	AddToData("CG PERCENT LATERAL", "Percent");
	
	//-----------------------------------------------------
	AddToData("VELOCITY BODY X", "Feet per second");
	AddToData("VELOCITY BODY Y", "Feet per second");
	AddToData("VELOCITY BODY Z", "Feet per second");
	//-----------------------------------------------------
	AddToData("VELOCITY WORLD X", "Feet per second");
	AddToData("VELOCITY WORLD Y", "Feet per second");
	AddToData("VELOCITY WORLD Z", "Feet per second");
	//-----------------------------------------------------
	AddToData("ACCELERATION WORLD X", "Feet per second squared");
	AddToData("ACCELERATION WORLD Y", "Feet per second squared");
	AddToData("ACCELERATION WORLD Z", "Feet per second squared");
	//-----------------------------------------------------
	AddToData("ACCELERATION BODY X", "Feet per second squared");
	AddToData("ACCELERATION BODY Y", "Feet per second squared");
	AddToData("ACCELERATION BODY Z", "Feet per second squared");
	//-----------------------------------------------------
	AddToData("ROTATION VELOCITY BODY X", "Feet per second");
	AddToData("ROTATION VELOCITY BODY Y", "Feet per second");
	AddToData("ROTATION VELOCITY BODY Z", "Feet per second");
	//-----------------------------------------------------
	AddToData("RELATIVE WIND VELOCITY BODY X", "Feet per second");
	AddToData("RELATIVE WIND VELOCITY BODY Y", "Feet per second");
	AddToData("RELATIVE WIND VELOCITY BODY Z", "Feet per second");
	//-----------------------------------------------------
	// Aircraft Environment Data
	//-----------------------------------------------------
	AddToData("AMBIENT DENSITY", "Slugs per cubic feet");
	AddToData("AMBIENT TEMPERATURE", "Celsius");
	AddToData("STANDARD ATM TEMPERATURE", "Celsius");
	AddToData("TOTAL AIR TEMPERATURE", "Celsius");

	AddToData("AMBIENT WIND VELOCITY", "Knots");
	AddToData("AMBIENT WIND DIRECTION", "Degrees");

	AddToData("AMBIENT WIND X", "m/s");
	AddToData("AMBIENT WIND Y", "m/s");
	AddToData("AMBIENT WIND Z", "m/s");

	AddToData("AIRCRAFT WIND X", "Knots");
	AddToData("AIRCRAFT WIND Y", "Knots");
	AddToData("AIRCRAFT WIND Z", "Knots");

	AddToData("AMBIENT PRESSURE", "inHg");
	AddToData("BAROMETER PRESSURE", "inHg");
	AddToData("SEA LEVEL PRESSURE", "inHg");
	AddToData("AMBIENT VISIBILITY", "km");

	// Aircraft Engine Data
	AddToData("GENERAL ENG RPM:1", "rpm");
	AddToData("GENERAL ENG THROTTLE LEVER POSITION:1", "Percent");
	AddToData("GENERAL ENG EXHAUST GAS TEMPERATURE:1", "Celsius");
	AddToData("GENERAL ENG OIL PRESSURE:1", "psf");
	AddToData("GENERAL ENG OIL TEMPERATURE:1", "Celsius");
	AddToData("GENERAL ENG FUEL PRESSURE:1", "psf");
	AddToData("GENERAL ENG ELAPSED TIME:1", "Hour");
	AddToData("TURB ENG N1:1", "Percent");
	AddToData("TURB ENG N2:1", "Percent");
	AddToData("TURB ENG CORRECTED N1:1", "Percent");
	AddToData("TURB ENG CORRECTED N2:1", "Percent");
	AddToData("TURB ENG CORRECTED FF:1", "Pounds per hour");
	//---------------------------------------------------------
	AddToData("TURB ENG MAX TORQUE PERCENT:1", "Percent");
	AddToData("TURB ENG PRESSURE RATIO:1", "Ratio");
	AddToData("TURB ENG ITT:1", "Celsius");
	AddToData("TURB ENG JET THRUST:1", "Pounds");
	AddToData("TURB ENG BLEED AIR:1", "psi");
	AddToData("TURB ENG FUEL FLOW PPH:1", "Pounds per hour");
	AddToData("TURB ENG VIBRATION:1", "Number");
	AddToData("TURB ENG REVERSE NOZZLE PERCENT:1", "Percent");

	// Flight Controls
	AddToData("ELEVATOR DEFLECTION", "Degrees");
	AddToData("ELEVATOR DEFLECTION PCT", "Percent");
	AddToData("AILERON LEFT DEFLECTION", "Degrees");
	AddToData("AILERON LEFT DEFLECTION PCT", "Percent");
	AddToData("RUDDER DEFLECTION", "Degrees");
	AddToData("RUDDER DEFLECTION PCT", "Percent");
	//----------------------------------------------
	AddToData("ELEVATOR TRIM PCT", "Percent");
	AddToData("AILERON TRIM PCT", "Percent");
	AddToData("RUDDER TRIM PCT", "Percent");
	//----------------------------------------------
	AddToData("PROP RPM:1", "rpm");
	AddToData("PROP MAX RPM PERCENT:1", "Percent");
	AddToData("PROP BETA:1", "Degrees");
	AddToData("PROP THRUST:1", "Pounds");



	// ========================================================	
	
	::SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_AIRCRAFT_NAME, 
		"ATC TYPE", NULL, SIMCONNECT_DATATYPE_STRING64);
	// ========================================================
	
	::SimConnect_AddToDataDefinition(m_hSimConnect, 
		DEFINITION_CONTROLS, "ELEVATOR POSITION", "Position");

	::SimConnect_AddToDataDefinition(m_hSimConnect, 
		DEFINITION_CONTROLS, "AILERON POSITION", "Position");

	::SimConnect_AddToDataDefinition(m_hSimConnect, 
		DEFINITION_CONTROLS, "RUDDER POSITION", "Position");

	::SimConnect_AddToDataDefinition(m_hSimConnect, 
		DEFINITION_CONTROLS, "GENERAL ENG THROTTLE LEVER POSITION:1", 
		"Percent");

	::SimConnect_AddToDataDefinition(m_hSimConnect, 
		DEFINITION_CONTROLS, "GENERAL ENG THROTTLE LEVER POSITION:2", 
		"Percent");

	::SimConnect_AddToDataDefinition(m_hSimConnect, 
		DEFINITION_CONTROLS, "GENERAL ENG THROTTLE LEVER POSITION:3", 
		"Percent");

	::SimConnect_AddToDataDefinition(m_hSimConnect, 
		DEFINITION_CONTROLS, "GENERAL ENG THROTTLE LEVER POSITION:4", 
		"Percent");
	// ========================================================

	::SimConnect_AddToDataDefinition(m_hSimConnect, 
		DEFINITION_RADIO_HEIGHT, "RADIO HEIGHT", "Feet");

#endif // SIMCONNECT_DYNAMIC_LOAD
}

void CSimConnect::CallDispatch(DispatchProc Proc, CSimConnect* pSimConnect)
{
	ASSERT(pSimConnect);
	ASSERT(AfxIsValidAddress(m_hSimConnect, sizeof(HANDLE)));

#ifndef SIMCONNECT_DYNAMIC_LOAD
	::SimConnect_CallDispatch(m_hSimConnect, Proc, static_cast<void*>(pSimConnect));
#endif // #ifndef SIMCONNECT_DYNAMIC_LOAD
}

void CSimConnect::CloseConnection()
{
	ASSERT(AfxIsValidAddress(m_hSimConnect, sizeof(HANDLE)));

#ifndef SIMCONNECT_DYNAMIC_LOAD
	m_hCurrentResult = SimConnect_Close(m_hSimConnect);
	CHECK_RESULT(m_hCurrentResult);
#endif // #ifndef SIMCONNECT_DYNAMIC_LOAD

	TRACE0("-------- Connection was closed ---------\n");
	
	// CloseConnection the output file if it was opened
	if (m_Options.IsPrintFile())
	{
		m_pOutput->CloseFile();
		TRACE0("Output file was closed\n");
	}

	m_Options.SaveOptions();
}

void CSimConnect::EventPause(SIMCONNECT_RECV_EVENT* evt)
{
	Lock lock(&g_csSimData);
	m_fSimPause = evt->dwData;
	lock.Release();

	if (m_fSimPause)
	{
		TCHAR szText[LONG_STR_LEN];
		StringCbPrintf(szText, LONG_STR_LEN, _T("Simulation paused..."));
		ShowStatus(0, szText);
	}
	else
	{
		TCHAR szText[LONG_STR_LEN];
		StringCbPrintf(szText, LONG_STR_LEN, _T("Simulation continue..."));
		ShowStatus(0, szText);
	}
}

void CSimConnect::EventPaused()
{
	// TODO: Add 'Paused' event handler
	// Currently the simulator on pause.
	// m_fSimPause was set to TRUE
}

void CSimConnect::EventUnpaused()
{
	// TODO: Add 'Unpaused' event handler
	// Currently the simulator on pause.
	// m_fSimPause was set to FALSE
}

void CSimConnect::EventStart()
{
	// TODO: Add 'Start' event handler
	// The simulator is running.
	// The user is controlling the SimObject.
}

void CSimConnect::EventStop()
{
	// TODO: Add 'Stop' event handler
	// The simulator is not running.
	// Typically the user is loading a flight,
	// navigating the shell or in a dialog.
}



void CSimConnect::EventOpen(SIMCONNECT_RECV_OPEN* pData)
{

	TRACE0("SimConnect event 'OpenConnection'\n");

	// NOTE: This function may be overloaded in the future
	TCHAR szBuffer[256];
	StringCbPrintf(szBuffer, 256,
	          _T("FSQAR - %S (%u.%u.%u.%u)  SimConnect: %u.%u.%u.%u"),
	          pData->szApplicationName,
	          pData->dwApplicationVersionMajor,
	          pData->dwApplicationVersionMinor,
	          pData->dwApplicationBuildMajor,
	          pData->dwApplicationBuildMinor,
	          pData->dwSimConnectVersionMajor,
	          pData->dwSimConnectVersionMinor,
	          pData->dwSimConnectBuildMajor,
	          pData->dwSimConnectBuildMinor);

	AfxGetApp()->SetCaption(szBuffer);

	TRACE1("**** Running on  %s  ****\n",	szBuffer);
	if (strstr(pData->szApplicationName, "Microsoft") != NULL)
		m_SimType = FSX;
	else
	{
		m_SimType = static_cast<SIMTYPE>(pData->dwApplicationVersionMajor);
	}


	CreateOutputFolder();
	ReadOptions(&m_Options);

	RequestDataOnUserObject(REQUEST_AIRCRAFT_NAME, DEFINITION_AIRCRAFT_NAME);

}

void CSimConnect::CreateOutputFolder()
{
	ASSERT(m_pOutput);
	__analysis_assume(m_pOutput);

	switch (m_SimType)
	{
	case FSX:
		if (!m_pOutput->CreateOutputFolder(_T("Flight Simulator X Files")))
		{
			AfxGetApp()->Error(IDS_CANNOT_CREATE_FOLDER);
			TRACE0("CreateOutputFolder failed\n");
		}
		break;

	case P3D14:
		if (!m_pOutput->CreateOutputFolder(_T("Prepar3D Files")))
		{
			AfxGetApp()->Error(IDS_CANNOT_CREATE_FOLDER);
			TRACE0("CreateOutputFolder failed\n");
		}
		break;

	case P3D20:
		if (!m_pOutput->CreateOutputFolder(_T("Prepar3D v2 Files")))
		{
			AfxGetApp()->Error(IDS_CANNOT_CREATE_FOLDER);
			TRACE0("CreateOutputFolder failed\n");
		}
		break;

	case P3D30:
		if (!m_pOutput->CreateOutputFolder(_T("Prepar3D v3 Files")))
		{
			AfxGetApp()->Error(IDS_CANNOT_CREATE_FOLDER);
			TRACE0("CreateOutputFolder failed\n");
		}
		break;

	case P3D40:
		if (!m_pOutput->CreateOutputFolder(_T("Prepar3D v4 Files")))
		{
			AfxGetApp()->Error(IDS_CANNOT_CREATE_FOLDER);
			TRACE0("CreateOutputFolder failed\n");
		}
		break;

	default:
		break;
	}
}

void CSimConnect::RequestDataOnSimObjectType(SIMCONNECT_DATA_REQUEST_ID RequestID, SIMCONNECT_DATA_DEFINITION_ID DefineID)
{
	ASSERT(AfxIsValidAddress(m_hSimConnect, sizeof(HANDLE)));

#ifndef SIMCONNECT_DYNAMIC_LOAD
	m_hCurrentResult = SimConnect_RequestDataOnSimObjectType(m_hSimConnect,
	          RequestID, DefineID, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
	CHECK_RESULT(m_hCurrentResult);
#endif // #ifndef SIMCONNECT_DYNAMIC_LOAD

}



void CSimConnect::RequestDataOnUserObject(SIMCONNECT_DATA_REQUEST_ID RequestID, SIMCONNECT_DATA_DEFINITION_ID DefineID)
{
	ASSERT(AfxIsValidAddress(m_hSimConnect, sizeof(HANDLE)));

	// We should protect the m_hCurrentResult variable
	// to prevent RequestData() <=> SetData() data-racing
#ifndef SIMCONNECT_DYNAMIC_LOAD
	Lock lock(&g_csSimData);
	m_hCurrentResult = SimConnect_RequestDataOnSimObject(m_hSimConnect,
	          RequestID, DefineID,
	          SIMCONNECT_SIMOBJECT_TYPE_USER,
	          SIMCONNECT_PERIOD_ONCE);
	
	CHECK_RESULT(m_hCurrentResult);

	lock.Release();
#endif // #ifndef SIMCONNECT_DYNAMIC_LOAD

}

void CSimConnect::Event6Hz()
{
	if (m_Options.IsUseSchedule())
	{
		if (m_InternalData.radio_height < 100)
		{
			m_uFrequency = 166;
			if (!m_fSimPause)
				RequestDataOnUserObject(REQUEST_FLIGHT_DATA, DEFINITION_FLIGHT_DATA);
		}
	}
}

void CSimConnect::EventOneSecond()
{
	// Every time request
	if (!m_fSimPause)
	{
		RequestDataOnUserObject(REQUEST_RADIO_HEIGHT, DEFINITION_RADIO_HEIGHT);	
	}

	// Scheduled request
	if (m_Options.IsUseSchedule())
	{
		if ((m_InternalData.radio_height >= 100) &&
			(m_InternalData.radio_height <= 2000))
		{
			m_uFrequency = 1000;
			if (!m_fSimPause)
				RequestDataOnUserObject(REQUEST_FLIGHT_DATA, DEFINITION_FLIGHT_DATA);
		}
	}
}

void CSimConnect::EventFourSecond()
{
	// Scheduled request
	if (m_Options.IsUseSchedule())
	{
		if (m_InternalData.radio_height > 2000)
		{
			m_uFrequency = 4000;
			if (!m_fSimPause)
				RequestDataOnUserObject(REQUEST_FLIGHT_DATA, DEFINITION_FLIGHT_DATA);
		}
	}
}

void CSimConnect::EventFrame()
{
	// TODO: Add 'Frame' handler
	// This event occurs when the visual frame is updated.
	// Information about this event is returned in
	// a SIMCONNECT_RECV_EVENT_FRAME structure.
}

void CSimConnect::EventQuit()
{
	m_pOutput->CloseFile();
	m_Options.SaveOptions();
	m_fEndSession = TRUE;
}

void CSimConnect::EventPositionChanged()
{
	// TODO: Add 'PositionChanged' event handler
	// This event occurs when the user changes the position
	// of the SimObject through a dialog.
}



//------------------------------------------------------------------------
// UpdateFlightData is called every time when the simulator data are updated
//
// pArrayAddress	A data array containing information on a specified object
//					in 8-byte (double word) elements.
//
// dwArrayCount		The number of 8-byte elements in the pArrayAddress array.
//
//------------------------------------------------------------------------
void CSimConnect::UpdateFlightData(LPVOID pArrayAddress,  DWORD_PTR dwArrayCount)
{
	//
	// This is required operation. Flight data array must be initialized.
	// Get pointer to the flight data array from the pArrayAddress.
	//
	PDOUBLE_VARIABLES_ARRAY pDA = static_cast<PDOUBLE_VARIABLES_ARRAY>(pArrayAddress);
	

	Lock lock(&g_csSimData);

	static SimData* pVariables = SimData::GetSimData();
	if (dwArrayCount <= SIM_VARS_COUNT)
	{
		// CORRECT SIGN '<' NOT '<=' !!!
		for (UINT_PTR nPosition = 0; nPosition < dwArrayCount; nPosition++)
		{
			if (((*pDA)[nPosition] < MIN_DOUBLE) && ((*pDA)[nPosition] > -MIN_DOUBLE))
				(*pDA)[nPosition] = 0;

			pVariables->SetValue(nPosition, (*pDA)[nPosition]);
		}
		pVariables->SetArraySize(dwArrayCount);
	}

	// We should lock the global variable g_GraphData
	// to prevent data-racing

	// InterlockedExchangeAdd requires a pointer as the first argument
	volatile LONG nSize = g_GraphData.GetSize();
	for (INT_PTR nLine = 0; nLine < MAX_CHART_COUNT; nLine++)
	{
		INT_PTR nWhat = g_GraphData.GetSelectedItem(nLine); 
		double dblValue = pVariables->GetVariable(nWhat)->Value;

		g_GraphData.SetBufferValue(nLine, nSize, dblValue);

	}

	// See Richter (p. 209): we should not use g_GraphData.m_nSize++;
	// member-function must be called to get address of the variable
	InterlockedExchangeAdd(g_GraphData.GetAddress(), 1);

	lock.Release();

	// It has internal crit.section protection
	AfxGetApp()->UpdateAllDisplays();
	

	if (m_Options.IsPrintFile())
	{
		m_pOutput->PrintData(pVariables);

		TCHAR szText[LONG_STR_LEN];
		StringCbPrintf(szText, LONG_STR_LEN, 
			_T("Records:  %u, Freq: %u times per minute"), 
			m_pOutput->GetRecordsCount(),
			60000/GetFrequency());

		ShowStatus(2, szText);
	}

	
	SimData::FreeSimData();


}



// ==================================================================================
// The description of szState parameter
//
// 'AircraftLoaded' Requests the full path name of the last loaded aircraft AIR-file.
//
// 'DialogMode'		Requests whether the simulation is in Dialog mode or not.
//
// 'FlightLoaded'	Requests the full path name of the last loaded flight.
//
// 'FlightPlan'		Requests the full path name of the active flight plan.
//
// 'Sim'			Requests the state of the simulation. If 1 is returned,
//				the user is in control of the aircraft, if 0 is returned,
//				the user is navigating the UI.
// ==================================================================================
void CSimConnect::RequestSystemState(SIMCONNECT_DATA_REQUEST_ID  RequestID,  const char*  szState)
{
	ASSERT(AfxIsValidAddress(m_hSimConnect, sizeof(HANDLE)));

#ifndef SIMCONNECT_DYNAMIC_LOAD
	m_hCurrentResult = SimConnect_RequestSystemState(m_hSimConnect, 
		RequestID, szState);
	CHECK_RESULT(m_hCurrentResult);
#endif // #ifndef SIMCONNECT_DYNAMIC_LOAD
}


void CSimConnect::UpdateAircraftName(LPVOID pArrayAddress, DWORD_PTR dwArrayCount)
{
	// Get pointer to returned data
	AIRCRAFT_NAME* pData = static_cast<AIRCRAFT_NAME*>(pArrayAddress);

	TRACE0("Static data recievied\n");
	
	// Aircraft model was changed
	// Reopen output file

	if (pData)
	{
		StringCchCopyA(m_pAircraft.szName, SHORT_STR_LEN, pData->szName);

		TRACE0("Aircraft model was changed\n");
		TRACE1("Aircraft name: %S\n", m_pAircraft.szName);

		if (m_Options.IsPrintFile())
		{
			m_pOutput->SetDatafileSeparator(m_Options.GetSeparator());
			CreateOutputFile();
		}
	}


	UNREFERENCED_PARAMETER(dwArrayCount);
}

void CSimConnect::CreateOutputFile()
{
	ASSERT(m_pOutput);
	__analysis_assume(m_pOutput);

	static SimData* pVars = SimData::GetSimData();
	m_pOutput->CreateOutputFile(m_pAircraft.szName, pVars);
	SimData::FreeSimData();

	ASSERT(AfxIsValidString(m_pOutput->GetFileName()));

	if (m_Options.IsPrintFile())
	{
		TCHAR szText[LONG_STR_LEN];
		StringCbPrintf(szText, LONG_STR_LEN, 
			_T("Output file: %s.csv (compressed)"), m_pOutput->GetFileName());

		ShowStatus(1, szText);
	}
	else
	{
		TCHAR szText[SHORT_STR_LEN];
		if (MyLoadString(IDS_NO_PRINT_FILE, szText, SHORT_STR_LEN))
			ShowStatus(1, szText);
	}
}

//------------------------------------------------------------------------
// This function is called when the aircraft flight dynamics file is changed
// These files have a .AIR extension. The filename is returned in 
// a pszPathName parameter
//------------------------------------------------------------------------
void CSimConnect::AIRFileLoaded(LPCSTR pszPathName)
{
	// User have loaded a new aircraft
	TRACE0("Event: New AIR-file was loaded\n");
	TRACE1("AIR-file: %S\n", pszPathName);

#ifndef _DEBUG
	// Reserved
	UNREFERENCED_PARAMETER(pszPathName);
#endif

	// Send request to determine the name of the aircraft model
	RequestDataOnUserObject(REQUEST_AIRCRAFT_NAME, DEFINITION_AIRCRAFT_NAME);
	TRACE0("Request internal data...\n");
}


VOID CSimConnect::ShowStatus(INT nPart, LPCTSTR pMessage)
{
	// This function may be redefined for different output contexts
	// For example: Find status window and send the message to it
	AfxGetApp()->GetStatus()->ShowStatus(nPart, pMessage);
}

void CSimConnect::AssertValid()
{
	ASSERT(m_hSimConnect);
}

void CSimConnect::SetDataOnSimObject(SIMCONNECT_DATA_DEFINITION_ID  DefineID, DWORD  ArrayCount, DWORD  cbUnitSize, void*  pDataSet)
{
	ASSERT(AfxIsValidAddress(m_hSimConnect, sizeof(HANDLE)));

#ifndef SIMCONNECT_DYNAMIC_LOAD
	// RequestData <=> SetData racing
	Lock lock(&g_csSimData);

	m_hCurrentResult = SimConnect_SetDataOnSimObject(
		m_hSimConnect,
		DefineID,
		SIMCONNECT_OBJECT_ID_USER,
		SIMCONNECT_DATA_SET_FLAG_DEFAULT,
		ArrayCount,
		cbUnitSize,
		pDataSet);
	CHECK_RESULT(m_hCurrentResult);

	lock.Release();

#endif // #ifndef SIMCONNECT_DYNAMIC_LOAD
}

void CSimConnect::MapClientEventToSimEvent(SIMCONNECT_CLIENT_EVENT_ID  EventID, const char*  EventName)
{

	ASSERT(AfxIsValidAddress(m_hSimConnect, sizeof(HANDLE)));

#ifndef SIMCONNECT_DYNAMIC_LOAD

	Lock lock(&g_csSimData);

	m_hCurrentResult = SimConnect_MapClientEventToSimEvent(m_hSimConnect, 
		EventID, EventName);
	CHECK_RESULT(m_hCurrentResult);

	lock.Release();

#endif // #ifndef SIMCONNECT_DYNAMIC_LOAD
}

void CSimConnect::TransmitClientEvent(SIMCONNECT_CLIENT_EVENT_ID EventID,  DWORD dwData)
{


	ASSERT(AfxIsValidAddress(m_hSimConnect, sizeof(HANDLE)));
	
#ifndef SIMCONNECT_DYNAMIC_LOAD
	Lock lock(&g_csSimData);
	m_hCurrentResult = SimConnect_TransmitClientEvent(m_hSimConnect,
		SIMCONNECT_OBJECT_ID_USER,
		EventID,
		dwData,
		SIMCONNECT_GROUP_PRIORITY_DEFAULT,
		SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	CHECK_RESULT(m_hCurrentResult);

	lock.Release();
#endif // SIMCONNECT_DYNAMIC_LOAD
}

CTimer::CTimer(UINT nElapse)
{
	m_uFrequency = nElapse;
	m_nID = SetTimer(NULL, 0, m_uFrequency, (TIMERPROC)SimClientTimerProc);
}

CTimer::~CTimer()
{
	if (m_nID)
		KillTimer(NULL, m_nID);
}



void CSimConnect::UpdateRadioHeight(LPVOID pArrayAddress,  DWORD_PTR dwArrayCount)
{
	RADIO_HEIGHT* pData = static_cast<RADIO_HEIGHT*>(pArrayAddress);
	if (pData)
		m_InternalData.radio_height = static_cast<long>(pData->radio_height);

	UNREFERENCED_PARAMETER(dwArrayCount);
}

void CSimConnect::TimerReset(UINT nNewElapse)
{
	ASSERT(nNewElapse != 0);

	if (m_pTimer)
	{
		delete m_pTimer;
		m_pTimer = NULL;
	}

	m_pTimer = new CTimer(nNewElapse);
}

void CSimConnect::GetOptions(SIMCLIENT_OPTIONS* pSimOptions)
{
	ASSERT(pSimOptions);

#pragma warning(suppress: 6001)		// I don't know why !!!
	m_Options.GetOptions(pSimOptions);

}

void CSimConnect::SetOptions(const SIMCLIENT_OPTIONS* pSimOptions)
{
	ASSERT(pSimOptions);
	m_Options.SetOptions(pSimOptions);
}

void CSimConnect::ReadOptions(CSimClientOptions* pSimConnectOpt)
{
	pSimConnectOpt->ReadOptions();
}

void CSimConnect::SaveOptions(CSimClientOptions& SimConnectOpt)
{
	SimConnectOpt.SaveOptions();
}

UINT CSimConnect::GetFrequency()
{
	UINT freq = 1;

	if (m_Options.IsUseTimer())
	{
		freq = m_pTimer->GetFrequency();
	}
	else
	{
		freq = m_uFrequency;
	}
	return freq;
}


void CSimConnect::SendDataToSimulator(DWORD dwDataDefinition,  DWORD dwSize, void* pDataSet)
{
	ASSERT(pDataSet);
	SetDataOnSimObject(dwDataDefinition, 0, dwSize, pDataSet);
}

void CSimConnect::SetPause(BOOL fPause)
{
	if (fPause)
	{
		if (IsRunning())
		{
			SIMCONNECT_CLIENT_EVENT_ID EventID = 63542;
			MapClientEventToSimEvent(EventID, "PAUSE_ON");
			TransmitClientEvent(EventID, 1);
		}
	} 
	else
	{
		if (IsRunning())
		{
			SIMCONNECT_CLIENT_EVENT_ID EventID = 63543;
			MapClientEventToSimEvent(EventID, "PAUSE_OFF");
			TransmitClientEvent(EventID, 1);
		}
	}

}

//////////////////////////////////////////////////////////////////////////

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


#ifndef __SIMCLIENT_H__
#define __SIMCLIENT_H__



//========================================================================
//				These are SimConnect data structures
//========================================================================

static enum EVENT_ID
{
	EVENT_START,
	EVENT_STOP,
	EVENT_PAUSE,
	EVENT_PAUSED,
	EVENT_UNPAUSED,
	EVENT_ONE_SECOND,
	EVENT_FOUR_SECONDS,
	EVENT_6HZ,
	EVENT_AIR_LOADED,
	EVENT_A,
	EVENT_Z
	
};

static enum DATA_DEFINE_ID
{
	DEFINITION_FLIGHT_DATA,
	DEFINITION_AIRCRAFT_NAME,
	DEFINITION_CONTROLS,
	DEFINITION_RADIO_HEIGHT
};


//========================================================================
//				The height is used in internal schedule
//========================================================================

typedef struct _RADIO_HEIGHT
{
	double radio_height;

}RADIO_HEIGHT;

//========================================================================
//				The name of aircraft is used as a file name
//========================================================================

typedef struct _AIRCRAFT_NAME
{
	char szName[SHORT_STR_LEN];	// ANSI string !!!

} AIRCRAFT_NAME;



static enum DATA_REQUEST_ID
{
	REQUEST_FLIGHT_DATA,
	REQUEST_AIRCRAFT_NAME,
	REQUEST_CONTROLS_POS,
	REQUEST_RADIO_HEIGHT
};


//========================================================================
//				Aircraft controls
//========================================================================

typedef struct _CONTROLS_DEFLECTION 
{
	double elevator;
	double aileron;
	double rudder;
	double lever1;
	double lever2;
	double lever3;
	double lever4;

}CONTROLS_DEFLECTION;


//========================================================================
//				Abstract class for string output
//========================================================================

class COutput
{
public:
	COutput();
	virtual ~COutput();
	
	// Must be implemented in successors
	virtual void OutputString(LPCTSTR lpsz) = 0;   // do nothing
	
	virtual COutput& operator<<(__in_z LPCTSTR lpsz);
#ifdef _UNICODE
	COutput& operator<<(__in_z LPCSTR lpsz);    // automatically widened
#else
	COutput& operator<<(__in_z LPCWSTR lpsz);   // automatically thinned
#endif
	
#ifdef _UNICODE
	COutput& operator<<(char ch);    // automatically widened
#else
	COutput& operator<<(wchat_t wc);   // automatically thinned
#endif

	COutput& operator<<(int n);
	COutput& operator<<(float f);
	COutput& operator<<(double d);

protected:
	DISABLE_COPY_AND_ASSIGN(COutput);
};

//========================================================================
//				The simplest file class
//========================================================================

class CFile
{
public:
	CFile();
	virtual ~CFile();

	BOOL Open(LPCTSTR pszFileName, BOOL bFileExist);
	void Close();
	DWORD GetSize();
	BOOL Write(LPCVOID lpBuffer, size_t dwBufferSize);
	BOOL Read(LPVOID lpBuffer, DWORD dwBufferSize);

private:
	HANDLE m_hFile;

protected:
	DISABLE_COPY_AND_ASSIGN(CFile);
};

inline void CFile::Close()
{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);
	::CloseHandle(m_hFile);
}

inline DWORD CFile::GetSize()
{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);
	return ::GetFileSize(m_hFile, NULL);
}


//========================================================================
//				Some output to a disk file
//========================================================================

class SimData;
class CFileOutput : public COutput, public CFile
{
public:
	CFileOutput() :
		m_fFileOpened(FALSE),
		m_nDataSeparator(DATAFILE_SEPARATOR_COMMA),
		m_chSeparator(';')
	{
	}
	BOOL IsFileOpened() { return m_fFileOpened; }

	// Must be implemented
	void OutputString(LPCTSTR lpsz)
	{
		size_t cbSize;
		if (SUCCEEDED(StringCbLength(lpsz, MAX_PATH, &cbSize)))
			Write(lpsz, cbSize);
	}

	void SetSeparator(INT nSeparator);
	INT GetSeparator() { return m_nDataSeparator; };
	void PrintFile(const SimData* pSimData, SIZE_T nDataSize);
	void PrintHeader(const SimData* pSimData, SIZE_T nDataSize);
	__checkReturn BOOL OpenFile(__in_z LPCTSTR pszFileName);
	
protected:
	DISABLE_COPY_AND_ASSIGN(CFileOutput);

private:
	BOOL m_fFileOpened;
	INT m_nDataSeparator;
	CHAR m_chSeparator;
};


//========================================================================
//				One of simulator variables
//========================================================================

#define MIN_DOUBLE	0.01

typedef struct _SIMVARIABLE
{
	const char* DatumName;	// Name of the simulation variable
	const char* UnitsName;	// Units of the variable
	double Value;			// Value of the variable
}SIMVARIABLE;


//========================================================================
//				These are the simulator variables ID
//========================================================================

enum SIM_VARIABLES
{
	ONGROUND,
	ALPHA,	// Alpha
	BETA,	// Beta
	BETADOT,// Beta dot
	TAS,	// True airspeed
	MACH,	// Mach number
	VSPEED,	// Vertical speed
	GFORCE,	// G-Force
	WEIGHT,	// Current total weight
	DYNRESS,
	//---------------------------
	IAS,	// Indicated airspeed
	LAT,	// Latitude
	LON,	// Longitude
	ALT,	// Altitude
	PITCH,	// Pitch
	BANK,	// Bank
	HTRUE,	// True heading
	HMAG,	// Magnetic heading
	MAGVAR,	// Current magnetic variation
	HEIGHT,	// Radio height
	GSPEED,	// Ground speed
	CGLON,	// CG longitudinal pos.
	CGLAT,	// CG lateral pos.
	//------------------------
	VBX,	// Velocity body X
	VBY,	// Velocity body Y
	VBZ,	// Velocity body Z
	//-------------------------
	VWX,	// Velocity world X
	VWY,	// Velocity world Y
	VWZ,	// Velocity world Z
	//------------------------------
	AWX,	// Acceleration world X
	AWY,	// Acceleration world Y
	AWZ,	// Acceleration world Z
	//-----------------------------
	ABX,	// Acceleration body X
	ABY,	// Acceleration body Y
	ABZ,	// Acceleration body Z
	//------------------------------
	RBX,	// Rotation relative to body axis X
	RBY,	// Rotation relative to body axis Y
	RBZ,	// Rotation relative to body axis Z
	//-----------------------------------------
	WINDX,	// Lateral speed relative to wind
	WINDY,	// Vertical speed relative to wind
	WINDZ,	// Longitudinal speed relative to wind
	//--------------------------------------------
	AMBDENS,	// Ambient air density 
	AMBTEMP,	// Ambient air temperature
	STDTEMP,	// ISA air temperature
	TAT,		// Total air temperature
	//--------------------------------------------
	AWVEL,		// Ambient wind velocity
	AWDIR,		// Ambient wind direction (true)
	AWINDX,		// Ambient wind X-velocity
	AWINDY,		// Ambient wind Y-velocity
	AWINDZ,		// Ambient wind Z-velocity
	//--------------------------------------------
	ACWINDX,	// A/C wind X-velocity
	ACWINDY,	// A/C wind Y-velocity
	ACWINDZ,	// A/C wind Z-velocity
	//--------------------------------------------
	AMBPRESS,	// Ambient air pressure
	BAROPRESS,	// Barometric pressure
	SLPRESS,	// Sea level pressure
	AMBVIS,		// Ambient visibility
	//--------------------------------------------
	RRM,		// Engine rotor RPM
	LEVER,		// Lever position
	EGT,		// EGT
	OILPRESS,	// Oil pressure
	OILTEMP,	// Oil temperature
	FUEL,		// Fuel flow
	TIME,		// Engine worked time
	//--------------------------------------------
	N1,			// Engine N1
	N2,			// Engine N2
	CORRN1,		// Engine corrected N1
	CORRN2,		// Engine corrected N2
	CORRFF,		// Engine corrected fuel flow
	//--------------------------------------------
	TORQUE,		// Engine rotor torque
	EPR,		// Engine EPR
	ITT,		// Turbine interstage temperature
	THRUST,		// Total thrust per one engine
	BLEED,		// Air bleed
	FF,			// Fuel flow ???
	VIB,		// Engine vibration
	REVERS,		// Revers percent
	//--------------------------------------------
	EDEF,		// Elevator deflection in degrees
	EPCT,		// Percent of elevator deflection
	ADEF,		// Aileron deflection in degrees
	APCT,		// Percent of aileron deflection
	RDEF,		// Rudder deflection in degrees
	RPCT,		// Percent of rudder deflection
	ETRIM,		// Percent of elevator trim deflection
	ATRIM,		// Percent of aileron trim deflection
	RTRIM,		// Percent of rudder trim deflection
	//--------------------------------------------
	PROPRPM,	// Propeller rpm
	PROPRPMPCT,	// Percent of max rated rpm
	PROPBETA,	// Prop blade pitch angle
	PROPTHRUST,	// Propeller thrust
	//--------------------------------------------
	END_VARIABLES		// must be the last

};

// Total number of requested variables
#define SIM_VARS_COUNT	END_VARIABLES


// This pointer is returned in dwData of SIMCONNECT_RECV_SIMOBJECT_DATA structure
typedef double(*PDOUBLE_VARIABLES_ARRAY)[SIM_VARS_COUNT];

//========================================================================
//				SimConnect Thread
//========================================================================

class CSimThread : public CObject
{
public:
	CSimThread();
	~CSimThread();

	static const DWORD TERMINATION_TIMEOUT = 125; // It's time to wait in ms

	BOOL CreateThread(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter);
	BOOL ExitThread();

	void SendSignal()   
	{ 
		ASSERT(m_hThread); 
		::SetEvent(m_hEventExit); 
	}

	BOOL IsTerminated() 
	{ 
		ASSERT(m_hThread); 
		return ((::WaitForSingleObject(m_hEventExit, 0) 
			== WAIT_OBJECT_0) ? TRUE : FALSE); 
	}

	void AssertValid()
	{
		ASSERT(m_hThread);
		ASSERT(m_hEventExit);
	}

private:
	HANDLE m_hThread;
	HANDLE m_hEventExit;
};


//=======================================================================
//				These are used in SimConnect exception
//=======================================================================

#define MAX_EXCEPTION_RECORDS        256

// Declare a structure to hold the send IDs and identification strings
struct SIMCONNECT_DATA_SENDER
{
	char  szSenderName[256];
	DWORD dwSenderID;
};

//=======================================================================
//				These are known flight simulators
//=======================================================================

enum SIMTYPE
{
	FSX,
	P3D14,
	P3D20,
	P3D30,
	P3D40
};

//=======================================================================
//				Output from SimConnect to a file
//=======================================================================

class CSimConnectOutput
{
public:
	CSimConnectOutput();
	virtual ~CSimConnectOutput();

	BOOL CreateOutputFolder(LPCTSTR pszSimFolder);

	BOOL DoOpenFile(const char* pAircraftName);
	void CloseFile();
	void CreateOutputFile(const char* pszFileName, const SimData* pSimData);
	void PrintData(const SimData* pSimData);
	LPCTSTR GetFileName() { return m_pszCurrentFileName; }
	UINT GetRecordsCount() { return m_nRecordsCount; }
	void SetDatafileSeparator(INT nSeparator);

	
protected:

	DISABLE_COPY_AND_ASSIGN(CSimConnectOutput);

private:

	BOOL ZipOutputFile();
	BOOL PrepareArchive(LPTSTR pszArchivePath, LPTSTR pszFilePath);
	BOOL AnsiFileName( LPSTR pszAnsiName);
	BOOL DoCompression(LPCTSTR pszFilePath,  LPCTSTR pszArchivePath, LPCSTR pszANSIName);
	void MakeFullPath(LPTSTR pOutputFileName, size_t nSize, LPCTSTR pAircraftName);
	BOOL ClearOutputFolder();

	BOOL m_fDefaultFileName;
	BOOL m_fZipOutputFile;
	UINT m_nRecordsCount;

	CFileOutput* m_pFileOutput;
	LPTSTR m_pszOutputFolder;
	LPTSTR m_pszCurrentFileName;
};


//=======================================================================
//				The simplest system timer class
//=======================================================================

#define ID_SIMCONNECT_TIMER		4853

class CTimer
{
public:
	CTimer(UINT nElapse = 250);
	virtual ~CTimer();
	UINT GetFrequency() { return m_uFrequency; }

protected:
	DISABLE_COPY_AND_ASSIGN(CTimer);

private:
	UINT m_uFrequency;
	UINT_PTR m_nID;

};

//=======================================================================
//				This structure is used in dialog
//=======================================================================

typedef struct _SIMCLIENT_OPTIONS
{
	BOOL fUseSchedule;
	BOOL fUseTimer;
	BOOL fPrintFile;
	INT  nSeparator;
}SIMCLIENT_OPTIONS;


//=======================================================================
//				The SimConnect options class
//=======================================================================

class CSimClientOptions
{
public:
	CSimClientOptions()
	{
		SetDefaultOptions();
	}

	virtual ~CSimClientOptions(){}

	BOOL IsUseSchedule() 
	{ 
		BOOL flag;
		Lock lock(&g_csAppData);
		flag = m_Options.fUseSchedule; 
		lock.Release();
		return flag;
	}

	BOOL IsUseTimer() 
	{ 
		BOOL flag;
		Lock lock(&g_csAppData);
		flag = m_Options.fUseTimer; 
		lock.Release();
		return flag;
	}


	BOOL IsPrintFile() 
	{ 
		BOOL flag;
		Lock lock(&g_csAppData);
		flag =  m_Options.fPrintFile; 
		lock.Release();
		return flag;
	}

	INT GetSeparator()
	{
		BOOL flag;
		Lock lock(&g_csAppData);
		flag = m_Options.nSeparator;
		lock.Release();
		return flag;
	}
	void SetDefaultOptions();
	void SetOptions(__in const SIMCLIENT_OPTIONS* pSimOptions);
	void GetOptions(__inout SIMCLIENT_OPTIONS* pSimOptions);

	// Registry support
	BOOL ReadOptions();
	void SaveOptions();

private:

	SIMCLIENT_OPTIONS m_Options;

protected:
	DISABLE_COPY_AND_ASSIGN(CSimClientOptions);

};


//=======================================================================
//				The data for internal using in SimConnect
//=======================================================================

typedef struct _INTERNAL_DATA
{
	bool onground;
	long radio_height;

}INTERNAL_DATA;

//=======================================================================
//				Check result from functions
//   Usage: CHECK_RESULT(m_hCurrentResult);
//=======================================================================

inline void CheckResult(LONG hResult, LPCTSTR pszFunctionName, LPCTSTR pszFileName, UINT nLine)
{
	if (hResult != S_OK)
	{
#ifdef _DEBUG
		TRACE4("%s failed with code %#x : File: %s (%d)\n",
			pszFunctionName, hResult, pszFileName, nLine);
#else
		UNREFERENCED_PARAMETER(pszFunctionName);
		UNREFERENCED_PARAMETER(pszFileName);
		UNREFERENCED_PARAMETER(nLine);
#endif // _DEBUG
	}
}

#ifndef _UNICODE
#define CHECK_RESULT(hResult) \
	CheckResult(hResult, __FUNCTION__, __FILE__, __LINE__)
#else
#define CHECK_RESULT(hResult) \
	CheckResult(hResult, __FUNCTIONW__, __FILEW__, __LINE__)
#endif

//=======================================================================
//				SimConnect API wrapper
//=======================================================================

class CSimConnect : public CObject
{
public:
	CSimConnect();
	virtual ~CSimConnect();

	BOOL IsRunning() const { return !m_fEndSession; }
	void Quit() { m_fEndSession = TRUE; }

	BOOL IsPaused() const 
	{ 
		BOOL flag;

		Lock lock(&g_csSimData);
		flag = m_fSimPause; 
		lock.Release();
		return flag;
	}

	// Must be defined
	void CallDispatch(DispatchProc Proc, CSimConnect* pSimConnect);
	BOOL OpenConnection(LPCSTR pSessionName);
	void CloseConnection();
	void Initialize();

	// Event Handlers
	void EventPause(SIMCONNECT_RECV_EVENT* evt);
	void EventPaused();
	void EventUnpaused();
	void EventStart();
	void EventStop();
	
	void Event6Hz();
	void EventPositionChanged();
	void EventOneSecond();
	void EventFourSecond();
	void EventFrame();
	void EventQuit();
	void EventOpen(SIMCONNECT_RECV_OPEN* pData);
	void EventException(SIMCONNECT_RECV_EXCEPTION* pException);
	
	// Simulator Variable Update
	void UpdateFlightData(LPVOID pArrayAddress, DWORD_PTR dwArrayCount);
	void UpdateAircraftName(LPVOID pArrayAddress, DWORD_PTR dwArrayCount);
	void UpdateRadioHeight(LPVOID pArrayAddress, DWORD_PTR dwArrayCount);
	void AIRFileLoaded(LPCSTR pszPathName);
	
	// SimConnect functions wrapper
	void RequestDataOnUserObject(SIMCONNECT_DATA_REQUEST_ID RequestID, SIMCONNECT_DATA_DEFINITION_ID DefineID);
	void RequestDataOnSimObjectType(SIMCONNECT_DATA_REQUEST_ID RequestID, SIMCONNECT_DATA_DEFINITION_ID DefineID);
	void RequestSystemState(SIMCONNECT_DATA_REQUEST_ID  RequestID, const char*  szState);
	void GetLastSentPacketID(__in_opt DWORD* pdwSendID);
	void MapClientEventToSimEvent(SIMCONNECT_CLIENT_EVENT_ID  EventID, const char*  EventName);
	void TransmitClientEvent(SIMCONNECT_CLIENT_EVENT_ID EventID, DWORD dwData);
	void SetDataOnSimObject(SIMCONNECT_DATA_DEFINITION_ID  DefineID, DWORD  ArrayCount, DWORD  cbUnitSize, void*  pDataSet);
	void SendDataToSimulator(DWORD dwDataDefinition, DWORD dwSize, void* pDataSet);

	// Subscribe on system event and add to data definition
	void Subscribe(SIMCONNECT_CLIENT_EVENT_ID EventID, const char* EventName)
	{
		CHECK_RESULT(::SimConnect_SubscribeToSystemEvent(m_hSimConnect, EventID, EventName));
		AddSender(EventName);
	}

	void AddToData(const char* DatumName, const char* UnitsName)
	{
		CHECK_RESULT(::SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_FLIGHT_DATA, DatumName, UnitsName));
		AddSender(DatumName);
	}

	void AddStatic(const char* DatumName, const char* UnitsName)
	{
		CHECK_RESULT(::SimConnect_AddToDataDefinition(m_hSimConnect, DEFINITION_AIRCRAFT_NAME, DatumName, UnitsName));
		AddSender(DatumName);
	}

	void SetPause(BOOL fPause);

	void AssertValid();

	BOOL IsUseTimer() { return m_Options.IsUseTimer(); }
	
	// Access to options for using in global scope
	void GetOptions(SIMCLIENT_OPTIONS* pSimOptions);
	void SetOptions(const SIMCLIENT_OPTIONS* pSimOptions);

protected:
	DISABLE_COPY_AND_ASSIGN(CSimConnect);

private:

	UINT GetFrequency();
	void ShowStatus(INT nPart, LPCTSTR lpszText);

	BOOL m_fSimPause;
	INTERNAL_DATA m_InternalData;
	AIRCRAFT_NAME m_pAircraft;
	
	BOOL m_fEndSession;
	HANDLE m_hSimConnect;
	HRESULT m_hCurrentResult;
	enum SIMTYPE m_SimType;

	// ---- Output system -------------------------------
	CSimConnectOutput* m_pOutput;
	void CreateOutputFolder();
	void CreateOutputFile();
	// ---- Timer system --------------------------------
	UINT m_uFrequency;
	CTimer* m_pTimer;
	void TimerReset(UINT nNewElapse);
	// ---- Exception handler system ---------------------
	void AddSender(const char* pSenderName);
	char* FindSender(DWORD dwSenderID);
	int m_SenderID;
	struct SIMCONNECT_DATA_SENDER aDataSender[MAX_EXCEPTION_RECORDS];
	//------ Options -----------------------------
	CSimClientOptions m_Options;
	void ReadOptions(CSimClientOptions* pSimConnectOpt);
	void SaveOptions(CSimClientOptions& SimConnectOpt);

};

//=======================================================================
//				SimConnect Interface
//======================================================================= 

class CSimClient
{
public:
	CSimClient()
	{
		m_pSimConnect = new CSimConnect();
		m_pSimThread = new CSimThread();
	}

	virtual ~CSimClient()
	{
		delete m_pSimConnect;
		delete m_pSimThread;
	}

	DISABLE_COPY_AND_ASSIGN(CSimClient);
	
	BOOL OpenConnection(LPCSTR pSessionName)
	{
		ASSERT_VALID(m_pSimConnect);
		return m_pSimConnect->OpenConnection(pSessionName);
	}

	void Initialize()
	{
		ASSERT_VALID(m_pSimConnect);
		m_pSimConnect->Initialize();
	}

	BOOL IsRunning()
	{
		ASSERT_VALID(m_pSimConnect);
		return m_pSimConnect->IsRunning();
	}

	void CallDispatch(DispatchProc lpfnProc)
	{
		ASSERT_VALID(m_pSimConnect);
		m_pSimConnect->CallDispatch(lpfnProc, m_pSimConnect);
	}

	void Quit()
	{
		ASSERT_VALID(m_pSimConnect);
		m_pSimConnect->Quit();
	}

	void CloseConnection()
	{
		ASSERT_VALID(m_pSimConnect);
		m_pSimConnect->CloseConnection();
	}

	void SetPause(BOOL fPause)
	{
		ASSERT_VALID(m_pSimConnect);
		m_pSimConnect->SetPause(fPause);
	}

	void GetOptions(SIMCLIENT_OPTIONS* pSimOptions)
	{
		ASSERT_VALID(m_pSimConnect);
		m_pSimConnect->GetOptions(pSimOptions);
	}

	void SetOptions(const SIMCLIENT_OPTIONS* pSimOptions)
	{
		ASSERT_VALID(m_pSimConnect);
		m_pSimConnect->SetOptions(pSimOptions);
	}

	void SendDataToSimulator(DWORD dwDataDefinition, DWORD dwSize, void* pDataSet)
	{
		ASSERT_VALID(m_pSimConnect);
		m_pSimConnect->SendDataToSimulator(dwDataDefinition, dwSize, pDataSet);
	}

	BOOL BeginSession(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter)
	{
		ASSERT_VALID(m_pSimThread);
		return m_pSimThread->CreateThread(lpStartAddress, lpParameter);
	}

	void EndSession()
	{
		ASSERT_VALID(m_pSimThread);
		m_pSimThread->ExitThread();
	}

	void SendTerminationSignal()
	{
		ASSERT_VALID(m_pSimThread);
		m_pSimThread->SendSignal();
	}

	BOOL IsTerminated()
	{
		ASSERT_VALID(m_pSimThread);
		return m_pSimThread->IsTerminated();
	}

	CSimConnect* GetSimConnect() const
	{
		ASSERT_VALID(m_pSimConnect);
		return m_pSimConnect;
	}

	void AssertValid()
	{
		ASSERT(m_pSimConnect);
		ASSERT(m_pSimThread);
	}

private:

	CSimConnect* m_pSimConnect;
	CSimThread* m_pSimThread;
};


//=======================================================================
//				Array of simulator variables
//======================================================================= 

class SimData
{

public:
	static SimData* GetSimData();
	static void FreeSimData();

	static const SIMVARIABLE* GetVariable(INT_PTR nPosition) 
	{ 
		return &m_Variables[nPosition]; 
	}

	void SetValue(INT_PTR nPosition, double Value)
	{
		Lock lock(&g_csSimData);
		m_Variables[nPosition].Value = Value;
		lock.Release();
	}

	void SetArraySize(SIZE_T nSize) { m_nSize = nSize; }
	static SIZE_T GetArraySize() { return m_nSize; }

protected:
	static SimData* volatile m_data;
	static LONG m_nRefCount;

	SimData();
	~SimData();

private:
	static SIZE_T m_nSize;
	static SIMVARIABLE m_Variables[SIM_VARS_COUNT];
};


//=======================================================================
//				Global scope functions
//======================================================================= 

VOID CALLBACK SimClientDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext);
VOID CALLBACK SimClientTimerProc(HWND hwnd,  UINT uMsg, UINT_PTR idEvent,  DWORD dwTime);

BOOL ReadSimClientOptions(SIMCLIENT_OPTIONS* pSimClientOpt);
BOOL SaveSimClientOptions(const SIMCLIENT_OPTIONS* pSimOptions);

//=======================================================================
//				Global scope variables
//======================================================================= 

// TODO: Make it a class member
extern CONTROLS_DEFLECTION g_CD;


//////////////////////////////////////////////////////////////////////////

#endif // __SIMCLIENT_H__


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

SimData* volatile SimData::m_data = NULL;
LONG SimData::m_nRefCount = 0;
size_t SimData::m_nSize = SIM_VARS_COUNT;

SimData::SimData() 
{
}

SimData::~SimData() 
{
}

SimData* SimData::GetSimData()
{
	SimData* volatile temp = m_data;

	// See Meyers and Alexandrescu
	// 'C++ and the Perils of Double-Checked Locking'
	MemoryBarrier();

	if (temp == NULL)
	{
		Lock lock(&g_csSimData);
		//EnterCriticalSection(&g_csSimData);	// Lock
		if (temp == NULL)
		{
			temp = new SimData();
			MemoryBarrier();
			m_data = temp;
			m_nRefCount++;
		}
		//LeaveCriticalSection(&g_csSimData);	// Unlock
		lock.Release();
	}

	return temp;
}

void SimData::FreeSimData()
{

	//EnterCriticalSection(&g_csSimData);	// Lock

	Lock lock(&g_csSimData);
	m_nRefCount--; 
	if(!m_nRefCount) 
	{
		delete m_data; 
		m_nRefCount = NULL; 
	}
	//LeaveCriticalSection(&g_csSimData);	// Unlock
	lock.Release();
}

//////////////////////////////////////////////////////////////////////////

SIMVARIABLE SimData::m_Variables[SIM_VARS_COUNT] =
{

	{"OnGround", "", 0},
	{"Alpha", "deg", 0},
	{"Beta", "deg", 0},
	{"BDot", "deg/sec", 0},
	{"TAS", "kts", 0},
	{"Mach", "n", 0},
	{"VSpeed", "fpm", 0},
	{"GForce", "g", 0},
	{"Weight", "lbs", 0},
	{"Dyn", "pft2", 0},
	//------------------
	{"IAS", "kts", 0},
	{"Lat", "deg", 0},
	{"Lon", "deg", 0},
	{"Alt", "ft", 0},
	{"Pitch", "deg", 0},
	{"Bank", "deg", 0},
	{"HTrue", "deg", 0},
	{"HMag", "deg", 0},
	{"MagVar", "deg", 0},
	{"Height", "ft", 0},
	{"GSpeed", "kts", 0},
	{"CGlon", "%", 0},
	{"CGlat", "%", 0},
	//------------------
	{"VBX", "fps", 0},
	{"VBY", "fps", 0},
	{"VBZ", "fps", 0},
	//------------------
	{"VWX", "fps", 0},
	{"VWY", "fps", 0},
	{"VWZ", "fps", 0},
	//------------------
	{"AWX", "fps2", 0},
	{"AWY", "fps2", 0},
	{"AWZ", "fps2", 0},
	//------------------
	{"ABX", "fps2", 0},
	{"ABY", "fps2", 0},
	{"ABZ", "fps2", 0},
	//------------------
	{"RBX", "fps", 0},
	{"RBY", "fps", 0},
	{"RBZ", "fps", 0},
	//------------------
	{"WINDX", "fps", 0},
	{"WINDY", "fps", 0},
	{"WINDZ", "fps", 0},
	//------------------
	{"AMBDENS", "slug", 0},
	{"AMBTEM", "C", 0},
	{"STDTEMP", "C", 0},
	{"TAT", "C", 0},
	//------------------
	{"AWVEL", "kts", 0},
	{"AWDIR", "deg", 0},
	{"AWINDX", "m/s", 0},
	{"AWINDY", "m/s", 0},
	{"AWINDZ", "m/s", 0},
	//------------------
	{"ACWINDX", "kts", 0},
	{"ACWINDY", "kts", 0},
	{"ACWINDZ", "kts", 0},
	//------------------
	{"AMBPRESS", "inHg", 0},
	{"BAROPRESS", "inHg", 0},
	{"SLPRESS", "inHg", 0},
	{"AMBVIS", "m", 0},
	//------------------
	{"RRM", "RPM", 0},
	{"Lever", "%", 0},
	{"EGT", "C", 0},
	{"OilPress", "psf", 0},
	{"OilTemp", "C", 0},
	{"FuelPress", "psf", 0},
	{"EngTime", "H", 0},
	//------------------
	{"N1", "%", 0},
	{"N2", "%", 0},
	{"CorrN1", "%", 0},
	{"CorrN2", "%", 0},
	{"CorrFF", "PPH", 0},
	//------------------
	{"Torque", "%", 0},
	{"EPR", "Ratio", 0},
	{"ITT", "C", 0},
	{"Thrust", "Pounds", 0},
	{"AirBleed", "psi", 0},
	{"FF", "PPH", 0},
	{"Vibat", "Num", 0},
	{"Revers", "%", 0},

	// Flight Controls
	{"Elev. Def", "deg", 0},
	{"Elev. Pct", "%", 0},
	{"Ail. Def", "deg", 0},
	{"Ail. Pct", "%", 0},
	{"Rud. Def", "deg", 0},
	{"Rud. Pct", "%", 0},

	{"Elev.Trim", "%", 0},
	{"Ail. Trim", "%", 0},
	{"Rud. Trim", "%", 0},

	{"Prop. RPM", "rpm", 0},
	{"Prop. Pct", "%", 0},
	{"Prop. Beta", "deg", 0},
	{"Prop. Thrust", "Pounds", 0},
};

//////////////////////////////////////////////////////////////////////////


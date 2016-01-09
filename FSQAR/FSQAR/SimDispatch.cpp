
#include "stdafx.h"
#include "fsqar.h"
#include "simclient.h"

//////////////////////////////////////////////////////////////////////////
// SimConnect dispatch procedure
//
void CALLBACK SimClientDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
	UNREFERENCED_PARAMETER(cbData); 

	// Create a local copy of the CSimConnect object
	CSimConnect* pSimConnect = static_cast<CSimConnect*>(pContext);
	ASSERT(pSimConnect);
	__analysis_assume(pSimConnect);
	
	switch (pData->dwID)
	{
	case SIMCONNECT_RECV_ID_OPEN:
		{
			SIMCONNECT_RECV_OPEN* pOpen = static_cast<SIMCONNECT_RECV_OPEN*>(pData);
			pSimConnect->EventOpen(pOpen);
		}
		break;

	case SIMCONNECT_RECV_ID_EVENT:
	{
		SIMCONNECT_RECV_EVENT* pEvent = static_cast<SIMCONNECT_RECV_EVENT*>(pData);
		
		switch (pEvent->uEventID)
		{
		case EVENT_START:
			pSimConnect->EventStart();
			break;
		case EVENT_STOP:
			pSimConnect->EventStop();
			break;
		case EVENT_PAUSE:
			pSimConnect->EventPause(pEvent);
			break;
		case EVENT_PAUSED:
			pSimConnect->EventPaused();
			break;
		case EVENT_UNPAUSED:
			pSimConnect->EventUnpaused();
			break;
		case EVENT_6HZ:
			pSimConnect->Event6Hz();
			break;
		case EVENT_ONE_SECOND:
			pSimConnect->EventOneSecond();
			break;
		case EVENT_FOUR_SECONDS:
			pSimConnect->EventFourSecond();
			break;

		default:
			break;
		}
	}
	break;

	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
	{
		SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData = static_cast<SIMCONNECT_RECV_SIMOBJECT_DATA*>(pData);
		
		switch (pObjData->dwRequestID)
		{
		case REQUEST_FLIGHT_DATA:
			pSimConnect->UpdateFlightData(&pObjData->dwData, pObjData->dwDefineCount);
			break;

		case REQUEST_AIRCRAFT_NAME:
			pSimConnect->UpdateAircraftName(&pObjData->dwData, pObjData->dwDefineCount);
			break;

		case REQUEST_CONTROLS_POS:
			{
				CONTROLS_DEFLECTION* pCtrlDef = reinterpret_cast<CONTROLS_DEFLECTION*>(&pObjData->dwData);
				g_lever_pos = pCtrlDef->lever1;
			}
			break;

		case REQUEST_RADIO_HEIGHT:
				pSimConnect->UpdateRadioHeight(&pObjData->dwData, pObjData->dwDefineCount);
				break;

		default:
			break;
		}
	}
	break;

	case SIMCONNECT_RECV_ID_EVENT_FILENAME:
		{
			SIMCONNECT_RECV_EVENT_FILENAME* pAIRFile = (SIMCONNECT_RECV_EVENT_FILENAME*) pData;
			pSimConnect->AIRFileLoaded(pAIRFile->szFileName);
		}
		break;

	case SIMCONNECT_RECV_ID_EXCEPTION:
		{
			SIMCONNECT_RECV_EXCEPTION* pExcept = static_cast<SIMCONNECT_RECV_EXCEPTION*>(pData);
			pSimConnect->EventException(pExcept);
		}
		break;

	case SIMCONNECT_RECV_ID_QUIT:
		pSimConnect->EventQuit();
		break;

	default:
		break;
	}
}
//////////////////////////////////////////////////////////////////////////

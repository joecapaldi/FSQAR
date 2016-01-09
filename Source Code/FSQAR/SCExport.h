
#ifndef __SCEXPORT_H__
#define __SCEXPORT_H__

/*

_SimConnect_Open@24 referenced in function "public: int __thiscall CSimConnect::OpenConnection(char const *)" (?OpenConnection@CSimConnect@@QAEHPBD@Z)
_SimConnect_AddToDataDefinition@28 referenced in function "public: void __thiscall CSimConnect::Initialize(void)" (?Initialize@CSimConnect@@QAEXXZ)
_SimConnect_SubscribeToSystemEvent@12 referenced in function "public: void __thiscall CSimConnect::Initialize(void)" (?Initialize@CSimConnect@@QAEXXZ)
_SimConnect_CallDispatch@12 referenced in function "public: void __thiscall CSimConnect::CallDispatch(void (__stdcall*)(struct SIMCONNECT_RECV *,unsigned long,void *),class CSimConnect *)" (?CallDispatch@CSimConnect@@QAEXP6GXPAUSIMCONNECT_RECV@@KPAX@ZPAV1@@Z)
_SimConnect_Close@4 referenced in function "public: void __thiscall CSimConnect::CloseConnection(void)" (?CloseConnection@CSimConnect@@QAEXXZ)
_SimConnect_RequestDataOnSimObjectType@20 referenced in function "public: void __thiscall CSimConnect::RequestDataOnSimObjectType(unsigned long,unsigned long)" (?RequestDataOnSimObjectType@CSimConnect@@QAEXKK@Z)
_SimConnect_RequestDataOnSimObject@36 referenced in function "public: void __thiscall CSimConnect::RequestDataOnUserObject(unsigned long,unsigned long)" (?RequestDataOnUserObject@CSimConnect@@QAEXKK@Z)
_SimConnect_GetLastSentPacketID@8 referenced in function "public: void __thiscall CSimConnect::GetLastSentPacketID(unsigned long *)" (?GetLastSentPacketID@CSimConnect@@QAEXPAK@Z)
_SimConnect_GetLastSentPacketID@8
_SimConnect_RequestSystemState@12 referenced in function "public: void __thiscall CSimConnect::RequestSystemState(unsigned long,char const *)" (?RequestSystemState@CSimConnect@@QAEXKPBD@Z)
_SimConnect_SetDataOnSimObject@28 referenced in function "public: void __thiscall CSimConnect::SetDataOnSimObject(unsigned long,unsigned long,unsigned long,void *)" (?SetDataOnSimObject@CSimConnect@@QAEXKKKPAX@Z)
_SimConnect_MapClientEventToSimEvent@12 referenced in function "public: void __thiscall CSimConnect::MapClientEventToSimEvent(unsigned long,char const *)" (?MapClientEventToSimEvent@CSimConnect@@QAEXKPBD@Z)
_SimConnect_TransmitClientEvent@24 referenced in function "public: void __thiscall CSimConnect::TransmitClientEvent(unsigned long,unsigned long)" (?TransmitClientEvent@CSimConnect@@QAEXKK@Z)

*/

/*
_SimConnect_Open
_SimConnect_Close
_SimConnect_AddToDataDefinition
_SimConnect_SubscribeToSystemEvent
_SimConnect_CallDispatch

_SimConnect_RequestDataOnSimObjectType
_SimConnect_RequestDataOnSimObject
_SimConnect_GetLastSentPacketID
_SimConnect_GetLastSentPacketID
_SimConnect_RequestSystemState
_SimConnect_SetDataOnSimObject
_SimConnect_MapClientEventToSimEvent
_SimConnect_TransmitClientEvent

*/

#define STDCALL __stdcall

typedef HRESULT (*_SimConnect_Open)(HANDLE* , LPCSTR, HWND, DWORD, HANDLE, DWORD);
typedef HRESULT (*_SimConnect_Close)(HANDLE);


#ifdef SIMCONNECT_DYNAMIC_LOAD



class CSimConnectLoader
{
public:

	CSimConnectLoader();
	virtual ~CSimConnectLoader();
private:
	void LoadLibrary();
	void FreeLibrary();

_SimConnect_Open SimConnect_Open;
_SimConnect_Close SimConnect_Close;
};

#endif // SIMCONNECT_DYNAMIC_LOAD

#endif // __SCEXPORT_H__

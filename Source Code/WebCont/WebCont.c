//////////////////////////////////////////////////////////////////////////
//
// WEBCONT: The web browser control implementation
// -----------------------------------------------------------------------
// Original code was taken from article 'Embed an HTML control in your own 
// window using plain C' by Jeff Glatt, on www.codeproject.net
// 
// Copyright (C) 2006 Jeff Glatt, 2009, 2016 AlexShag
// -----------------------------------------------------------------------
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// For the author all your responses and wishes about this program 
// are important. To make a suggestion or report a bug or another 
// feature of this product, write to the alexshag@mail.ru. 
//
//////////////////////////////////////////////////////////////////////////

// Declare VTBLs as constants
#define CONST_VTABLE 
 
#include <windows.h>
#include <tchar.h>
#include <exdisp.h>
#include <mshtml.h>
#include <mshtmhst.h>
#include <crtdbg.h>	

#include "webcont.h"
#include "webcres.h"


static UINT g_nWindowCount = 0;
static const TCHAR szWebControlClassName[] = _T(ALX_WEB_CONTROL_CLASS);
static const SAFEARRAYBOUND ArrayBound = {1, 0};

/////////////////////////////////////////////////////////////////////////////
// IOleInPlaceFrame
//
// The IOleInPlaceFrame interface controls the container's top-level frame window.
// You will need to implement this interface if you are writing a container 
// application that will be participating in in-place activation.
//
// IOleInPlaceFrame methods in VTable Order
//
HRESULT STDMETHODCALLTYPE Frame_QueryInterface(IOleInPlaceFrame* This, REFIID riid, LPVOID* ppvObj);
HRESULT STDMETHODCALLTYPE Frame_AddRef(IOleInPlaceFrame* This);
HRESULT STDMETHODCALLTYPE Frame_Release(IOleInPlaceFrame* This);
HRESULT STDMETHODCALLTYPE Frame_GetWindow(IOleInPlaceFrame* This, HWND * lphwnd);
HRESULT STDMETHODCALLTYPE Frame_ContextSensitiveHelp(IOleInPlaceFrame* This, BOOL fEnterMode);
HRESULT STDMETHODCALLTYPE Frame_GetBorder(IOleInPlaceFrame* This, LPRECT lprectBorder);
HRESULT STDMETHODCALLTYPE Frame_RequestBorderSpace(IOleInPlaceFrame* This, LPCBORDERWIDTHS pborderwidths);
HRESULT STDMETHODCALLTYPE Frame_SetBorderSpace(IOleInPlaceFrame* This, LPCBORDERWIDTHS pborderwidths);
HRESULT STDMETHODCALLTYPE Frame_SetActiveObject(IOleInPlaceFrame* This, IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName);
HRESULT STDMETHODCALLTYPE Frame_InsertMenus(IOleInPlaceFrame* This, HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
HRESULT STDMETHODCALLTYPE Frame_SetMenu(IOleInPlaceFrame* This, HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
HRESULT STDMETHODCALLTYPE Frame_RemoveMenus(IOleInPlaceFrame* This, HMENU hmenuShared);
HRESULT STDMETHODCALLTYPE Frame_SetStatusText(IOleInPlaceFrame* This, LPCOLESTR pszStatusText);
HRESULT STDMETHODCALLTYPE Frame_EnableModeless(IOleInPlaceFrame* This, BOOL fEnable);
HRESULT STDMETHODCALLTYPE Frame_TranslateAccelerator(IOleInPlaceFrame* This, LPMSG lpmsg, WORD wID);

IOleInPlaceFrameVtbl g_IOleInPlaceFrameVtbl = 
{
	Frame_QueryInterface,
	Frame_AddRef,
	Frame_Release,
	Frame_GetWindow,
	Frame_ContextSensitiveHelp,
	Frame_GetBorder,
	Frame_RequestBorderSpace,
	Frame_SetBorderSpace,
	Frame_SetActiveObject,
	Frame_InsertMenus,
	Frame_SetMenu,
	Frame_RemoveMenus,
	Frame_SetStatusText,
	Frame_EnableModeless,
	Frame_TranslateAccelerator
};


typedef struct _COleInPlaceFrame
{
	IOleInPlaceFrame m_OleInPlaceFrame; // Must be first
	HWND hWndOwner;

	// TODO: Add here any extra variables that you need
	// to access in your IOleInPlaceFrame functions.

} COleInPlaceFrame;

/////////////////////////////////////////////////////////////////////////////
// IOleClientSite
//
// The IOleClientSite interface is the primary means by which an embedded 
// object obtains information about the location and extent of its display site, 
// its moniker, its user interface, and other resources provided by its container. 
// An object server calls IOleClientSite to request services from the container. 
// A container must provide one instance of IOleClientSite for every 
// compound-document object it contains.
//
// IOleClientSite methods in VTable Order
//
HRESULT STDMETHODCALLTYPE ClientSite_QueryInterface(IOleClientSite * This, REFIID riid, void ** ppvObject);
HRESULT STDMETHODCALLTYPE ClientSite_AddRef(IOleClientSite * This);
HRESULT STDMETHODCALLTYPE ClientSite_Release(IOleClientSite * This);
HRESULT STDMETHODCALLTYPE ClientSite_SaveObject(IOleClientSite * This);
HRESULT STDMETHODCALLTYPE ClientSite_GetMoniker(IOleClientSite * This, DWORD dwAssign, DWORD dwWhichMoniker, IMoniker ** ppmk);
HRESULT STDMETHODCALLTYPE ClientSite_GetContainer(IOleClientSite * This, LPOLECONTAINER * ppContainer);
HRESULT STDMETHODCALLTYPE ClientSite_ShowObject(IOleClientSite * This);
HRESULT STDMETHODCALLTYPE ClientSite_OnShowWindow(IOleClientSite * This, BOOL fShow);
HRESULT STDMETHODCALLTYPE ClientSite_RequestNewObjectLayout(IOleClientSite * This);

IOleClientSiteVtbl g_IOleClientSiteVtbl = 
{
	ClientSite_QueryInterface,
	ClientSite_AddRef,
	ClientSite_Release,
	ClientSite_SaveObject,
	ClientSite_GetMoniker,
	ClientSite_GetContainer,
	ClientSite_ShowObject,
	ClientSite_OnShowWindow,
	ClientSite_RequestNewObjectLayout
};

/////////////////////////////////////////////////////////////////////////////
// IDocHostUIHandler
//
// Enables an application that is hosting the WebBrowser Control or automating 
// Microsoft Internet Explorer to replace the menus, toolbars, and context menus 
// used by MSHTML.
//
// IDocHostUIHandler methods in VTable Order
//
HRESULT STDMETHODCALLTYPE UI_QueryInterface(IDocHostUIHandler * This, REFIID riid, void ** ppvObject);
HRESULT STDMETHODCALLTYPE UI_AddRef(IDocHostUIHandler * This);
HRESULT STDMETHODCALLTYPE UI_Release(IDocHostUIHandler * This);
HRESULT STDMETHODCALLTYPE UI_ShowContextMenu(IDocHostUIHandler * This, DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved);
HRESULT STDMETHODCALLTYPE UI_GetHostInfo(IDocHostUIHandler * This, DOCHOSTUIINFO __RPC_FAR *pInfo);
HRESULT STDMETHODCALLTYPE UI_ShowUI(IDocHostUIHandler * This, DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc);
HRESULT STDMETHODCALLTYPE UI_HideUI(IDocHostUIHandler * This);
HRESULT STDMETHODCALLTYPE UI_UpdateUI(IDocHostUIHandler * This);
HRESULT STDMETHODCALLTYPE UI_EnableModeless(IDocHostUIHandler * This, BOOL fEnable);
HRESULT STDMETHODCALLTYPE UI_OnDocWindowActivate(IDocHostUIHandler * This, BOOL fActivate);
HRESULT STDMETHODCALLTYPE UI_OnFrameWindowActivate(IDocHostUIHandler * This, BOOL fActivate);
HRESULT STDMETHODCALLTYPE UI_ResizeBorder(IDocHostUIHandler * This, LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow);
HRESULT STDMETHODCALLTYPE UI_TranslateAccelerator(IDocHostUIHandler * This, LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID);
HRESULT STDMETHODCALLTYPE UI_GetOptionKeyPath(IDocHostUIHandler * This, LPOLESTR __RPC_FAR *pchKey, DWORD dw);
HRESULT STDMETHODCALLTYPE UI_GetDropTarget(IDocHostUIHandler * This, IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget);
HRESULT STDMETHODCALLTYPE UI_GetExternal(IDocHostUIHandler * This, IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
HRESULT STDMETHODCALLTYPE UI_TranslateUrl(IDocHostUIHandler * This, DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut);
HRESULT STDMETHODCALLTYPE UI_FilterDataObject(IDocHostUIHandler * This, IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet);

IDocHostUIHandlerVtbl g_IDocHostUIHandlerVtbl =  
{
	UI_QueryInterface,
	UI_AddRef,
	UI_Release,
	UI_ShowContextMenu,
	UI_GetHostInfo,
	UI_ShowUI,
	UI_HideUI,
	UI_UpdateUI,
	UI_EnableModeless,
	UI_OnDocWindowActivate,
	UI_OnFrameWindowActivate,
	UI_ResizeBorder,
	UI_TranslateAccelerator,
	UI_GetOptionKeyPath,
	UI_GetDropTarget,
	UI_GetExternal,
	UI_TranslateUrl,
	UI_FilterDataObject
};

/////////////////////////////////////////////////////////////////////////////
// IOleInPlaceSite
//
// The IOleInPlaceSite interface manages interaction between the container 
// and the object's in-place client site. Recall that the client site is 
// the display site for embedded objects, and provides position and conceptual 
// information about the object. 
//
// IOleInPlaceSite methods in VTable Order
//
HRESULT STDMETHODCALLTYPE InPlaceSite_QueryInterface(IOleInPlaceSite * This, REFIID riid, void ** ppvObject);
HRESULT STDMETHODCALLTYPE InPlaceSite_AddRef(IOleInPlaceSite * This);
HRESULT STDMETHODCALLTYPE InPlaceSite_Release(IOleInPlaceSite * This);
HRESULT STDMETHODCALLTYPE InPlaceSite_GetWindow(IOleInPlaceSite * This, HWND * lphwnd);
HRESULT STDMETHODCALLTYPE InPlaceSite_ContextSensitiveHelp(IOleInPlaceSite * This, BOOL fEnterMode);
HRESULT STDMETHODCALLTYPE InPlaceSite_CanInPlaceActivate(IOleInPlaceSite * This);
HRESULT STDMETHODCALLTYPE InPlaceSite_OnInPlaceActivate(IOleInPlaceSite * This);
HRESULT STDMETHODCALLTYPE InPlaceSite_OnUIActivate(IOleInPlaceSite * This);
HRESULT STDMETHODCALLTYPE InPlaceSite_GetWindowContext(IOleInPlaceSite * This, LPOLEINPLACEFRAME * lplpFrame,LPOLEINPLACEUIWINDOW * lplpDoc,LPRECT lprcPosRect,LPRECT lprcClipRect,LPOLEINPLACEFRAMEINFO lpFrameInfo);
HRESULT STDMETHODCALLTYPE InPlaceSite_Scroll(IOleInPlaceSite * This, SIZE scrollExtent);
HRESULT STDMETHODCALLTYPE InPlaceSite_OnUIDeactivate(IOleInPlaceSite * This, BOOL fUndoable);
HRESULT STDMETHODCALLTYPE InPlaceSite_OnInPlaceDeactivate(IOleInPlaceSite * This);
HRESULT STDMETHODCALLTYPE InPlaceSite_DiscardUndoState(IOleInPlaceSite * This);
HRESULT STDMETHODCALLTYPE InPlaceSite_DeactivateAndUndo(IOleInPlaceSite * This);
HRESULT STDMETHODCALLTYPE InPlaceSite_OnPosRectChange(IOleInPlaceSite * This, LPCRECT lprcPosRect);

IOleInPlaceSiteVtbl g_IOleInPlaceSiteVtbl =  
{
	InPlaceSite_QueryInterface,
	InPlaceSite_AddRef,
	InPlaceSite_Release,
	InPlaceSite_GetWindow,
	InPlaceSite_ContextSensitiveHelp,
	InPlaceSite_CanInPlaceActivate,
	InPlaceSite_OnInPlaceActivate,
	InPlaceSite_OnUIActivate,
	InPlaceSite_GetWindowContext,
	InPlaceSite_Scroll,
	InPlaceSite_OnUIDeactivate,
	InPlaceSite_OnInPlaceDeactivate,
	InPlaceSite_DiscardUndoState,
	InPlaceSite_DeactivateAndUndo,
	InPlaceSite_OnPosRectChange
};


typedef struct _COleInPlaceSite
{
	IOleInPlaceSite m_OleInPlaceSite;	// Must be first with in COleInPlaceSite.
	COleInPlaceFrame m_InPlaceFrame;	//Must be first within my COleInPlaceFrame

	// TODO: Add here any extra variables that you need
	// to access in your IOleInPlaceSite functions.
	
}COleInPlaceSite;

typedef struct _CDocHostUIHandler
{
	IDocHostUIHandler m_DocHostUI; //  Must be first.

	// TODO: Add here any extra variables that you need
	// to access in your IDocHostUIHandler functions.

}CDocHostUIHandler;

typedef struct _COleClientSite
{
	IOleClientSite		m_OleClientSite; // Must be first.
	COleInPlaceSite		m_InPlaceSite;	 // A convenient place to put it.
	CDocHostUIHandler	m_DocHostUI; // Must be first within my CDocHostUIHandler.

	// TODO: Add here any extra variables that you need
	// to access in your IOleClientSite functions.

}COleClientSite;


#define NOTIMPLEMENTED _ASSERT(0); return(E_NOTIMPL)

/////////////////////////////////////////////////////////////////////////////
// The browser object asks us for the pointer to our IDocHostUIHandler object 
// by calling our IOleClientSite's
// QueryInterface (ie, ClientClientSite_QueryInterface) and specifying a REFIID 
// of IID_IDocHostUIHandler.
//
// NOTE: You need at least IE 4.0. Previous versions do not ask for, nor utilize, 
// our IDocHostUIHandler functions.

HRESULT STDMETHODCALLTYPE UI_QueryInterface(IDocHostUIHandler * This, REFIID riid, LPVOID* ppvObj)
{
	return(ClientSite_QueryInterface((IOleClientSite *)((CHAR *)This - sizeof(IOleClientSite) - sizeof(COleInPlaceSite)), riid, ppvObj));
}

HRESULT STDMETHODCALLTYPE UI_AddRef(IDocHostUIHandler * This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE UI_Release(IDocHostUIHandler * This)
{
	return(1);
}

// Called when the browser object is about to display its context menu.
HRESULT STDMETHODCALLTYPE UI_ShowContextMenu(IDocHostUIHandler * This, DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved)
{
	// If desired, we can pop up your own custom context menu here. Of course,
	// we would need some way to get our window handle. Then we have to add an 
	// extra HWND field to store our window handle. For example:
	//
	// typedef struct CDocHostUIHandler 
	// {
	//		IDocHostUIHandler	ui;	
	//		HWND				window;
	// } IDocHostUIHandlerEx;
	//
	 // We will return S_OK to tell the browser not to display its default context menu,
	// or return S_FALSE to let the browser show its default context menu. For this
	// example, we wish to disable the browser's default context menu.
	return(S_OK);
}

// Called at initialization of the browser object UI.
HRESULT STDMETHODCALLTYPE UI_GetHostInfo(IDocHostUIHandler * This, DOCHOSTUIINFO __RPC_FAR *pInfo)
{
	pInfo->cbSize = sizeof(DOCHOSTUIINFO);

	// We don't want any 3D border. 
	// pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER;
	// Other flags:
	// Hide the scroll bar (DOCHOSTUIFLAG_SCROLL_NO) 
	// Dont display pictures (DOCHOSTUIFLAG_NOPICS)
	// Disable scripts (DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE)
	// Open a site in a new window when the user clicks on some link (DOCHOSTUIFLAG_OPENNEWWIN)
	// See the MSDN docs on the DOCHOSTUIINFO struct passed to us.
	

	// Set what happens when the user double-clicks on the object. 
	// Here we use the default.
	pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

	return(S_OK);
}

// Called when the browser object shows its UI. 
// This allows us to replace its menus and toolbars by creating our own and displaying them here.
HRESULT STDMETHODCALLTYPE UI_ShowUI(IDocHostUIHandler * This, DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc)
{
	// We've already got our own UI in place so just return S_OK to tell the browser
	// not to display its menus/toolbars. Otherwise we'd return S_FALSE to let it do
	// that.
	return(S_OK);
}

// Called when browser object hides its UI. 
// This allows us to hide any menus/toolbars we created in ShowUI.
HRESULT STDMETHODCALLTYPE UI_HideUI(IDocHostUIHandler * This)
{
	return(S_OK);
}

// Called when the browser object wants to notify us that the command state has changed. 
// We should update any controls we have that are dependent upon our embedded object, 
// such as "Back", "Forward", "Stop", or "Home" buttons.
HRESULT STDMETHODCALLTYPE UI_UpdateUI(IDocHostUIHandler * This)
{
	// We update our UI in our window message loop so we don't do anything here.
	return(S_OK);
}

// Called from the browser object's IOleInPlaceActiveObject object's EnableModeless() function. 
// Also called when the browser displays a modal dialog box.
HRESULT STDMETHODCALLTYPE UI_EnableModeless(IDocHostUIHandler * This, BOOL fEnable)
{
	return(S_OK);
}

// Called from the browser object's IOleInPlaceActiveObject object's OnDocWindowActivate() function.
// This informs off of when the object is getting/losing the focus.
HRESULT STDMETHODCALLTYPE UI_OnDocWindowActivate(IDocHostUIHandler * This, BOOL fActivate)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE UI_OnFrameWindowActivate(IDocHostUIHandler * This, BOOL fActivate)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE UI_ResizeBorder(IDocHostUIHandler * This, LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow)
{
	return(S_OK);
}


HRESULT STDMETHODCALLTYPE UI_TranslateAccelerator(IDocHostUIHandler * This, LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID)
{
	// We don't intercept any keystrokes, so we do nothing here. 
	// if you want to override the TAB key, perhaps do something with it ourselves, 
	// and then tell the browser not to do anything with this keystroke, for example:
	//
	//	if (pMsg && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
	//	{
	//		// Here we do something as a result of a TAB key press.
	//
	//		// Tell the browser not to do anything with it.
	//		return(S_FALSE);
	//	}
	//
	//	// Otherwise, let the browser do something with this message.
	//	return(S_OK);

	return(S_FALSE);
}

// Called by the browser object to find where the host wishes the browser to get its options in the registry.
// We can use this to prevent the browser from using its default settings in the registry, by telling it to use
// some other registry key we've setup with the options we want.
HRESULT STDMETHODCALLTYPE UI_GetOptionKeyPath(IDocHostUIHandler * This, LPOLESTR __RPC_FAR *pchKey, DWORD dw)
{
	// Let the browser use its default registry settings.
	return(S_FALSE);
}

HRESULT STDMETHODCALLTYPE UI_GetDropTarget(IDocHostUIHandler * This, IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
{
	// Return our IDropTarget object associated with this IDocHostUIHandler object. I don't
	// know why we don't do this via UI_QueryInterface(), but we don't.

	// NOTE: If we want/need an IDropTarget interface, then we would have had to setup our own
	// IDropTarget functions, IDropTarget VTable, and create an IDropTarget object. We'd want to put
	// a pointer to the IDropTarget object in our own custom IDocHostUIHandlerEx object (like how
	// we may add an HWND field for the use of UI_ShowContextMenu). So when we created our
	// IDocHostUIHandlerEx object, maybe we'd add a 'idrop' field to the end of it, and
	// store a pointer to our IDropTarget object there. Then we could return this pointer as so:
	//
	// *pDropTarget = ((IDocHostUIHandlerEx FAR *)This)->idrop;
    // return(S_OK);

	// But for our purposes, we don't need an IDropTarget object, so we'll tell whomever is calling
	// us that we don't have one.
    return(S_FALSE);
}

// Called by the browser when it wants a pointer to our IDispatch object. This object allows us to expose
// our own automation interface (ie, our own COM objects) to other entities that are running within the
// context of the browser so they can call our functions if they want. An example could be a javascript
// running in the URL we display could call our IDispatch functions. We'd write them so that any args passed
// to them would use the generic datatypes like a BSTR for utmost flexibility.
HRESULT STDMETHODCALLTYPE UI_GetExternal(IDocHostUIHandler * This, IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	// Return our IDispatch object associated with this IDocHostUIHandler object. I don't
	// know why we don't do this via UI_QueryInterface(), but we don't.

	// NOTE: If we want/need an IDispatch interface, then we would have had to setup our own
	// IDispatch functions, IDispatch VTable, and create an IDispatch object. We'd want to put
	// a pointer to the IDispatch object in our custom CDocHostUIHandler object (like how
	// we may add an HWND field for the use of UI_ShowContextMenu). So when we defined our
	// CDocHostUIHandler object, maybe we'd add a 'idispatch' field to the end of it, and
	// store a pointer to our IDispatch object there. Then we could return this pointer as so:
	//
	// *ppDispatch = ((CDocHostUIHandler FAR *)This)->idispatch;
    // return(S_OK);

	// But for our purposes, we don't need an IDispatch object, so we'll tell whomever is calling
	// us that we don't have one. Note: We must set ppDispatch to 0 if we don't return our own
	// IDispatch object.
	*ppDispatch = 0;
	return(S_FALSE);
}

// Called by the browser object to give us an opportunity to modify the URL to be loaded.
HRESULT STDMETHODCALLTYPE UI_TranslateUrl(IDocHostUIHandler * This, DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
{
	// We don't need to modify the URL. Note: We need to set ppchURLOut to 0 if we don't
	// return an OLECHAR (buffer) containing a modified version of pchURLIn.
	*ppchURLOut = 0;
    return(S_FALSE);
}

// Called by the browser when it does cut/paste to the clipboard. This allows us to block certain clipboard
// formats or support additional clipboard formats.
HRESULT STDMETHODCALLTYPE UI_FilterDataObject(IDocHostUIHandler * This, IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet)
{
	// Return our IDataObject object associated with this IDocHostUIHandler object. I don't
	// know why we don't do this via UI_QueryInterface(), but we don't.

	// NOTE: If we want/need an IDataObject interface, then we would have had to setup our own
	// IDataObject functions, IDataObject VTable, and create an IDataObject object. We'd want to put
	// a pointer to the IDataObject object in our custom CDocHostUIHandler object (like how
	// we may add an HWND field for the use of UI_ShowContextMenu). So when we defined our
	// CDocHostUIHandler object, maybe we'd add a 'idata' field to the end of it, and
	// store a pointer to our IDataObject object there. Then we could return this pointer as so:
	//
	// *ppDORet = ((CDocHostUIHandler FAR *)This)->idata;
    // return(S_OK);

	// But for our purposes, we don't need an IDataObject object, so we'll tell whomever is calling
	// us that we don't have one. Note: We must set ppDORet to 0 if we don't return our own
	// IDataObject object.
	*ppDORet = 0;
	return(S_FALSE);
}


////////////////////////////////////// My IOleClientSite functions  /////////////////////////////////////

HRESULT STDMETHODCALLTYPE ClientSite_QueryInterface(IOleClientSite * This, REFIID riid, void ** ppvObject)
{

	if (!memcmp(riid, &IID_IUnknown, sizeof(GUID)) || !memcmp(riid, &IID_IOleClientSite, sizeof(GUID)))
		*ppvObject = &((COleClientSite *)This)->m_OleClientSite;
	else if (!memcmp(riid, &IID_IOleInPlaceSite, sizeof(GUID)))
		*ppvObject = &((COleClientSite *)This)->m_InPlaceSite;
	else if (!memcmp(riid, &IID_IDocHostUIHandler, sizeof(GUID)))
		*ppvObject = &((COleClientSite *)This)->m_DocHostUI;
	else
	{
		*ppvObject = 0;
		return(E_NOINTERFACE);
	}

	return(S_OK);
}

HRESULT STDMETHODCALLTYPE ClientSite_AddRef(IOleClientSite * This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE ClientSite_Release(IOleClientSite * This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE ClientSite_SaveObject(IOleClientSite * This)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE ClientSite_GetMoniker(IOleClientSite * This, DWORD dwAssign, DWORD dwWhichMoniker, IMoniker ** ppmk)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE ClientSite_GetContainer(IOleClientSite * This, LPOLECONTAINER * ppContainer)
{
	// Tell the browser that we are a simple object and don't support a container
	*ppContainer = 0;

	return(E_NOINTERFACE);
}

HRESULT STDMETHODCALLTYPE ClientSite_ShowObject(IOleClientSite * This)
{
	return(NOERROR);
}

HRESULT STDMETHODCALLTYPE ClientSite_OnShowWindow(IOleClientSite * This, BOOL fShow)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE ClientSite_RequestNewObjectLayout(IOleClientSite * This)
{
	NOTIMPLEMENTED;
}



////////////////////////////////////// My IOleInPlaceSite functions  /////////////////////////////////////
// The browser object asks us for the pointer to our IOleInPlaceSite object by calling our IOleClientSite's
// QueryInterface (ie, ClientSite_QueryInterface) and specifying a REFIID of IID_IOleInPlaceSite.

HRESULT STDMETHODCALLTYPE InPlaceSite_QueryInterface(IOleInPlaceSite * This, REFIID riid, LPVOID* ppvObj)
{
	return(ClientSite_QueryInterface((IOleClientSite *)((CHAR *)This - sizeof(IOleClientSite)), riid, ppvObj));
}

HRESULT STDMETHODCALLTYPE InPlaceSite_AddRef(IOleInPlaceSite * This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE InPlaceSite_Release(IOleInPlaceSite * This)
{
	return(1);
}

HRESULT STDMETHODCALLTYPE InPlaceSite_GetWindow(IOleInPlaceSite * This, HWND * lphwnd)
{
	*lphwnd = ((COleInPlaceSite *)This)->m_InPlaceFrame.hWndOwner;

	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlaceSite_ContextSensitiveHelp(IOleInPlaceSite * This, BOOL fEnterMode)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE InPlaceSite_CanInPlaceActivate(IOleInPlaceSite * This)
{
	// Tell the browser we can in place activate
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlaceSite_OnInPlaceActivate(IOleInPlaceSite * This)
{
	// Tell the browser we did it ok
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlaceSite_OnUIActivate(IOleInPlaceSite * This)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlaceSite_GetWindowContext(IOleInPlaceSite * This, LPOLEINPLACEFRAME * lplpFrame, LPOLEINPLACEUIWINDOW * lplpDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	// Give the browser the pointer to our IOleInPlaceFrame struct
	*lplpFrame = (LPOLEINPLACEFRAME)&((COleInPlaceSite *)This)->m_InPlaceFrame;

	// We have no OLEINPLACEUIWINDOW
	*lplpDoc = 0;

	// Fill in some other info for the browser
	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->hwndFrame = ((COleInPlaceFrame *)*lplpFrame)->hWndOwner;
	lpFrameInfo->haccel = 0;
	lpFrameInfo->cAccelEntries = 0;

	// Give the browser the dimensions of where it can draw. We give it our entire window to fill.
	// We do this in InPlaceSite_OnPosRectChange() which is called right when a window is first
	// created anyway, so no need to duplicate it here.
	//	GetClientRect(lpFrameInfo->hwndFrame, lprcPosRect);
	//	GetClientRect(lpFrameInfo->hwndFrame, lprcClipRect);

	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlaceSite_Scroll(IOleInPlaceSite * This, SIZE scrollExtent)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE InPlaceSite_OnUIDeactivate(IOleInPlaceSite * This, BOOL fUndoable)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlaceSite_OnInPlaceDeactivate(IOleInPlaceSite * This)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE InPlaceSite_DiscardUndoState(IOleInPlaceSite * This)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE InPlaceSite_DeactivateAndUndo(IOleInPlaceSite * This)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE InPlaceSite_OnPosRectChange(IOleInPlaceSite * This, LPCRECT lprcPosRect)
{
	IOleObject			*browserObject;
	IOleInPlaceObject	*inplace;

	// We need to get the browser's IOleInPlaceObject object so we can call its SetObjectRects
	// function.
	browserObject = *((IOleObject **)((CHAR *)This - sizeof(IOleObject *) - sizeof(IOleClientSite)));
	if (SUCCEEDED(browserObject->lpVtbl->QueryInterface(browserObject, &IID_IOleInPlaceObject, (void**)&inplace)))
	{
		// Give the browser the dimensions of where it can draw.
		inplace->lpVtbl->SetObjectRects(inplace, lprcPosRect, lprcPosRect);
		inplace->lpVtbl->Release(inplace);
	}

	return(S_OK);
}



////////////////////////////////////// My IOleInPlaceFrame functions  /////////////////////////////////////////

HRESULT STDMETHODCALLTYPE Frame_QueryInterface(IOleInPlaceFrame* This, REFIID riid, LPVOID* ppvObj)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_AddRef(IOleInPlaceFrame* This)
{
	return 1;
}

HRESULT STDMETHODCALLTYPE Frame_Release(IOleInPlaceFrame* This)
{
	return 1;
}

HRESULT STDMETHODCALLTYPE Frame_GetWindow(IOleInPlaceFrame* This, HWND * lphwnd)
{
	*lphwnd = ((COleInPlaceFrame *)This)->hWndOwner;
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Frame_ContextSensitiveHelp(IOleInPlaceFrame* This, BOOL fEnterMode)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_GetBorder(IOleInPlaceFrame* This, LPRECT lprectBorder)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_RequestBorderSpace(IOleInPlaceFrame* This, LPCBORDERWIDTHS pborderwidths)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_SetBorderSpace(IOleInPlaceFrame* This, LPCBORDERWIDTHS pborderwidths)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_SetActiveObject(IOleInPlaceFrame* This, IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Frame_InsertMenus(IOleInPlaceFrame* This, HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_SetMenu(IOleInPlaceFrame* This, HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Frame_RemoveMenus(IOleInPlaceFrame* This, HMENU hmenuShared)
{
	NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Frame_SetStatusText(IOleInPlaceFrame* This, LPCOLESTR pszStatusText)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Frame_EnableModeless(IOleInPlaceFrame* This, BOOL fEnable)
{
	return(S_OK);
}

HRESULT STDMETHODCALLTYPE Frame_TranslateAccelerator(IOleInPlaceFrame* This, LPMSG lpmsg, WORD wID)
{
	NOTIMPLEMENTED;
}

/////////////////////////////////////////////////////////////////////////////
// UnEmbedBrowserObject
// Called to detach the browser object from our host window, and free its
// resources, right before we destroy our window.
// 
// hWnd  Handle to the window hosting the browser object.
//
// NOTE: The pointer to the browser object must have been stored in the
// window's USERDATA field. In other words, don't call UnEmbedBrowserObject().
// with a HWND that wasn't successfully passed to EmbedBrowserObject().
//

void UnEmbedBrowserObject(HWND hWnd)
{
	IOleObject	**browserHandle;
	IOleObject	*browserObject;

	// Retrieve the browser object's pointer we stored in our window's GWL_USERDATA when
	// we initially attached the browser object to this window.
	if ((browserHandle = (IOleObject **)GetWindowLong(hWnd, GWL_USERDATA)))
	{
		browserObject = *browserHandle;
		browserObject->lpVtbl->Close(browserObject, OLECLOSE_NOSAVE);
		browserObject->lpVtbl->Release(browserObject);

		GlobalFree(browserHandle);

		return;
	}

	_ASSERT(0);
}


#define WEBPAGE_GOBACK		0
#define WEBPAGE_GOFORWARD	1
#define WEBPAGE_GOHOME		2
#define WEBPAGE_SEARCH		3
#define WEBPAGE_REFRESH		4
#define WEBPAGE_STOP		5

/////////////////////////////////////////////////////////////////////////////
// DoPageAction
//
// Implements the functionality of a "Back". "Forward", "Home", "Search",
// "Refresh", or "Stop" button.
//
// hWnd =		Handle to the window hosting the browser object.
// dwAction =		One of the following:
//				0 = Move back to the previously viewed web page.
//				1 = Move forward to the previously viewed web page.
//				2 = Move to the home page.
//				3 = Search.
//				4 = Refresh the page.
//				5 = Stop the currently loading page.
//
// NOTE: EmbedBrowserObject() must have been successfully called once with the
// specified window, prior to calling this function. You need call
// EmbedBrowserObject() once only, and then you can make multiple calls to
// this function to display numerous pages in the specified window.
//

void DoPageAction(HWND hWnd, DWORD dwAction)
{	
	IWebBrowser2	*webBrowser2;
	IOleObject		*browserObject;

	// Retrieve the browser object's pointer we stored in our window's GWL_USERDATA when
	// we initially attached the browser object to this window.
	browserObject = *((IOleObject **)GetWindowLong(hWnd, GWL_USERDATA));

	if (SUCCEEDED(browserObject->lpVtbl->QueryInterface(browserObject, &IID_IWebBrowser2, (void**)&webBrowser2)))
	{
		switch (dwAction)
		{
		case WEBPAGE_GOBACK:
		{
			webBrowser2->lpVtbl->GoBack(webBrowser2);
			break;
		}

		case WEBPAGE_GOFORWARD:
		{
			webBrowser2->lpVtbl->GoForward(webBrowser2);
			break;
		}

		case WEBPAGE_GOHOME:
		{
			webBrowser2->lpVtbl->GoHome(webBrowser2);
			break;
		}

		case WEBPAGE_SEARCH:
		{
			webBrowser2->lpVtbl->GoSearch(webBrowser2);
			break;
		}

		case WEBPAGE_REFRESH:
		{
			webBrowser2->lpVtbl->Refresh(webBrowser2);
		}

		case WEBPAGE_STOP:
		{
			webBrowser2->lpVtbl->Stop(webBrowser2);
		}
		}

		webBrowser2->lpVtbl->Release(webBrowser2);
	}
}

/////////////////////////////////////////////////////////////////////////////
// DisplayHTMLStr
//
// Takes a string containing some HTML BODY, and displays it in the specified
// window. For example, perhaps you want to display the HTML text of...
//
// <P>This is a picture.<P><IMG src="mypic.jpg">
//
// hWnd =		Handle to the window hosting the browser object.
// string =		Pointer to nul-terminated string containing the HTML BODY.
//				(NOTE: No <BODY></BODY> tags are required in the string).
//
// RETURNS: 0 if success, or non-zero if an error.
//
// NOTE: EmbedBrowserObject() must have been successfully called once with the
// specified window, prior to calling this function. You need call
// EmbedBrowserObject() once only, and then you can make multiple calls to
// this function to display numerous pages in the specified window.
//

long DisplayHTMLStr(HWND hWnd, LPCSTR pszString)
{	
	IWebBrowser2	*webBrowser2;
	LPDISPATCH		lpDispatch;
	IHTMLDocument2	*htmlDoc2;
	IOleObject		*browserObject;
	SAFEARRAY		*sfArray;
	VARIANT			myURL;
	VARIANT			*pVar;
	BSTR			bstr;

	// Retrieve the browser object's pointer we stored in our window's GWL_USERDATA when
	// we initially attached the browser object to this window.
	browserObject = *((IOleObject **)GetWindowLong(hWnd, GWL_USERDATA));

	// Assume an error.
	bstr = 0;

	if (SUCCEEDED(browserObject->lpVtbl->QueryInterface(browserObject, &IID_IWebBrowser2, (void**)&webBrowser2)))
	{
		// Before we can get_Document(), we actually need to have some HTML page loaded in the browser. So,
		// let's load an empty HTML page. Then, once we have that empty page, we can get_Document() and
		// write() to stuff our HTML string into it.
		VariantInit(&myURL);
		myURL.vt = VT_BSTR;
		myURL.bstrVal = SysAllocString(L"about:blank");

		// Call the Navigate2() function to actually display the page.
		webBrowser2->lpVtbl->Navigate2(webBrowser2, &myURL, 0, 0, 0, 0);

		// Free any resources (including the BSTR).
		VariantClear(&myURL);

		if (SUCCEEDED(webBrowser2->lpVtbl->get_Document(webBrowser2, &lpDispatch)))
		{

			if (SUCCEEDED(lpDispatch->lpVtbl->QueryInterface(lpDispatch, &IID_IHTMLDocument2, (void**)&htmlDoc2)))
			{
				// Our HTML must be in the form of a BSTR. And it must be passed to write() in an
				// array of "VARIENT" structs. So let's create all that.
				if ((sfArray = SafeArrayCreate(VT_VARIANT, 1, (SAFEARRAYBOUND *)&ArrayBound)))
				{
					if (SUCCEEDED(SafeArrayAccessData(sfArray, (void**)&pVar)))
					{
						pVar->vt = VT_BSTR;
						{
							wchar_t		*buffer;
							DWORD		size;

							size = MultiByteToWideChar(CP_ACP, 0, pszString, -1, 0, 0);

							if (!(buffer = (wchar_t *)GlobalAlloc(GMEM_FIXED, sizeof(wchar_t) * size))) 
								goto bad;
							else
							{
								MultiByteToWideChar(CP_ACP, 0, pszString, -1, buffer, size);
								if (buffer)
									bstr = SysAllocString(buffer);
								GlobalFree(buffer);
							}
						}

						if (bstr)
						{
							// Store our BSTR pointer in the VARIENT.
							if ((pVar->bstrVal = bstr))
							{
								// Pass the VARIENT with its BSTR to write() in order to shove our desired HTML string
								// into the body of that empty page we created above.
								htmlDoc2->lpVtbl->write(htmlDoc2, sfArray);

								// Close the document. If we don't do this, subsequent calls to DisplayHTMLStr
								// would append to the current contents of the page
								htmlDoc2->lpVtbl->close(htmlDoc2);

								// Normally, we'd need to free our BSTR, but SafeArrayDestroy() does it for us
								//SysFreeString(bstr);
							}
						}
					}

					// Free the array. This also frees the VARIENT that SafeArrayAccessData created for us,
					// and even frees the BSTR we allocated with SysAllocString
					SafeArrayDestroy(sfArray);
				}

bad:			htmlDoc2->lpVtbl->Release(htmlDoc2);
			}
			lpDispatch->lpVtbl->Release(lpDispatch);
		}
		webBrowser2->lpVtbl->Release(webBrowser2);
	}

	if (bstr) 
	{
		SysFreeString(bstr);
		return(0);
	}


	return(-1);
}

/////////////////////////////////////////////////////////////////////////////
//

// Displays a URL, or HTML file on disk.
//
// hwnd =		Handle to the window hosting the browser object.
// webPageName =	Pointer to nul-terminated name of the URL/file.
//
// RETURNS: 0 if success, or non-zero if an error.
//
// NOTE: EmbedBrowserObject() must have been successfully called once with the
// specified window, prior to calling this function. You need call
// EmbedBrowserObject() once only, and then you can make multiple calls to
// this function to display numerous pages in the specified window.
//

long DisplayHTMLPage(HWND hWnd, LPCTSTR pszPageURL)
{
	IWebBrowser2	*webBrowser2;
	VARIANT			myURL;
	IOleObject		*browserObject;

	// Retrieve the browser object's pointer we stored in our window's GWL_USERDATA when
	// we initially attached the browser object to this window.
	browserObject = *((IOleObject **)GetWindowLong(hWnd, GWL_USERDATA));

	if (SUCCEEDED(browserObject->lpVtbl->QueryInterface(browserObject, &IID_IWebBrowser2, (void**)&webBrowser2)))
	{

		// Our URL (ie, web address, such as "http://www.microsoft.com" or an HTM filename on disk
		// such as "c:\myfile.htm") must be passed to the IWebBrowser2's Navigate2() function as a BSTR.
		// A BSTR is like a pascal version of a double-byte character string. In other words, the
		// first unsigned short is a count of how many characters are in the string, and then this
		// is followed by those characters, each expressed as an unsigned short (rather than a
		// CHAR). The string is not nul-terminated. The OS function SysAllocString can allocate and
		// copy a UNICODE C string to a BSTR. Of course, we'll need to free that BSTR after we're done
		// with it. If we're not using UNICODE, we first have to convert to a UNICODE string.
		//
		// What's more, our BSTR needs to be stuffed into a VARIENT struct, and that VARIENT struct is
		// then passed to Navigate2(). Why? The VARIENT struct makes it possible to define generic
		// 'datatypes' that can be used with all languages. Not all languages support things like
		// nul-terminated C strings. So, by using a VARIENT, whose first field tells what sort of
		// data (ie, string, float, etc) is in the VARIENT, COM interfaces can be used by just about
		// any language.
		VariantInit(&myURL);
		myURL.vt = VT_BSTR;

#ifndef UNICODE
		{
			wchar_t		*buffer;
			DWORD		size;

			size = MultiByteToWideChar(CP_ACP, 0, pszPageURL, -1, 0, 0);
			if (!(buffer = (wchar_t *)GlobalAlloc(GMEM_FIXED, sizeof(wchar_t) * size))) goto badalloc;
			MultiByteToWideChar(CP_ACP, 0, pszPageURL, -1, buffer, size);
			myURL.bstrVal = SysAllocString(buffer);
			GlobalFree(buffer);
		}
#else
		myURL.bstrVal = SysAllocString(pszPageURL);
#endif
		if (!myURL.bstrVal)
		{
#ifndef UNICODE
badalloc:	webBrowser2->lpVtbl->Release(webBrowser2);
			return(-6);
#endif
		}

		// Call the Navigate2() function to actually display the page.
		webBrowser2->lpVtbl->Navigate2(webBrowser2, &myURL, 0, 0, 0, 0);

		// Free any resources (including the BSTR we allocated above).
		VariantClear(&myURL);
		webBrowser2->lpVtbl->Release(webBrowser2);


		return(0);
	}

	return(-5);
}

/////////////////////////////////////////////////////////////////////////////
//

void ResizeBrowser(HWND hwnd, DWORD width, DWORD height)
{
	IWebBrowser2	*webBrowser2;
	IOleObject		*browserObject;

	// Retrieve the browser object's pointer we stored in our window's GWL_USERDATA when
	// we initially attached the browser object to this window.
	browserObject = *((IOleObject **)GetWindowLong(hwnd, GWL_USERDATA));


	if (SUCCEEDED(browserObject->lpVtbl->QueryInterface(browserObject, &IID_IWebBrowser2, (void**)&webBrowser2)))
	{
		webBrowser2->lpVtbl->put_Width(webBrowser2, width);
		webBrowser2->lpVtbl->put_Height(webBrowser2, height);
		webBrowser2->lpVtbl->Release(webBrowser2);
	}
}

/////////////////////////////////////////////////////////////////////////////
// EmbedBrowserObject
//
// Puts the browser object inside our host window, and save a pointer to this
// window's browser object in the window's GWL_USERDATA field.
//
// hWnd  Handle of our window into which we embed the browser object.
//
// RETURNS: 0 if success, or non-zero if an error.
//
// NOTE: We tell the browser object to occupy the entire client area of the
// window.
//
// NOTE: No HTML page will be displayed here. We can do that with a subsequent
// call to either DisplayHTMLPage() or DisplayHTMLStr(). This is merely once-only
// initialization for using the browser object. In a nutshell, what we do
// here is get a pointer to the browser object in our window's GWL_USERDATA
// so we can access that object's functions whenever we want, and we also pass
// the browser a pointer to our IOleClientSite struct so that the browser can
// call our functions in our struct's VTable.
//

long EmbedBrowserObject(HWND hWnd)
{
	LPCLASSFACTORY pClassFactory;
	IOleObject* browserObject;
	IWebBrowser2* webBrowser2;
	RECT rect;
	CHAR* ptr;
	COleClientSite* pClientSite;

	if (!(ptr = (CHAR*)GlobalAlloc(GMEM_FIXED, sizeof(COleClientSite) + sizeof(IOleObject*))))
		return(-1);

	pClientSite = (COleClientSite*)(ptr + sizeof(IOleObject*));

	pClientSite->m_OleClientSite.lpVtbl = &g_IOleClientSiteVtbl;
	pClientSite->m_InPlaceSite.m_OleInPlaceSite.lpVtbl = &g_IOleInPlaceSiteVtbl;
	pClientSite->m_InPlaceSite.m_InPlaceFrame.m_OleInPlaceFrame.lpVtbl = &g_IOleInPlaceFrameVtbl;
	pClientSite->m_InPlaceSite.m_InPlaceFrame.hWndOwner = hWnd;
	pClientSite->m_DocHostUI.m_DocHostUI.lpVtbl = &g_IDocHostUIHandlerVtbl;

	pClassFactory = 0;
	if (SUCCEEDED(CoGetClassObject(&CLSID_WebBrowser, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, NULL, &IID_IClassFactory, (void **)&pClassFactory)) && pClassFactory)
	{
		if (SUCCEEDED(pClassFactory->lpVtbl->CreateInstance(pClassFactory, 0, &IID_IOleObject, &browserObject)))
		{
			pClassFactory->lpVtbl->Release(pClassFactory);

			// Store data in window's GWL_USERDATA
			*((IOleObject **)ptr) = browserObject;
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)ptr);

			// Give the browser a pointer to IOleClientSite object
			if (SUCCEEDED(browserObject->lpVtbl->SetClientSite(browserObject, (IOleClientSite*)pClientSite)))
			{
				browserObject->lpVtbl->SetHostNames(browserObject, L"My Host Name", 0);

				GetClientRect(hWnd, &rect);

				// Let browser object know that it is embedded in an OLE container.
				if (SUCCEEDED(OleSetContainedObject((struct IUnknown *)browserObject, TRUE)) &&
					SUCCEEDED(browserObject->lpVtbl->DoVerb(browserObject, OLEIVERB_SHOW, NULL, (IOleClientSite *)pClientSite, -1, hWnd, &rect)) &&
					SUCCEEDED(browserObject->lpVtbl->QueryInterface(browserObject, &IID_IWebBrowser2, (void**)&webBrowser2)))
				{
					webBrowser2->lpVtbl->put_Left(webBrowser2, 0);
					webBrowser2->lpVtbl->put_Top(webBrowser2, 0);
					webBrowser2->lpVtbl->put_Width(webBrowser2, rect.right);
					webBrowser2->lpVtbl->put_Height(webBrowser2, rect.bottom);


					webBrowser2->lpVtbl->Release(webBrowser2);
					return(0);
				}
			}
			UnEmbedBrowserObject(hWnd);
			return(-4);
		}

		pClassFactory->lpVtbl->Release(pClassFactory);
		GlobalFree(ptr);
		return(-3);
	}

	GlobalFree(ptr);
	return(-2);
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
		{
			ResizeBrowser(hwnd, LOWORD(lParam), HIWORD(lParam));
			return 0;
		}

	case WM_CREATE:
		{
			if (EmbedBrowserObject(hwnd)) 
				return(-1);
			++g_nWindowCount;
			return 0;
		}

	case WM_DESTROY:
		{
			UnEmbedBrowserObject(hwnd);
			--g_nWindowCount;
			// For standalone application use the code:
			//if (!g_nWindowCount) 
				//PostQuitMessage(0);

			return(TRUE);
		}
	}

	return(DefWindowProc(hwnd, uMsg, wParam, lParam));
}

/////////////////////////////////////////////////////////////////////////////
//

int InitWebControl(HINSTANCE hInstance)
{

	if (OleInitialize(NULL) == S_OK)
	{
		WNDCLASSEX wc;

		ZeroMemory(&wc, sizeof(WNDCLASSEX));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_GLOBALCLASS; //!!!!!!
		wc.hInstance = hInstance;
		wc.lpfnWndProc = WindowProc;
		wc.lpszClassName = &szWebControlClassName[0];
		RegisterClassEx(&wc);

		// For standalone application in WinMain use the code:
/*
		if ((msg.hwnd = CreateWindowEx(0, 
			&ClassName[0], 
			_T("An HTML string"),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
			HWND_DESKTOP, NULL, hInstance, 0)))
		{
			DisplayHTMLStr(msg.hwnd, <... string here...>));
.
			ShowWindow(msg.hwnd, WS_SHOW);
			UpdateWindow(msg.hwnd);
		}

		// Create  window with another browser object embedded in it.
		if ((msg.hwnd = CreateWindowEx(0, 
			&ClassName[0], 
			_T("Microsoft's web site"), 
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
			HWND_DESKTOP, NULL, hInstance, 0)))
		{
			// For this window, display a URL. 
			// This could also be a HTML file on disk such as "c:\\myfile.htm".
			DisplayHTMLPage(msg.hwnd, _T("http://www.microsoft.com"));

			// Show the window.
			ShowWindow(msg.hwnd, nCmdShow);
			UpdateWindow(msg.hwnd);
		}

		while (GetMessage(&msg, 0, 0, 0) == 1)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		OleUninitialize();
		return 0 ;
*/
	}

	return (-1);
}

/////////////////////////////////////////////////////////////////////////////
//
void UninitWebControl()
{
	OleUninitialize();
}

/////////////////////////////////////////////////////////////////////////////
//

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		InitWebControl(hInstance);
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		UninitWebControl();
		break;

	case DLL_PROCESS_DETACH:
		//UninitWebControl();
		break;
	}
	return TRUE;
}
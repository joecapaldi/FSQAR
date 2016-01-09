

#ifndef __WEBCONT_H__
#define __WEBCONT_H__

#pragma once

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

#define WEBCONT_EXPORT __declspec(dllexport)

#define ALX_WEB_CONTROL_CLASS "AlxWebBrowserClass"

WEBCONT_EXPORT long EmbedBrowserObject(HWND hWnd);
WEBCONT_EXPORT void UnEmbedBrowserObject(HWND hWnd);
WEBCONT_EXPORT long DisplayHTMLPage(HWND hWnd, LPCTSTR pszPageURL);
WEBCONT_EXPORT long DisplayHTMLStr(HWND hWnd, LPCSTR string);
WEBCONT_EXPORT void DoPageAction(HWND hWnd, DWORD dwAction);
WEBCONT_EXPORT void ResizeBrowser(HWND hwnd, DWORD width, DWORD height);

#ifdef __cplusplus
}
#endif

#endif // __WEBCONT_H__
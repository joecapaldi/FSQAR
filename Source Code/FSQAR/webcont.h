

#ifndef __WEBCONT_H__
#define __WEBCONT_H__

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

#define WEBCONT_EXPORT __declspec(dllexport)

#define ALX_WEB_BROWSER_CLASS "AlxWebBrowserClass"

WEBCONT_EXPORT LONG EmbedBrowserObject(HWND hWnd);
WEBCONT_EXPORT VOID UnEmbedBrowserObject(HWND hWnd);
WEBCONT_EXPORT LONG DisplayHTMLPage(HWND hWnd, LPCTSTR pszPageURL);
WEBCONT_EXPORT LONG DisplayHTMLStr(HWND hWnd, LPCSTR string);
WEBCONT_EXPORT VOID DoPageAction(HWND hWnd, DWORD dwAction);
WEBCONT_EXPORT VOID ResizeBrowser(HWND hwnd, DWORD width, DWORD height);

#ifdef __cplusplus
}
#endif

#endif // __WEBCONT_H__
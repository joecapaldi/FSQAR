

#ifndef __ABOUTGPL_H__
#define __ABOUTGPL_H__

// The dialog box marking
#define STD_LARGE_ICON_HEIGHT	32
// Remember: 10 pixels == 20 Dlg Units
#define ICON_X					32
#define ICON_Y					10
#define APPNAME_X				ICON_X + 2*STD_LARGE_ICON_HEIGHT
#define BANNER_HEIGHT			ICON_Y + 2*STD_LARGE_ICON_HEIGHT

#define COLOR_APPNAME			RGB(45, 45, 128)	// Dark Blue
#define COLOR_BANNER			RGB(255, 255, 255)	// White

#define DS_STRETCH	1

typedef struct _PRODUCT_INFORMATION
{
	TCHAR szProductName[SHORT_STR_LEN];
	TCHAR szProductDescription[LONG_STR_LEN];
	TCHAR szAuthors[LONG_STR_LEN];
	UINT nMajorVersion;
	UINT nMinorVersion;
	UINT nRevision;
	HICON hIcon;
}PRODUCT_INFORMATION;
typedef PRODUCT_INFORMATION* LPPRODUCT_INFORMATION;

// -----------------------------------------------------------------------
// AboutFreewareDialog
//
// Create dialog which has a hyperlink control and can display WebControl.
// 
// Parameters:
// 
// hWndOwner	Handle of owner window
// pProductInfo Pointer to structure filled of information about software
// hBmpBanner	Handle of bitmap that represents the software
// dwDrawStyle	Unused
// -----------------------------------------------------------------------
BOOL AboutFreewareDialog(HWND hWndOwner, 
						 LPPRODUCT_INFORMATION pProductInfo, 
						 HBITMAP hBmpBanner,
						 DWORD dwDrawStyle);

#endif // __ABOUTGPL_H__
//////////////////////////////////////////////////////////////////////////
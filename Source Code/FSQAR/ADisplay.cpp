
#include "stdafx.h"
#include "fsqar.h"
#include "adisplay.h"


ADisplay::ADisplay()
{

}

ADisplay::~ADisplay()
{

}

BOOL ADisplay::Create(const Window* pFrameWnd, LPCRECT lpRect)
{
	return Window::Create(0, szADisplayWndClass, NULL, 
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		lpRect, pFrameWnd, 0, this);
}

LRESULT CALLBACK ADisplayWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	if (Msg == WM_CREATE)
	{
		// Pointer to a value to be passed to the window through 
		// the CREATESTRUCT structure is out window object instance.
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		ADisplay* pADisplay = static_cast<ADisplay*>(pcs->lpCreateParams);
		// Store the pointer as private data of the window
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, PtrToUlong(pADisplay)); 
		pADisplay->OnCreate();
		return 0;
	}

	// Extract the pointer to out window object
	ADisplay* pADisplay = static_cast<ADisplay*>(UlongToPtr(::GetWindowLongPtr(hWnd, GWLP_USERDATA)));

	if (pADisplay)
	{
		switch (Msg)
		{
		case WM_DISPLAYCHANGE:
		case WM_PAINT:
			BeginPaint(hWnd, &ps);
			pADisplay->OnPaint(ps.hdc);
			EndPaint(hWnd, &ps);
			return 0;
		}
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}


void ADisplay::OnCreate()
{

}

void ADisplay::OnPaint(HDC hDC)
{
	TextOut(hDC, 20, 20, _T("AIR-file"), 9);
}
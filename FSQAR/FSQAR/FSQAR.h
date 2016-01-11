
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

#ifndef __WINNAPP_H__
#define __WINNAPP_H__

// =======================================================================
// This is a monolithic header file that includes of all GUI application 
// classes declarations. It contains all needs to create a simple Windows 
// application which displays "live" data from some buffer. These classes 
// are not attached to any GUI library. These are not universal and intend 
// for this application only. These propose the simples and direct way to 
// create classical GUI application with a few windows.
// =======================================================================

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

#define SHORT_STR_LEN	48
#define LONG_STR_LEN	192		// For more length use MAX_PATH



//========================================================================
//				Version control
//========================================================================
#define APP_NAME				"FSQAR"
#define APP_NAME_ERROR			"FSQAR - Error"
#define APP_VERSION_MAJOR		1
#define APP_VERSION_MINOR		1
#define APP_VERSION_REVISION	0
#define VERSION_STR				"1.1.0"
#define APP_MAIN_KEY			_T("SOFTWARE\\Shag\\FSQAR\\1.0")


const TCHAR szMainWndClass[]		= _T("QARMainWndClass");
const TCHAR szNDisplayWndClass[]	= _T("QARDisplayWndClass");
const TCHAR szSDisplayWndClass[]	= _T("QARStaticDisplayWndClass");
const TCHAR szGDisplayWndClass[]	= _T("GDisplayWndClass");
const TCHAR szControlPadWndClass[]	= _T("QARControlPadWndClass");

// Prepar3D v3 Keys
const TCHAR szAppDataFolder[] = _T("%APPDATA%\\Lockheed Martin\\Prepar3D v3");
const TCHAR szAppRegistryKey_x86[] = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Lockheed Martin\\Prepar3D v3");
const TCHAR szAppRegistryKey_x64[] = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Lockheed Martin\\Prepar3D v3");

//=======================================================================
//				_newstring is analog of new operator
//=======================================================================

LPVOID _AllocateMemory(SIZE_T nSize, LPCTSTR pszFile, UINT nLine);
BOOL _FreeMemory(LPVOID lpAddress);

wchar_t* _newstringW(SIZE_T nSize, LPCTSTR pszFile, UINT nLine);
char* _newstringA(SIZE_T nSize, LPCTSTR pszFile, UINT nLine);
void _freestringW(wchar_t* string);
void _freestringA(char* string);

#ifdef _UNICODE
#define _newstring(s) _newstringW(s, __FILEW__, __LINE__)
#define _freestring(s) _freestringW(s)
#else
#define _newstring(s) _newstringA(s, __FILE__, __LINE__)
#define _freestring(s) _freestringA(s)
#endif // _UNICODE



// It should be zero-base index in ComboBox control !!!
#define DATAFILE_SEPARATOR_COMMA				0
#define DATAFILE_SEPARATOR_SEMICOLON			1
#define DATAFILE_SEPARATOR_SPACE				2
#define DATAFILE_SEPARATOR_TAB					3


// These values are adapted for screen resolution 640x480
#define FRAME_WIDTH			400	
#define FRAME_HEIGHT		520

#define DEFAULT_WINDOWS_X	200
#define DEFAULT_WINDOWS_Y	40
#define DEFAULT_WINDOWS_DX	FRAME_WIDTH
#define DEFAULT_WINDOWS_DY	FRAME_HEIGHT

//=======================================================================
//					The basic colors (16 items)
//=======================================================================

#define GREEN		RGB(0,255,0)
#define DKGREEN		RGB(0,128,0)
#define YELLOW		RGB(255,255,0)
#define LTYELLOW	RGB(255,255,128)
#define RED			RGB(255,0,0)
#define DKRED		RGB(128,0,0)
#define BLACK		RGB(0,0,1)
#define WHITE		RGB(255,255,255)
#define CYAN		RGB(0,255,255)
#define PURPLE		RGB(255,0,255)
#define GREY		RGB(192, 192,192)
#define BLUE		RGB(0,0,255)
#define ORANGE		RGB(255,127,0)
#define DKBLUE		RGB(0,0,127)
#define MAGENTA		RGB(255,0,255)
#define NAVY		RGB(0,0,128)

// These are used in NDisplay constructor
#define DISPLAY_BACKGROUND_COLOR	RGB(250, 252, 255);
#define DISPLAY_TEXT_COLOR			RGB(0, 0, 32);

//========================================================================
//					Grid options
//========================================================================

#define MAX_BUFFER_SIZE			512
#define MAX_CHART_COUNT			3

//========================================================================
//					Global scope critical sections
//========================================================================

extern CRITICAL_SECTION g_csAppData;	// protect GUI thread data
extern CRITICAL_SECTION g_csSimData;	// protect SimConnect data


#ifndef DISABLE_COPY_AND_ASSIGN
#define DISABLE_COPY_AND_ASSIGN(Object) \
	Object(const Object&); \
	const Object& operator=(const Object&);	
#endif

//========================================================================
//					Wrapper for Window critical section object
//========================================================================

//
// Automatically enters the critical section in the constructor and leaves
// the critical section in the destructor.
// 
class Lock 
{
public:
	// Creates a new instance with the given critical section object
	// and enters the critical section immediately.
	explicit Lock(CRITICAL_SECTION* cs) : cs_(cs), taken_(false) 
	{
		ASSERT(cs_);
		Acquire();
	}

	// Destructor: leaves the critical section.
	~Lock() 
	{
		if (taken_) 
			Release();
	}

	// Enters the critical section. Recursive Acquire() calls are not allowed.
	void Acquire() 
	{
		ASSERT(!taken_);
		EnterCriticalSection(cs_);
		taken_ = true;
	}

	// Leaves the critical section. The caller should not call Release() unless
	// the critical section has been entered already.
	void Release() 
	{
		ASSERT(taken_);
		taken_ = false;
		LeaveCriticalSection(cs_);
	}

private:

	DISABLE_COPY_AND_ASSIGN(Lock);

	CRITICAL_SECTION* cs_;
	bool taken_;
};

//========================================================================
//					Data buffer for graphical output
// INT_PTR should be used anywhere that a pointer is cast to an integer type.
// nLine - a line on the chart
// Value - a value of simulator data
// nPosition - the position of the value in the buffer
// MAX_CHART_COUNT how many lines on the chart (default: 3)
// MAX_BUFFER_SIZE how many values in the buffer (default: 512)
//========================================================================

class GraphData 
{
public:
	GraphData();

	INT_PTR GetSelectedItem(INT_PTR nPosition);
	LONG GetSize() { return m_nSize; }
	void SetBufferValue(INT_PTR nLine, INT_PTR nPosition, double Value);
	double GetBufferValue(INT_PTR nLine, INT_PTR nPosition);
	void SetSelectedChartValue(INT_PTR nPosition, INT_PTR nValue);

	// InterlockedExchangeAdd requires 'volatile' modifier
	volatile LONG* GetAddress() { return &m_nSize; }

#ifdef TEST_WITHOUT_SIM
	void AddValue(int nLine, double Value);
#endif

	void ResetBuffer(double InitValue);
	void SetSize(LONG nSize) { m_nSize = nSize; }

protected:
	volatile LONG m_nSize;
	INT_PTR m_nSelectedItem[MAX_CHART_COUNT];
	double m_dBuffer[MAX_CHART_COUNT][MAX_BUFFER_SIZE];
};

inline INT_PTR GraphData::GetSelectedItem(INT_PTR nLine) 
{ 
	ASSERT(nLine < MAX_CHART_COUNT);
	__analysis_assume(nLine < MAX_CHART_COUNT);
	return m_nSelectedItem[nLine]; 
}

inline void GraphData::SetBufferValue(int nLine, INT_PTR nPosition, double Value)
{
	ASSERT(nLine < MAX_CHART_COUNT);
	ASSERT(nPosition < MAX_BUFFER_SIZE);

	__analysis_assume(nLine < MAX_CHART_COUNT);
	__analysis_assume(nPosition < MAX_BUFFER_SIZE);
	m_dBuffer[nLine][nPosition] = Value;
}


inline double GraphData::GetBufferValue(INT_PTR nLine, INT_PTR nPosition)
{
	ASSERT(nLine < MAX_CHART_COUNT);
	ASSERT(nPosition < MAX_BUFFER_SIZE);

	__analysis_assume(nLine < MAX_CHART_COUNT);
	__analysis_assume(nPosition < MAX_BUFFER_SIZE);

	return m_dBuffer[nLine][nPosition];
}

inline void GraphData::SetSelectedChartValue(INT_PTR nLine, INT_PTR nValue)
{
	ASSERT(nLine < MAX_CHART_COUNT);
	__analysis_assume(nLine < MAX_CHART_COUNT);
	m_nSelectedItem[nLine] = nValue;
}


//========================================================================
//					Base class for application windows
//========================================================================

class Window 
{
public:
	Window();
	
	HWND GetSafeHwnd() const;
	void Attach(HWND hWnd);

	virtual BOOL CreateEx(DWORD dwExStyle,
		LPCTSTR lpClassName,
		LPCTSTR lpWindowName,
		DWORD dwStyle,
		int x,
		int y,
		int nWidth,
		int nHeight,
		HWND hWndParent,
		HMENU hMenu,
		Window* lpWndObject);

	virtual BOOL Create(DWORD dwExStyle, LPCTSTR lpszClassName, 
		LPCTSTR lpszWindowName, 
		DWORD dwStyle, 
		LPCRECT lpRect, 
		const Window* pParentWnd, 
		HMENU hMenu,
		const Window* lpWndObject);

	BOOL ShowWindow(int nCmdShow);
	BOOL UpdateWindow();
	int DestroyWindow();
	LRESULT SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	int GetWindowHeight();

	BOOL GetClientRect(LPRECT lpRect);
	BOOL InvalidateRect(LPRECT lpRect, BOOL bErase);
	void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType);

	void ScrollWindow(int xAmount, int yAmount, LPCRECT lpRect, LPCRECT lpClipRect);

	void CheckMenuItem(UINT uID, BOOL fCheck)
	{
		ASSERT(m_hMenu != NULL);
		::CheckMenuItem(m_hMenu, uID, fCheck ? MF_CHECKED : MF_UNCHECKED);
	}

	void SetTopMost(BOOL fTopMost)
	{
		ASSERT(::IsWindow(m_hWnd));
		::SetWindowPos(m_hWnd, fTopMost ? HWND_TOPMOST : HWND_NOTOPMOST, 
			0,0,0,0, 
			SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);
	}

	void CheckDlgButton(int nIDButton, UINT uCheck)
	{
		ASSERT(::IsWindow(m_hWnd));
		::CheckDlgButton(m_hWnd, nIDButton, uCheck);
	}

	void SetWindowText(LPCTSTR lpString)
	{
		ASSERT(::IsWindow(m_hWnd));
		::SetWindowText(m_hWnd, lpString);
	}

	LRESULT CALLBACK DefaultWindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);

protected:
	virtual ~Window();
	HWND m_hWnd;
	HFONT m_hFont;
	HMENU m_hMenu;

	
};

inline HWND Window::GetSafeHwnd() const
{ 
	return (this == NULL) ? NULL : m_hWnd; 
}

inline void Window::Attach(HWND hWnd)
{
	ASSERT(::IsWindow(hWnd));
	m_hWnd = hWnd;
}

inline BOOL Window::GetClientRect(LPRECT lpRect)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::GetClientRect(m_hWnd, lpRect);
}

inline BOOL Window::InvalidateRect(LPRECT lpRect, BOOL bErase)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::InvalidateRect(m_hWnd, lpRect, bErase);
}

inline LRESULT Window::SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::SendMessage(m_hWnd, Msg,  wParam,  lParam);
}

inline int Window::GetWindowHeight()
{
	// The GetWindowRect function retrieves the dimensions of the bounding 
	// rectangle of the specified window. The dimensions are given in screen 
	// coordinates that are relative to the upper-left corner of the screen.
	RECT rc;
	ASSERT(::IsWindow(m_hWnd));
	GetWindowRect(m_hWnd, &rc);
	return (rc.bottom - rc.top);
}

inline BOOL Window::ShowWindow(int nCmdShow)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::ShowWindow(m_hWnd, nCmdShow);
}

inline BOOL Window::UpdateWindow()
{
	ASSERT(::IsWindow(m_hWnd));
	return ::UpdateWindow(m_hWnd);
}

inline int Window::DestroyWindow()
{
	ASSERT(::IsWindow(m_hWnd));
	return ::DestroyWindow(m_hWnd);
}


//========================================================================
//				Standard Status Window
//========================================================================

#define ID_STATUSBAR			101
#define SBPARTS_NUMBER			3

#define DEFAULT_SBPART1_WIDTH	260
#define DEFAULT_SBPART2_WIDTH	600
#define DEFAULT_SBPART3_WIDTH	900

class StatusBar : public Window
{
public:
	StatusBar();

	BOOL Create(const Window* pFrame);
	void ShowStatus(int nPart, LPCTSTR pText)
	{
		ASSERT(IsWindow(m_hWnd));
		::SendMessage(m_hWnd, SB_SETTEXT, (WPARAM)(nPart | 0), (LPARAM)pText);
	}
};


//========================================================================
//				Standard Toolbar
//========================================================================

#define ID_TOOLBAR			102
#define BUTTON_SIZE			18 
#define TOOLBAR_HEIGHT		20

static HWND CreateStandartToolbar(HWND hWndParent);

class Toolbar : public Window
{ 
public: 
	Toolbar();
	virtual ~Toolbar();

	BOOL Create(const Window* pParent);
protected: 
	HIMAGELIST m_hImgList;
}; 

//========================================================================
//				Tabbed Window
//========================================================================

class TabWindow : public Window
{
public:
	TabWindow();
	virtual ~TabWindow();

	BOOL Create(const Window* pFrameWnd, LPCRECT lpRect);
	void AdjustRect(LPCRECT lpRect);
	BOOL InsertItem(UINT nResID, INT nImage);
protected:
	HIMAGELIST m_hImageList;
	void CreateImageList();
	void SetDefaultFont();
};

//========================================================================
//				Numerical Display
//========================================================================

#define XFIT					22	// fitting !!!

// Default count of lines printing on DC
#define DEFAULT_LINES			20

// Max. count of characters in a line
#define MAX_LINES_CHAR_COUNT	256

// It affects on horizontal scrollbar appearing
#define AVERAGE_LETTERS_COUNT	32

// Note: Only windows massages handlers are public
class NDisplay : public Window
{
public:

	NDisplay();
	virtual ~NDisplay();

	BOOL Create(const Window* pFrameWnd, LPCRECT lpRect);

	// Windows messages handlers
	BOOL OnCreate();
	void OnClose();
	void OnDestroy();
	void OnSize(UINT nWidth, UINT nHeight);
	void OnPaint(HDC hDC);
	void OnRightButtonUp(int xPos, int yPos);
	void OnHScroll(WPARAM wParam);
	void OnVScroll(WPARAM wParam);

private:
	
	// Current font
	LOGFONT m_lf;
	HFONT CreateDefaultFont();
	BOOL FontSelectDialog();

	// Size and colors
	UINT m_nWidth;
	UINT m_nHeight;
	COLORREF m_clrBackground;
	COLORREF m_clrText;

	//---------------------
	int m_yStep;
	int m_xStep;
	int m_yNewPos;    // new position
	int m_xNewPos;
	int m_nLinesCount;
	size_t m_nMaxLength;

};

//========================================================================
//				Grid Options
//========================================================================

#define MAX_X_COUNT			32
#define MAX_LINES_COUNT		23
#define MAX_LABEL_LENGTH	16

#define MAX_CHART_COUNT		3
#define DEFAULT_XSTEP		2	// this is a smoothness criterion
#define X_SHIFT_TO_CHART	24	// shift from client rectangle to the chart
#define Y_SHIFT_TO_CHART	32


//========================================================================
//				Graphical Display 
//========================================================================

#define INIT_BUFFER_VALUE	0.0
#define MIN_SCREEN_BUFFER_SIZE		64
#define MAX_SCREEN_BUFFER_SIZE		512
#define DEF_SCREEN_BUFFER_SIZE		128

class GDisplay : public Window
{
public:

	GDisplay();
	virtual ~GDisplay();

	// Window messages handlers
	BOOL Create(const Window* pFrameWnd, LPCRECT lpRect);
	void OnCreate();
	void OnSize(WPARAM wParam, LPARAM lParam);
	void OnPaint(HDC hDC);
	void OnDestroy();

	// Graphical buffer
	void ResetBuffer(double dInitValue = 0);	// Set initial value
	BOOL Update(GraphData* pGraphData);			// redraw the display

	// Types of the charts
	void SetChartType(enum CHART_TYPE nType);
	void UpdateChartType();

	// Export to an image
	void SaveAsBitmap(LPCTSTR pszFileName);
	void SaveAsJpeg(LPCTSTR pszFileName);

	// For maneuverable aircrafts
	BOOL IsManoeuverable() { return m_fManoeuverable; }
	void SetManoeuverable(BOOL fManoeuverable) { m_fManoeuverable = fManoeuverable; }

	LRESULT CALLBACK WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);

#ifdef TEST_WITHOUT_SIM
	// Timer for testing
	void StartTimer();
	void StopTimer();
#endif

private:

	BOOL m_fManoeuverable;
	int m_nScreenBufferSize;
	RECT m_rcChart;

	UINT_PTR m_nTimer;
	UINT m_xMinStep;			// min step on X direction

	// Labels of variables
	TCHAR m_szLabel1[MAX_LINES_COUNT][MAX_LABEL_LENGTH];
	TCHAR m_szLabel2[MAX_LINES_COUNT][MAX_LABEL_LENGTH];
	TCHAR m_szLabel3[MAX_LINES_COUNT][MAX_LABEL_LENGTH];


	int m_nChartCount;
	double m_dblMin[MAX_CHART_COUNT];	// Min value on the chart
	double m_dblMax[MAX_CHART_COUNT];	// Max value on the chart
	COLORREF m_clrMarker[MAX_CHART_COUNT];

	INT_PTR m_nSimVarID[MAX_CHART_COUNT];

	UINT m_xUpdatePosition;	// Current update position

	int m_dxText;	// Shift of text in X direction
	int m_dyText;	// Shift of text in Y direction

	int m_xCount;	// Count of X lines
	int m_yCount;	// Count of Y lines

	COLORREF m_clrBackDark;	
	COLORREF m_clrBackLight;
	COLORREF m_clrUpdateLine;
	COLORREF m_clrGrid;
	COLORREF m_clrBorder;
	COLORREF m_clrText;

	int m_nMarginTop;
	int m_nMarginBottom;
	int m_nMarginRight;

	WORD m_nClientWidth;
	WORD m_nClientHeight;

	double m_dScreenBuffer[MAX_CHART_COUNT][MAX_BUFFER_SIZE];

	enum CHART_TYPE m_nChartType;
	void ChartForNonManoeuverable(__in enum CHART_TYPE nType);
	void ChartForManoeuverable(__in enum CHART_TYPE nType);
	void SaveOptions();
	void ReadOptions();

	void DrawMarker(HDC hDC, LPPOINT ppOld, LPPOINT ppNew, int nRightBorder, COLORREF clrMarker);
	void DrawGrid(HDC hDC, LPRECT lpRect);
	void DrawChart(HDC hDC, LPRECT lpRect, INT xShift, INT yShift);
	void DrawUpdateLine(HDC hDC, UINT xUpdatePosition, LPRECT lpRect);
	void DrawValueLine(HDC hDC, double Value, int yPos, LPRECT lpRect);
	void DrawDescription(HDC hDC, LPRECT lpRect);
	void DrawLogo(HDC hDC, int x, int y);
	
};


//========================================================================
//				Static Data Display 
//========================================================================

#define FAVORITES_COUNT		8	// count of data that may be favorites

// List of integers support (nonspecific data storage)
// NOTE: We have to include this file as a separated header
#include "IntList.h"

class SDisplay : public Window
{ 
public: 
	SDisplay();
	virtual ~SDisplay();

	BOOL Create(const Window* pFrameWnd, LPCRECT lpRect);

	BOOL OnCreate();
	void OnSize(UINT_PTR nWidth, UINT_PTR nHeight);
	void OnPaint(HDC hDC);
	void OnDestroy();
	VOID OnRightButtonUp(int xPos, int yPos);

	// You must create ChartDlg object to make this member private
	IntList m_listFavorites;

	void AddToListBox(HWND hListBox);

private: 

	HFONT CreateDefaultFont();
	void DoPaint(HDC hDC, LPRECT lpRect);
	void PrintDouble(HDC hDC, int xPos, int yPos, double Value);

	UINT_PTR m_nWidth;
	UINT_PTR m_nHeight;
	COLORREF m_clrBackground;
	COLORREF m_clrText;
	int m_yStep;
	int m_xStep;
	int m_nLinesCount;

}; 

//========================================================================
//				How to get Window class by the window type 
//========================================================================

enum CHILD_WINDOW
{
	GDISPLAY,
	NDISPLAY,
	SDISPLAY,
	TABWINDOW,
	CTRLPAD,
	STATUSBAR,
	TOOLBAR
};

//========================================================================
//				Named constants for charts 
//========================================================================

enum CHART_TYPE
{
	CHART_TYPE_LANDING,
	CHART_TYPE_ANGLES,
	CHART_TYPE_PITCH,
	CHART_TYPE_ROLL,
	CHART_TYPE_YAW,
	CHART_TYPE_THRUST,
	CHART_TYPE_SIMPLE
};


//========================================================================
//				ControlPad
//========================================================================

class ControlPad : public Window
{ 
public: 
	ControlPad();
	virtual ~ControlPad();

	BOOL Create(const Window* pParent, LPCRECT lpRect);

	void SetControlDeflection();
	void SendToSimulator();

	void OnCreate();
	void OnSize(UINT nWidth, UINT nHeight);
	void OnPaint(HDC hDC);
	void OnRightButtonUp(UINT xPos, UINT yPos);
	void OnMouseMove(int xPos, int yPos);
	void OnKeyDown(UINT_PTR nKeyCode);

	LRESULT CALLBACK WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);

private: 

	static const DWORD dwControlPadWndStyle = WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION ;

	void DrawBackground(HDC hDC);
	void DrawCross(HDC hDC);
	void DrawTextLabels(HDC hDC);
	void DrawHint(HDC hDC);
	void DrawControl(HDC hDC, LPPOINT pptPos);

	void SetDefaultFont();

	UINT m_nWidth; 
	UINT m_nHeight;
	BOOL m_fAction;
	POINT m_ptCurrent;
};



//========================================================================
//				Frame Window (Main application window)
//========================================================================

class FrameWnd : public Window
{
public:
	FrameWnd();
	virtual ~FrameWnd();


	Window* GetChildWindow(enum CHILD_WINDOW childWindow) const;

	// Window message handlers
	BOOL OnCreate();						// WM_CREATE	
	void OnSize(UINT nWidth, UINT nHeight);	// WM_SIZE			
	void OnClose();							// WM_CLOSE
	void OnDestroy();						// WM_DESTROY
	void OnMouseWheel(WPARAM wParam);		// WM_MOUSEWHEEL
	void OnKeyDown(WPARAM wParam);			// WM_KEYDOWN
	void OnHelp();							// WM_HELP
	void OnNotify(WPARAM wParam, LPARAM lParam);

	// TabWindow Notification
	void OnTabChange();						// TCN_SELCHANGE

	// Menu items handlers
	void OnFileExit();
	void OnViewOptions();
	void OnHelpIndex();
	void OnHelpAbout();
	void OnControlPad();
	void OnScreenshot();
	void OnViewToolbar();
	void OnViewStatus();
	void OnPlaceTopMost();
	void SetChartType(enum CHART_TYPE nType);	// Menu 'Chart Type'

	// bad code (copy-paste)
	NDisplay* GetNDisplay() const { return m_pNDisplay; }
	GDisplay* GetGDisplay() const { return m_pGDisplay; }
	SDisplay* GetSDisplay() const { return m_pSDisplay; }
	ControlPad* GetCtrlPad()const { return m_pCtrlPad; }
	StatusBar* GetStatus() const  { return m_pStatus; }
	TabWindow* GetTabWindow()const{ return m_pTabWnd; }


	// Tray icon support
	BOOL GetState() { return m_uState; }
	void SetState(BOOL fState) { m_uState = fState; }
	void ChangeState();
	BOOL TrayMessage(DWORD dwMessage, HICON hIcon, LPCTSTR pszText);

	void ShowStartupDialog();

	void OnManageOutputFolder();
private:

	void Resize();

	BOOL CreatePropertySheet(__in HINSTANCE hInstance);
	

	Toolbar* m_pToolbar;		// standard toolbar object
	StatusBar* m_pStatus;		// standard status object
	TabWindow* m_pTabWnd;		// tabbed window object
	NDisplay* m_pNDisplay;		// numeric readout object
	GDisplay* m_pGDisplay;		// graphic display object
	SDisplay* m_pSDisplay;		// static data display
	ControlPad* m_pCtrlPad;		// control pad object

	BOOL m_uState;

	int nWindowPosX;
	int nWindowPosY;
	int nWindowPosDX;
	int nWindowPosDY;

	BOOL m_fShowStatus;
	BOOL m_fShowToolbar;
	BOOL m_fTopMost;

	void SaveOptions();
	void ReadOptions();
	void SetOptions();
};


//========================================================================
//					Forward declaration of SimClient class
//========================================================================


class CSimClient;

//========================================================================
//					Windows GUI Application Object
//========================================================================

// Tray icon support
#define WM_NOTIFYICON	WM_APP + 100	// handled message
#define IDC_NOTIFY		5412			// abstract number

class QARApplication
{
public:

	// Singleton implementation
	static QARApplication* GetInstance();
	static void FreeInstance();
	// -----------------------------------

	HINSTANCE Instance() { return m_hInstance; }	
	LPCTSTR Name() { return m_pszAppName; }

	// -------- interaction with simulator --------------
	DWORD SimThreadProc(LPCSTR pszSessionName);	
	BOOL BeginSession();
	void EndSession();
	void SendTerminationSignal();
	void UpdateAllDisplays();

	void OnSimContinue();
	void OnSimPause();
	// ---------------------------------------------------

	// ==== bad code (COPY-PASTE) ======
	FrameWnd* GetMainWnd() const 
	{ 
		ASSERT(m_pFrameWnd);
		return m_pFrameWnd; 
	}

	GDisplay* GetGDisplay() const
	{ 
		ASSERT(m_pFrameWnd);
		return m_pFrameWnd->GetGDisplay();
	}

	NDisplay* GetNDisplay() const
	{ 
		ASSERT(m_pFrameWnd);
		return m_pFrameWnd->GetNDisplay();
	}

	SDisplay* GetSDisplay() const
	{ 
		ASSERT(m_pFrameWnd);
		return m_pFrameWnd->GetSDisplay();
	}

	ControlPad* GetCtrlPad() const
	{ 
		ASSERT(m_pFrameWnd);
		return m_pFrameWnd->GetCtrlPad();
	}

	TabWindow* GetTabWindow() const
	{ 
		ASSERT(m_pFrameWnd);
		return m_pFrameWnd->GetTabWindow();
	}

	StatusBar* GetStatus() const
	{ 
		ASSERT(m_pFrameWnd);
		return m_pFrameWnd->GetStatus();
	}
	// ===================================

	void Error(UINT uResID);
	BOOL InitApplication(HINSTANCE hInstance);
	BOOL InitInstance(LPTSTR lpszCmdLine, int nCmdShow);
	INT Run();

	BOOL OpenFileDialog(LPCTSTR lpszFileName);

	void SetActiveWnd(enum CHILD_WINDOW ActiveWnd);
	HWND GetActiveWnd() { return m_hActiveWnd; };
	void SetCaption(LPCTSTR lpString);

	CSimClient* GetSimClient() const { return m_pSimClient; }

protected:

	//
	// Locked object is not required 'volatile' 
	// http://alenacpp.blogspot.ru/2006/04/volatile.html
	// 
	static QARApplication* volatile theApp;
	static LONG m_nRefCount;

	QARApplication();
	~QARApplication();

	DISABLE_COPY_AND_ASSIGN(QARApplication);

private:

	static const DWORD dwMainWndStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	
	BOOL RegisterWindowClass(WNDPROC lpfnWndProc, LPCTSTR pszIconName,  LPCTSTR pszMenuName,  LPCTSTR pszClassName);


	static LPCTSTR m_pszAppName;
	static HINSTANCE m_hInstance;

	MSG m_uMsg;
	HWND m_hActiveWnd;
	
	FrameWnd* m_pFrameWnd;		// Main application window

	CSimClient* m_pSimClient;

	BOOL ZipOutputFolder();
};


//========================================================================
//				Global variables
//========================================================================

// TODO: Make it a class member
extern GraphData g_GraphData;

// TODO: Make it a class member
extern double g_lever_pos;

//========================================================================
//				Global scope functions
//========================================================================

QARApplication* AfxGetApp();		// access to whole 'Singleton' object
HINSTANCE AfxGetInstanceHandle();	// access to 'Singleton' object data
LPCTSTR AfxGetAppName();


// For thread debugging
DWORD GetThisProcessID();
BOOL ListProcessThreads(DWORD dwOwnerPID);

size_t MyStrLength(LPCTSTR pString, size_t cbMax);


BOOL IsFolderExists(LPCTSTR pwsFolderName);
void BorderRect(HDC hDC, LPRECT lpRect, COLORREF clrPenColor);
void MyFillRect( HDC hDC, LPRECT lpRect, COLORREF Color);
void MyTextOut(HDC hDC, int xPos, int yPos,LPCTSTR pText, size_t nSize,  COLORREF BkColor, COLORREF TextColor);
void Paint3DShadow(HDC hDC, LPRECT lpRect);
PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp);
BOOL CreateBMPFile(LPCTSTR pszFile, PBITMAPINFO pbi,  HBITMAP hBMP,  HDC hDC);
LPTSTR GetFileExt(LPTSTR pFile);
BOOL FileExists(LPCTSTR pszFileName);
LPCTSTR GetFileName(LPCTSTR pFile);
INT_PTR GetFileDir(LPCTSTR pFile,  LPTSTR szFileDir,  size_t nFileDirLen);
HICON MyLoadIcon(LPCTSTR lpIconName);
int MyLoadString(UINT uID,LPTSTR lpBuffer, int cchBufferMax);
BOOL LoadWindowPos(LPINT lpX, LPINT lpY, LPINT lpDX, LPINT lpDY);
BOOL SaveWindowPos(HWND hWnd, int nWindowPosX,  int nWindowPosY, int nWindowPosDX,  int nWindowPosDY);


BOOL SearchFont(LPCTSTR pszFontName);
BOOL SaveFontToRegistry(const LOGFONT* lpLogFont);
BOOL LoadFontFromRegistry(LOGFONT* lpLogFont);

intptr_t _wcstombsz(__out_bcount(count) char* mbstr, 
				   __in const wchar_t* wcstr, 
				   __in size_t count);

intptr_t _mbstowcsz(__out_ecount_z(count) wchar_t* wcstr, 
				   __in const char* mbstr, 
				   __in size_t count);

BOOL CenterWindow(HWND hwndChild, HWND hwndParent);

LRESULT CALLBACK OutputFolderDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////////////////////////////

#endif // __WINNAPP_H__


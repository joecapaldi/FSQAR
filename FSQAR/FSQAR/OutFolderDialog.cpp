
#include "stdafx.h"
#include "fsqar.h"
#include "resource.h"

// Debug version of CRT library uses this operator
#ifdef _DEBUG
#ifndef DEBUG_NEW
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new DEBUG_NEW
#endif
#endif  // _DEBUG

class OutputFolderManager
{
public:
	OutputFolderManager();
	~OutputFolderManager();
	void OnCreate(HWND hDlg);

	void OnDestroy();

	void SetStyle(DWORD dwNewStyle);

	void OnDblClick();

private:
	HWND m_hListView;
	HIMAGELIST m_hImageLarge;
	HIMAGELIST m_hImageSmall;

	int InsertItem(LPTSTR pszText, int nItemIndex);
	BOOL FillListView();
};

OutputFolderManager::OutputFolderManager()
{
}

OutputFolderManager::~OutputFolderManager()
{
}

void OutputFolderManager::SetStyle(DWORD dwNewStyle)
{
	// Retrieve the current window style. 
	DWORD dwStyle = GetWindowLong(m_hListView, GWL_STYLE);
	// Only set the window style if the view bits have changed. 
	if ((dwStyle & LVS_TYPEMASK) != dwNewStyle)
		SetWindowLong(m_hListView, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | dwNewStyle);
}

typedef struct tagPETINFO
{
	char szKind[10];
	char szBreed[50];
	char szPrice[20];
}PETINFO;


PETINFO rgPetInfo[] =
{
	{ "Dog", "Poodle", "$300.00" },
	{ "Cat", "Siamese", "$100.00" },
	{ "Fish", "Angel Fish", "$10.00" },
};

void OutputFolderManager::OnCreate(HWND hDlg)
{
	ASSERT(::IsWindow(hDlg));
	m_hListView = GetDlgItem(hDlg, IDC_GZIP_LIST);
	
	// Must be 'single select' list view
	SetStyle(LVSIL_NORMAL | LVS_SINGLESEL);


	m_hImageLarge = ImageList_Create(GetSystemMetrics(SM_CXICON), 
		GetSystemMetrics(SM_CYICON), ILC_MASK, 1, 1);
	ASSERT(m_hImageLarge);
	if (m_hImageLarge)
	{
		HICON hIcon = MyLoadIcon(MAKEINTRESOURCE(IDI_GZIP));
		if (hIcon)
			ImageList_AddIcon(m_hImageLarge, hIcon);
	}

	ListView_SetImageList(m_hListView, m_hImageLarge, LVSIL_NORMAL);


	m_hImageSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON), 
		GetSystemMetrics(SM_CXSMICON), ILC_MASK, 1, 1);
	ASSERT(m_hImageSmall);
	if (m_hImageSmall)
	{
		HICON hIcon = MyLoadIcon(MAKEINTRESOURCE(IDI_GZIP));
		if (hIcon)
		if (ImageList_AddIcon(m_hImageSmall, hIcon) == -1)
		{
			TRACE0("error here\n");
		}
	}

	if (ListView_SetImageList(m_hListView, m_hImageSmall, LVSIL_SMALL) == NULL)
	{
		TRACE0("error here\n");
	}

	//----------------------------------------------------------
	FillListView();
	//------------------------------------------------------

}

int OutputFolderManager::InsertItem(LPTSTR pszText, int nItemIndex)
{
	LVITEM lvi;
	::ZeroMemory(&lvi, sizeof(LVITEM));

	lvi.mask = LVIF_TEXT | LVIF_IMAGE;
	//StringCbCopy(lvi.pszText, MAX_PATH, pszText);
	lvi.pszText = pszText;
	lvi.iImage = 0;
	lvi.iSubItem = 0;
	lvi.iItem = nItemIndex;
	int index = ListView_InsertItem(m_hListView, &lvi);
	if (index == -1)
	{
		TRACE0("eror here\n");
	}

	return index;
}


BOOL OutputFolderManager::FillListView()
{
	LPCTSTR m_pszOutputFolder = _T("C:\\Users\\alex\\Documents\\Prepar3D v3 Files\\QAR\\Jan_2016");
	int index = 0;

	if (m_pszOutputFolder)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		TCHAR szTemplate[2 * MAX_PATH];

		if (FAILED(StringCbPrintf(szTemplate, 2 * MAX_PATH, _T("%s\\*.gz"), m_pszOutputFolder)))
			return FALSE;

		hFind = FindFirstFile(szTemplate, &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			TRACE1("Invalid File Handle. GetLastError reports %d\n", GetLastError());
			return FALSE;
		}
		else
		{
			InsertItem(FindFileData.cFileName, index);

			while (FindNextFile(hFind, &FindFileData) != 0)
			{
				++index;
				InsertItem(FindFileData.cFileName, index);
			}

			DWORD dwError = GetLastError();
			FindClose(hFind);
			if (dwError != ERROR_NO_MORE_FILES)
			{
				TRACE1("FindNextFile error. Error is %u\n", dwError);
				return FALSE;
			}

			return TRUE;
		}
	}

	return TRUE;
}

void OutputFolderManager::OnDestroy()
{
	if (m_hImageLarge)
		ImageList_Destroy(m_hImageLarge);
}

void OutputFolderManager::OnDblClick()
{
	TCHAR szBuffer[MAX_PATH];

	int index = ListView_GetSelectionMark(m_hListView);
	if (index != -1)
	{
		ListView_GetItemText(m_hListView, index, 0, szBuffer, MAX_PATH);
		{
			::MessageBox(GetTopWindow(NULL), szBuffer, L"", 0);
		}
	}
}

HBITMAP hbm1;
HBITMAP hbm2;

LRESULT CALLBACK OutputFolderDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdcMem;
	LPDRAWITEMSTRUCT lpdis;

	static OutputFolderManager* pManager;

	switch (Msg)
	{
	case WM_INITDIALOG:
		pManager = new OutputFolderManager();
		pManager->OnCreate(hDlg);

		hbm1 = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LIST_LARGE));
		hbm2 = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LIST_LARGE));

		return TRUE;

	case WM_DRAWITEM:
		lpdis = (LPDRAWITEMSTRUCT)lParam;
		hdcMem = CreateCompatibleDC(lpdis->hDC);

		if (lpdis->itemState & ODS_SELECTED)  // if selected 
			SelectObject(hdcMem, hbm2);
		else
			SelectObject(hdcMem, hbm1);

		// Destination 
		StretchBlt(
			lpdis->hDC,         // destination DC 
			lpdis->rcItem.left, // x upper left 
			lpdis->rcItem.top,  // y upper left 

			// The next two lines specify the width and 
			// height. 
			lpdis->rcItem.right - lpdis->rcItem.left,
			lpdis->rcItem.bottom - lpdis->rcItem.top,
			hdcMem,    // source device context 
			0, 0,      // x and y upper left 
			32,        // source bitmap width 
			32,        // source bitmap height 
			SRCCOPY);  // raster operation 

		DeleteDC(hdcMem);
		return TRUE;

	case WM_DESTROY:
		DeleteObject(hbm1); 
		DeleteObject(hbm2);
		return FALSE;

	case WM_NOTIFY:
	{
					  LPNMHDR pnmhdr = (LPNMHDR)lParam;
					  switch (pnmhdr->code)
					  {
					  case NM_DBLCLK:
						  if (pnmhdr->idFrom == IDC_GZIP_LIST)
						  {
							  pManager->OnDblClick();
						  }
						  break;

					  default:
						  break;
					  }
	}
		// If the message handler is in a dialog box procedure, 
		// you must use the SetWindowLong function with DWL_MSGRESULT 
		// to set a return value.
		//return 
			SetWindowLong(hDlg, DWL_MSGRESULT, 0);
			break;
		//return FALSE;

	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			EndDialog(hDlg, TRUE);
			pManager->OnDestroy();
			delete pManager;
			break;

		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			pManager->OnDestroy();
			delete pManager;
			break;

			if (HIWORD(wParam) == BN_CLICKED)
			{
				switch (LOWORD(wParam))
				{
				case IDC_VIEW_LARGE:

					// application-defined processing 
					TextOut(hdcMem, 2, 2, L"gdhf", 5);
					break;
				}
			}

		case IDC_VIEW_LARGE:
			pManager->SetStyle(LVSIL_NORMAL);
			break;

		case IDC_VIEW_SMALL:
			pManager->SetStyle(LVSIL_SMALL);
			break;

		case IDC_VIEW_LIST:
			pManager->SetStyle(LVSIL_STATE);
			break;

		case IDC_VIEW_COLUMNS:
			pManager->SetStyle(LVSIL_GROUPHEADER);
			break;

		}
		return TRUE;
	}

	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	return FALSE;
}


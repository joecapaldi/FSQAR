
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
#include "SimVarIDS.h"

#include "resource.h"

void InitChartDialog(HWND hDlg);
void AddToChart(HWND hDlg);
void RemoveFromChart(HWND hDlg);

//
// TODO: IMPORTANT!!! Property 'Sort' of the IDC_AVAILABLE_DATA list box
// must be set to 'False'
// 
LRESULT CALLBACK ChartOptionsDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		InitChartDialog(hDlg);
		return TRUE;

	case WM_COMMAND: 
		switch (LOWORD(wParam)) 
		{ 
		case IDOK: 
			EndDialog(hDlg, IDOK); 
			return TRUE;

		case IDCANCEL: 
			EndDialog(hDlg, IDCANCEL); 
			return TRUE;

		case IDC_ADD_TO_CHART:
			AddToChart(hDlg);
			break;

		case IDC_REMOVE_FROM_CHART:
			RemoveFromChart(hDlg);
			break;
		} 
	}

	UNREFERENCED_PARAMETER(lParam);
	return FALSE;
}


void InitChartDialog(HWND hDlg)
{
	TCHAR szString[LONG_STR_LEN];

	// -------------------- available data -------------------------------
	for (int k = 0; k < SIM_VARS_COUNT; k++)
	{
		UINT id = SIMVAR_BASE_ID + k;
		if(MyLoadString(id, szString, LONG_STR_LEN))
		{
			SendMessage(GetDlgItem(hDlg, IDC_AVAILABLE_DATA), LB_ADDSTRING,
				0, (LPARAM)szString);
		}
	}
	// -------------------- selected  data -------------------------------
	SDisplay* pSDisplay = AfxGetApp()->GetSDisplay();
	pSDisplay->AddToListBox(GetDlgItem(hDlg, IDC_SELECTED_DATA));
	// -------------------- set scroll range -------------------------------
	SendMessage(GetDlgItem(hDlg, IDC_AVAILABLE_DATA), 
		LB_SETHORIZONTALEXTENT , (WPARAM)400, 0);

	SendMessage(GetDlgItem(hDlg, IDC_SELECTED_DATA), 
		LB_SETHORIZONTALEXTENT , (WPARAM)400, 0);

}

void AddToChart(HWND hDlg)
{
	int nSelectedItem = (int)SendMessage(GetDlgItem(hDlg, IDC_AVAILABLE_DATA),
		LB_GETCURSEL, 0, 0);

	SDisplay* pSDisplay = AfxGetApp()->GetSDisplay();
	if (pSDisplay->m_listFavorites.getCount() < 12)
	{
		int new_index = ++nSelectedItem;
		BOOL fAlreadyInList = FALSE;

		for (int i = 0; i < pSDisplay->m_listFavorites.getCount(); i++)
		{
			if (pSDisplay->m_listFavorites.getAt(i) == new_index)
				fAlreadyInList = TRUE;
		}

		if (!fAlreadyInList)
		{
			HWND hListBox = GetDlgItem(hDlg, IDC_SELECTED_DATA);
			pSDisplay->m_listFavorites.insert(new_index);
			SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
			pSDisplay->AddToListBox(hListBox);
		} 
		else
		{
			AfxGetApp()->Error(IDS_ALREADY_IN_LIST);
		}

	}
	else
	{
		AfxGetApp()->Error(IDS_TOO_MANY_ITEMS);
	}

}

void RemoveFromChart(HWND hDlg)
{
	HWND hListBox = GetDlgItem(hDlg, IDC_SELECTED_DATA);
	// Get the zero-based index of the currently selected item
	int nSelectedItem = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
	// Retrieve the value stored in the list box item
	int nFavorite = (int)SendMessage(hListBox, LB_GETITEMDATA, nSelectedItem, 0);

	SDisplay* pSDisplay = AfxGetApp()->GetSDisplay();
	pSDisplay->m_listFavorites.remove(nFavorite);
	SendMessage(GetDlgItem(hDlg, IDC_SELECTED_DATA), LB_RESETCONTENT, 0, 0);
	pSDisplay->AddToListBox(hListBox);
	
}

//////////////////////////////////////////////////////////////////////////

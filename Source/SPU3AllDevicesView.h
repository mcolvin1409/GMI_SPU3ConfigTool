
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="SPU3AllDevicesView.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include "afxcmn.h"
#include "afxeditbrowsectrl.h"
#include "afxbutton.h"

// CSPU3AllDevicesView form view

class CSPU3AllDevicesView : public CDialogEx
{
	DECLARE_DYNCREATE(CSPU3AllDevicesView)

public:
	CSPU3AllDevicesView( CWnd* pParent =NULL );          // protected constructor used by dynamic creation
	virtual ~CSPU3AllDevicesView();

	enum { IDD = IDD_ALL_DEVICES_FORM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LRESULT OnPopulateAllDevicesReport( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl m_allDevicesConfigListCtrl;
	CMFCEditBrowseCtrl m_folderBrowseCtrl;
	CToolTipCtrl m_toolTipControl;

private:
	virtual BOOL OnInitDialog();
	void LoadInitialSettings();	
	void AddHeaderToListControl();		
	CString GetReportFileName();
	void SetReportFolderPath();		
	bool CanCreateHistoryFile();
	void CreateToolTipControl();
		
private:
	afx_msg void OnGenerateReport();
	afx_msg void OnCreateHistoryFile();	
	CMFCButton m_createHistFileBtnCtrl;
	CMFCButton m_generateReportBtnCtrl;
public:
	virtual BOOL PreTranslateMessage( MSG* pMsg );
};




// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="DockableDialogBar.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include "afxeditbrowsectrl.h"
#include "afxwin.h"

// CDockableDialogBar Dialog

class CDockableDialogBar : public CDialogEx
{
	DECLARE_DYNAMIC(CDockableDialogBar)

public:
	CDockableDialogBar(CWnd* pParent = NULL);    // standard constructor
	virtual ~CDockableDialogBar();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOGBAR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	// Mercury System XML File
	CMFCEditBrowseCtrl m_fileBrowseXML;
	CMFCButton m_loadButton;
	CToolTipCtrl m_toolTipControl;

private:
	afx_msg void OnBnClickedLoadButton();
	void UpdateLoadButton();
	void CreateToolTipControl();

public:	
	virtual BOOL OnInitDialog();		
	afx_msg void OnEnChangeXmlFileBrowse();
	virtual BOOL PreTranslateMessage( MSG* pMsg );
};



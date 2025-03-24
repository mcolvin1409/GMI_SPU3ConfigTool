
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="DockableDialogBarPane.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include "afxdockablepane.h"
#include "DockableDialogBar.h"

class CDockableDialogBarPane :	public CDockablePane
{
	DECLARE_DYNAMIC( CDockableDialogBarPane );

public:
	CDockableDialogBarPane();
	virtual ~CDockableDialogBarPane();
	virtual BOOL IsResizable() const;

private:
	CDockableDialogBar m_wndDlgBar;

protected:
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void OnSize( UINT nType, int cx, int cy );

	DECLARE_MESSAGE_MAP()
};


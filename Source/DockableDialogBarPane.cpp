// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="DockableDialogBarPane.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#include "stdafx.h"
#include "DockableDialogBarPane.h"
#include "resource.h" 

IMPLEMENT_DYNAMIC( CDockableDialogBarPane, CDockablePane )

BEGIN_MESSAGE_MAP( CDockableDialogBarPane, CDockablePane )
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/// <summary>
/// Initializes a new instance of the <see cref="CDockableDialogBarPane"/> class.
/// </summary>
CDockableDialogBarPane::CDockableDialogBarPane()
{ }

/// <summary>
/// Finalizes an instance of the <see cref="CDockableDialogBarPane"/> class.
/// </summary>
CDockableDialogBarPane::~CDockableDialogBarPane()
{ }

/// <summary>
/// Called when [create].
/// </summary>
/// <param name="lpCreateStruct">The lp create structure.</param>
/// <returns></returns>
int CDockableDialogBarPane::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( CDockablePane::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	// Create the dialog bar  
	if( !m_wndDlgBar.Create( IDD_DIALOGBAR, this ) )
	{
		return -1;
	}

	return 0;
}

/// <summary>
/// Called when [size].
/// </summary>
/// <param name="nType">Type of the n.</param>
/// <param name="cx">The cx.</param>
/// <param name="cy">The cy.</param>
void CDockableDialogBarPane::OnSize( UINT nType, int cx, int cy )
{
	CDockablePane::OnSize( nType, cx, cy );
	m_wndDlgBar.SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
}

/// <summary>
/// Determines whether this instance is resizable.
/// </summary>
/// <returns></returns>
BOOL CDockableDialogBarPane::IsResizable() const
{
	return FALSE;
}
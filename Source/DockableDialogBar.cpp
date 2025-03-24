// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="DockableDialogBar.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

// DockableDialogBar.cpp : implementation file
//
#include "stdafx.h"
#include "SPU3AutoIPConfigTool.h"
#include "DockableDialogBar.h"
#include "Defines.h"

// CDockableDialogBar
IMPLEMENT_DYNAMIC(CDockableDialogBar, CDialogEx)

/// <summary>
/// Initializes a new instance of the <see cref="CDockableDialogBar"/> class.
/// </summary>
/// <param name="pParent">The p parent.</param>
CDockableDialogBar::CDockableDialogBar( CWnd* pParent /*=NULL*/ )
	: CDialogEx( IDD_DIALOGBAR, pParent )
{ }

/// <summary>
/// Finalizes an instance of the <see cref="CDockableDialogBar"/> class.
/// </summary>
CDockableDialogBar::~CDockableDialogBar()
{ }

/// <summary>
/// Does the data exchange.
/// </summary>
/// <param name="pDX">The data exchange.</param>
void CDockableDialogBar::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_XML_FILE_BROWSE, m_fileBrowseXML );
	DDX_Control( pDX, IDC_LOAD_BUTTON, m_loadButton );
}

BEGIN_MESSAGE_MAP( CDockableDialogBar, CDialogEx )
	ON_BN_CLICKED( IDC_LOAD_BUTTON, &CDockableDialogBar::OnBnClickedLoadButton )
	ON_EN_CHANGE( IDC_XML_FILE_BROWSE, &CDockableDialogBar::OnEnChangeXmlFileBrowse )
END_MESSAGE_MAP()

/// <summary>
/// Called when [initialize dialog].
/// </summary>
/// <returns></returns>
BOOL CDockableDialogBar::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CreateToolTipControl();

	m_fileBrowseXML.EnableFileBrowseButton( _T( "XML" ), _T( "System XML File|*.XML||" ) );
	UpdateLoadButton();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

/// <summary>
/// Creates the tool tip control.
/// </summary>
void CDockableDialogBar::CreateToolTipControl()
{
	if( m_toolTipControl.Create( this ) )
	{
		m_toolTipControl.AddTool( &m_loadButton, _T( "Loads the System XML File selected." ) );
	}
}

// CDockableDialogBar message handlers
/// <summary>
/// Called when [bn clicked load button].
/// </summary>
void CDockableDialogBar::OnBnClickedLoadButton()
{
	CString xmlFileName = _T( "" );
	//get the selected System XML file name
	m_fileBrowseXML.GetWindowText( xmlFileName );

	if( xmlFileName.IsEmpty() )
	{
		AfxMessageBox( _T( "System XML file path is Empty. Please select a System XML file." ), MB_ICONERROR );
		return;
	}

	// Is System XML file exists?
	if( PathFileExists( xmlFileName ) )
	{
		CFrameWnd *pWnd = GetParentFrame();
		if( NULL != pWnd )
		{
			// Post message to the Main Window to load the System XML file
			pWnd->PostMessage( AM_LOAD_SYSTEM_XML_FILE, 0, ( LPARAM )( new CString( xmlFileName ) ) );
		}
	}
}

/// <summary>
/// Called when [change XML file browse].
/// </summary>
void CDockableDialogBar::OnEnChangeXmlFileBrowse()
{
	UpdateLoadButton();
}

/// <summary>
/// Updates the load button.
/// </summary>
void CDockableDialogBar::UpdateLoadButton()
{
	CString xmlFileName = _T( "" );
	//get the selected System XML file name
	m_fileBrowseXML.GetWindowText( xmlFileName );
	// disable the Load button when System XML file name is empty or XML File not selected yet
	m_loadButton.EnableWindow( !xmlFileName.IsEmpty() );
}

/// <summary>
/// translate message.
/// </summary>
/// <param name="pMsg">The MSG.</param>
/// <returns></returns>
BOOL CDockableDialogBar::PreTranslateMessage( MSG* pMsg )
{
	if( ( pMsg->message == WM_LBUTTONDOWN ) ||
		( pMsg->message == WM_LBUTTONUP ) ||
		( pMsg->message == WM_MOUSEMOVE ) )
	{
		m_toolTipControl.RelayEvent( pMsg );
	}

	return CDialogEx::PreTranslateMessage( pMsg );
}

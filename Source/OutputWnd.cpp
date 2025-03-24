// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="OutputWnd.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "Defines.h"
#include <memory>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

/// <summary>
/// Initializes a new instance of the <see cref="COutputWnd"/> class.
/// </summary>
COutputWnd::COutputWnd()
{ }

/// <summary>
/// Finalizes an instance of the <see cref="COutputWnd"/> class.
/// </summary>
COutputWnd::~COutputWnd()
{ }

BEGIN_MESSAGE_MAP( COutputWnd, CDockablePane )
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_MESSAGE( AM_DISPLAY_LOG_WINDOW_MSG, &COutputWnd::OnDisplayLogWindowMessage )
END_MESSAGE_MAP()

/// <summary>
/// Called when [create].
/// </summary>
/// <param name="lpCreateStruct">The lp create structure.</param>
/// <returns></returns>
int COutputWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( CDockablePane::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create tabs window:
	if( !m_wndTabs.Create( CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1 ) )
	{
		TRACE0( "Failed to create output tab window\n" );
		return -1;      // fail to create
	}

	// Create output panes:
	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

	if( !m_wndOutputDebug.Create( dwStyle, rectDummy, &m_wndTabs, 2 ) )
	{
		TRACE0( "Failed to create output windows\n" );
		return -1;      // fail to create
	}

	UpdateFonts();

	CString strTabName = _T( "" );
	// Attach list windows to tab:
	BOOL bNameValid = strTabName.LoadString( IDS_DEBUG_TAB );
	ASSERT( bNameValid );
	m_wndTabs.AddTab( &m_wndOutputDebug, strTabName, ( UINT )0 );

	return 0;
}

/// <summary>
/// Called when [size].
/// </summary>
/// <param name="nType">Type of the n.</param>
/// <param name="cx">The cx.</param>
/// <param name="cy">The cy.</param>
void COutputWnd::OnSize( UINT nType, int cx, int cy )
{
	CDockablePane::OnSize( nType, cx, cy );

	// Tab control should cover the whole client area:
	m_wndTabs.SetWindowPos( NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );
}

/// <summary>
/// Adjusts the horz scroll.
/// </summary>
/// <param name="wndListBox">The WND ListBox.</param>
void COutputWnd::AdjustHorzScroll( CListBox& wndListBox )
{
	CClientDC dc( this );
	CFont* pOldFont = dc.SelectObject( &afxGlobalData.fontRegular );

	int cxExtentMax = 0;

	for( int i = 0; i < wndListBox.GetCount(); i++ )
	{
		CString strItem;
		wndListBox.GetText( i, strItem );

		cxExtentMax = max( cxExtentMax, ( int )dc.GetTextExtent( strItem ).cx );
	}

	wndListBox.SetHorizontalExtent( cxExtentMax );
	dc.SelectObject( pOldFont );
}

/// <summary>
/// Updates the fonts.
/// </summary>
void COutputWnd::UpdateFonts()
{
	m_wndOutputDebug.SetFont( &afxGlobalData.fontRegular );
}

/// <summary>
/// Called when [display log window message].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT COutputWnd::OnDisplayLogWindowMessage( WPARAM wParam, LPARAM lParam )
{
	std::unique_ptr< CString > pMessageText( ( CString * )lParam );
	m_wndOutputDebug.AddString( *pMessageText );

	return 0;
}

/// <summary>
/// Determines whether this instance is resizable.
/// </summary>
/// <returns></returns>
BOOL COutputWnd::IsResizable() const
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// COutputList1

/// <summary>
/// Initializes a new instance of the <see cref="COutputList"/> class.
/// </summary>
COutputList::COutputList()
{ }

/// <summary>
/// Finalizes an instance of the <see cref="COutputList"/> class.
/// </summary>
COutputList::~COutputList()
{ }

BEGIN_MESSAGE_MAP( COutputList, CListBox )
	ON_WM_CONTEXTMENU()
	ON_COMMAND( ID_EDIT_COPY, OnEditCopy )
	ON_COMMAND( ID_EDIT_CLEAR, OnEditClear )
	ON_COMMAND( ID_VIEW_OUTPUTWND, OnViewOutput )
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList message handlers

/// <summary>
/// Called when [context menu].
/// </summary>
/// <param name="">The .</param>
/// <param name="point">The point.</param>
void COutputList::OnContextMenu( CWnd* /*pWnd*/, CPoint point )
{
	CMenu menu;
	menu.LoadMenu( IDR_OUTPUT_POPUP );

	CMenu* pSumMenu = menu.GetSubMenu( 0 );

	if( AfxGetMainWnd()->IsKindOf( RUNTIME_CLASS( CFrameWndEx ) ) )
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if( !pPopupMenu->Create( this, point.x, point.y, ( HMENU )pSumMenu->m_hMenu, FALSE, TRUE ) )
			return;

		( ( CFrameWndEx* )AfxGetMainWnd() )->OnShowPopupMenu( pPopupMenu );
		UpdateDialogControls( this, FALSE );
	}

	SetFocus();
}

/// <summary>
/// Called when [edit copy].
/// </summary>
void COutputList::OnEditCopy()
{ }

/// <summary>
/// Called when [edit clear].
/// </summary>
void COutputList::OnEditClear()
{
	this->ResetContent();
}

/// <summary>
/// Called when [view output].
/// </summary>
void COutputList::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST( CDockablePane, GetOwner() );
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST( CMDIFrameWndEx, GetTopLevelFrame() );

	if( pMainFrame != NULL && pParentBar != NULL )
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane( pParentBar, FALSE, FALSE, FALSE );
		pMainFrame->RecalcLayout();
	}
}


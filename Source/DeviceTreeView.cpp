// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="DeviceTreeView.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#include "stdafx.h"
#include "MainFrm.h"
#include "DeviceTreeView.h"
#include "Resource.h"
#include "SPU3AutoIPConfigTool.h"
#include "Defines.h"
#include "MigrationConstants.h"

/// <summary>
/// class CDeviceTreeViewMenuButton 
/// </summary>
/// <seealso cref="CMFCToolBarMenuButton" />
class CDeviceTreeViewMenuButton : public CMFCToolBarMenuButton
{
	friend class CDeviceTreeView;

	DECLARE_SERIAL( CDeviceTreeViewMenuButton )

public:
	/// <summary>
	/// Initializes a new instance of the <see cref="CDeviceTreeViewMenuButton"/> class.
	/// </summary>
	/// <param name="hMenu">The h menu.</param>
	CDeviceTreeViewMenuButton( HMENU hMenu = NULL ) : CMFCToolBarMenuButton( ( UINT )-1, hMenu, -1 )
	{ }

	virtual void OnDraw( CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
						 BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE )
	{
		pImages = CMFCToolBar::GetImages();

		CAfxDrawState ds;
		pImages->PrepareDrawImage( ds );

		CMFCToolBarMenuButton::OnDraw( pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons );

		pImages->EndDrawImage( ds );
	}
};

IMPLEMENT_SERIAL( CDeviceTreeViewMenuButton, CMFCToolBarMenuButton, 1 )

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/// <summary>
/// The string no devices found
/// </summary>
const tstring STR_NO_DEVICES_FOUND = _T( "No Devices Found" );
/// <summary>
/// The string all devices
/// </summary>
const tstring STR_ALL_DEVICES = _T( "All Devices" );

/// <summary>
/// Initializes a new instance of the <see cref="CDeviceTreeView"/> class.
/// </summary>
CDeviceTreeView::CDeviceTreeView()
{
	m_treeItemToDeviceMap.clear();
}

/// <summary>
/// Finalizes an instance of the <see cref="CDeviceTreeView"/> class.
/// </summary>
CDeviceTreeView::~CDeviceTreeView()
{ }

BEGIN_MESSAGE_MAP( CDeviceTreeView, CDockablePane )
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND( ID_REFRESH_TREE, OnRefresh )
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_MESSAGE( AM_ADD_DEVICE_TO_TREE, &CDeviceTreeView::OnAddDeviceToTree )
	ON_MESSAGE( AM_UPDATE_TREE_ITEM_IMAGE, &CDeviceTreeView::OnUpdateTreeItemImage )
	ON_MESSAGE( AM_UPDATE_TREE_ITEM_DATA, &CDeviceTreeView::OnUpdateTreeItemData )
	ON_NOTIFY( TVN_SELCHANGED, IDC_DEVICE_TREE_CTRL, &CDeviceTreeView::OnTreeItemSelectionChanged )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClassView message handlers

/// <summary>
/// Called when [create].
/// </summary>
/// <param name="lpCreateStruct">The lp create structure.</param>
/// <returns></returns>
int CDeviceTreeView::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( CDockablePane::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create views:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_SHOWSELALWAYS | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if( !m_wndDeviceTreeView.Create( dwViewStyle, rectDummy, this, IDC_DEVICE_TREE_CTRL ) )
	{
		TRACE0( "Failed to create Device Tree View\n" );
		return -1;      // fail to create
	}

	// Load images:
	m_wndToolBar.Create( this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_TREE );
	m_wndToolBar.LoadToolBar( IDR_TREE, 0, 0, TRUE /* Is locked */ );

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle( m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY );
	m_wndToolBar.SetPaneStyle( m_wndToolBar.GetPaneStyle() & ~( CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT ) );

	m_wndToolBar.SetOwner( this );

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame( FALSE );

	return 0;
}

/// <summary>
/// Called when [size].
/// </summary>
/// <param name="nType">Type of the n.</param>
/// <param name="cx">The cx.</param>
/// <param name="cy">The cy.</param>
void CDeviceTreeView::OnSize( UINT nType, int cx, int cy )
{
	CDockablePane::OnSize( nType, cx, cy );
	AdjustLayout();
}

/// <summary>
/// Called when [context menu].
/// </summary>
/// <param name="pWnd">The Window.</param>
/// <param name="point">The point.</param>
void CDeviceTreeView::OnContextMenu( CWnd* pWnd, CPoint point )
{
	CTreeCtrl* pWndTree = ( CTreeCtrl* )&m_wndDeviceTreeView;
	ASSERT_VALID( pWndTree );

	if( pWnd != pWndTree )
	{
		CDockablePane::OnContextMenu( pWnd, point );
		return;
	}

	if( point != CPoint( -1, -1 ) )
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient( &ptTree );

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest( ptTree, &flags );
		if( hTreeItem != NULL )
		{
			pWndTree->SelectItem( hTreeItem );
		}
	}

	pWndTree->SetFocus();
	CMenu menu;
	menu.LoadMenu( IDR_POPUP_SORT );

	CMenu* pSumMenu = menu.GetSubMenu( 0 );

	if( AfxGetMainWnd()->IsKindOf( RUNTIME_CLASS( CMDIFrameWndEx ) ) )
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if( !pPopupMenu->Create( this, point.x, point.y, ( HMENU )pSumMenu->m_hMenu, FALSE, TRUE ) )
			return;

		( ( CMDIFrameWndEx* )AfxGetMainWnd() )->OnShowPopupMenu( pPopupMenu );
		UpdateDialogControls( this, FALSE );
	}
}

/// <summary>
/// Adjusts the layout.
/// </summary>
void CDeviceTreeView::AdjustLayout()
{
	if( GetSafeHwnd() == NULL )
	{
		return;
	}

	CRect rectClient;
	GetClientRect( rectClient );

	int cyTlb = m_wndToolBar.CalcFixedLayout( FALSE, TRUE ).cy;

	m_wndToolBar.SetWindowPos( NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER );
	m_wndDeviceTreeView.SetWindowPos( NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER );
}

/// <summary>
/// translate message.
/// </summary>
/// <param name="pMsg">The MSG.</param>
/// <returns></returns>
BOOL CDeviceTreeView::PreTranslateMessage( MSG* pMsg )
{
	return CDockablePane::PreTranslateMessage( pMsg );
}

/// <summary>
/// Called when [refresh].
/// </summary>
void CDeviceTreeView::OnRefresh()
{
	CFrameWnd *pWnd = GetParentFrame();
	if( NULL != pWnd )
	{
		pWnd->PostMessage( AM_QUERY_SPU3_DEVICES, 0, 0 );
	}
}

/// <summary>
/// Refreshes this instance.
/// </summary>
void CDeviceTreeView::Refresh()
{
	// Delete all items 
	m_wndDeviceTreeView.DeleteAllItems();
	m_treeItemToDeviceMap.clear();

	// Add root node as "No Devices Found" ( initial state )
	//	before any SPU3 devices are discovered on the Network	
	AddNoDevicesFoundNode();
}

/// <summary>
/// Called when [paint].
/// </summary>
void CDeviceTreeView::OnPaint()
{
	CPaintDC dc( this ); // device context for painting

	CRect rectTree;
	m_wndDeviceTreeView.GetWindowRect( rectTree );
	ScreenToClient( rectTree );

	rectTree.InflateRect( 1, 1 );
	dc.Draw3dRect( rectTree, ::GetSysColor( COLOR_3DSHADOW ), ::GetSysColor( COLOR_3DSHADOW ) );
}

/// <summary>
/// Called when [set focus].
/// </summary>
/// <param name="pOldWnd">The old WND.</param>
void CDeviceTreeView::OnSetFocus( CWnd* pOldWnd )
{
	CDockablePane::OnSetFocus( pOldWnd );

	m_wndDeviceTreeView.SetFocus();
}

/// <summary>
/// Called when [change visual style].
/// </summary>
void CDeviceTreeView::OnChangeVisualStyle()
{
	m_treeViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if( !bmp.LoadBitmap( uiBmpId ) )
	{
		TRACE( _T( "Can't load bitmap: %x\n" ), uiBmpId );
		ASSERT( FALSE );
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap( &bmpObj );

	UINT nFlags = ILC_MASK;

	nFlags |= ( theApp.m_bHiColorIcons ) ? ILC_COLOR24 : ILC_COLOR4;

	m_treeViewImages.Create( 16, bmpObj.bmHeight, nFlags, 0, 0 );
	m_treeViewImages.Add( &bmp, RGB( 255, 0, 0 ) );

	m_wndDeviceTreeView.SetImageList( &m_treeViewImages, TVSIL_NORMAL );

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap( theApp.m_bHiColorIcons ? IDB_TREE_24 : IDR_TREE, 0, 0, TRUE /* Locked */ );
}

/// <summary>
/// Called when [tree item selection changed].
/// </summary>
/// <param name="pNMHDR">The NMHDR.</param>
/// <param name="pResult">The result.</param>
void CDeviceTreeView::OnTreeItemSelectionChanged( NMHDR* pNMHDR, LRESULT* pResult )
{
	NM_TREEVIEW* pNMTreeView = ( NM_TREEVIEW* )pNMHDR;
	ASSERT( pNMTreeView );

	TVITEM itemOld{ }, itemNew{ };
	itemOld = pNMTreeView->itemOld;
	itemNew = pNMTreeView->itemNew;

	if( itemOld.hItem != itemNew.hItem )
	{
		CString itemText = m_wndDeviceTreeView.GetItemText( itemNew.hItem );

		// TODO: Display a prompt message to ask user, if the current changes have to be saved first??
		if( m_treeItemToDeviceMap.find( itemNew.hItem ) != m_treeItemToDeviceMap.end() )
		{
			CString macAddress = m_treeItemToDeviceMap[ itemNew.hItem ].first;
			// Post the message AM_DEVICE_SEL_CHANGE_TREE to the Main Window to notify the Tree item selection has changed			
			CFrameWnd *pWnd = GetParentFrame();
			if( NULL != pWnd )
			{
				pWnd->PostMessage( AM_DEVICE_SEL_CHANGE_TREE, 0, ( LPARAM )( new CString( macAddress ) ) );
			}
		}
		else if( 0 == itemText.CompareNoCase( STR_ALL_DEVICES.c_str() ) )
		{
			// Post the message AM_DISPLAY_ALL_DEVICES_VIEW to the Main Window to notify the Tree item selection has changed			
			CFrameWnd *pWnd = GetParentFrame();
			if( NULL != pWnd )
			{
				pWnd->PostMessage( AM_DISPLAY_ALL_DEVICES_VIEW, 0, 0 );
			}
		}
	}

	*pResult = 0;
}

/// <summary>
/// Adds the no devices found node.
/// </summary>
void CDeviceTreeView::AddNoDevicesFoundNode()
{
	HTREEITEM hRoot = m_wndDeviceTreeView.InsertItem( STR_NO_DEVICES_FOUND.c_str(), 0, 0 );
	ASSERT( hRoot );
	m_wndDeviceTreeView.SetItemState( hRoot, TVIS_BOLD, TVIS_BOLD );

	CFrameWnd *pWnd = GetParentFrame();
	if( NULL != pWnd )
	{
		pWnd->PostMessage( AM_DISPLAY_NO_DEVICES_VIEW, 0, 0 );
	}
}

/// <summary>
/// Handles the first device configuration record.
/// </summary>
void CDeviceTreeView::HandleFirstDeviceConfigRecord()
{
	int nodeCount = m_wndDeviceTreeView.GetCount();
	if( 1 == nodeCount )
	{
		HTREEITEM hRoot = m_wndDeviceTreeView.GetRootItem();
		ASSERT( hRoot );
		CString itemText = m_wndDeviceTreeView.GetItemText( hRoot );
		if( 0 == itemText.CompareNoCase( STR_NO_DEVICES_FOUND.c_str() ) )
		{
			if( m_wndDeviceTreeView.DeleteItem( hRoot ) )
			{
				m_wndDeviceTreeView.InsertItem( STR_ALL_DEVICES.c_str(), 0, 0 );
				m_wndDeviceTreeView.SetItemState( hRoot, TVIS_BOLD, TVIS_BOLD );
			}
		}
	}
}

/// <summary>
/// Determines whether [is duplicate device] [the specified mac address].
/// </summary>
/// <param name="MACAddress">The mac address.</param>
/// <returns>
///   <c>true</c> if [is duplicate device] [the specified mac address]; otherwise, <c>false</c>.
/// </returns>
bool CDeviceTreeView::IsDuplicateDevice( CString& MACAddress )
{
	HTREEITEM rootItem = m_wndDeviceTreeView.GetRootItem();
	ASSERT( rootItem );

	bool duplicateFound = false;

	// Search all of the children 
	if( m_wndDeviceTreeView.ItemHasChildren( rootItem ) )
	{
		HTREEITEM hNextItem{ };
		HTREEITEM hChildItem = m_wndDeviceTreeView.GetChildItem( rootItem );

		while( NULL != hChildItem )
		{
			hNextItem = m_wndDeviceTreeView.GetNextItem( hChildItem, TVGN_NEXT );

			TVITEM item{ };
			TCHAR szText[ 1024 ] = { '\0' };
			item.hItem = hChildItem;
			item.mask = ( TVIF_TEXT | TVIF_HANDLE );
			item.pszText = szText;
			item.cchTextMax = 1024;

			// Get the current item
			m_wndDeviceTreeView.GetItem( &item );

			// Is MAC Address found?
			if( -1 != CString( szText ).Find( MACAddress, 0 ) )
			{
				duplicateFound = true;
				break;
			}

			// Move to next item
			hChildItem = hNextItem;
		}
	}

	return duplicateFound;
}

/// <summary>
/// Called when [add device to tree].
/// </summary>
/// <param name="wParam">The wParameter.</param>
/// <param name="lParam">The lParameter.</param>
/// <returns></returns>
LRESULT CDeviceTreeView::OnAddDeviceToTree( WPARAM wParam, LPARAM lParam )
{
	HandleFirstDeviceConfigRecord();

	std::unique_ptr< DeviceConfigRecord > pDeviceConfigRecord( ( DeviceConfigRecord * )lParam );
	ASSERT( pDeviceConfigRecord );

	CString MACAddress = _T( "" );
	MACAddress.Format( _T( "%02X-%02X-%02X-%02X-%02X-%02X" ),
					   pDeviceConfigRecord->mac_address[ 0 ],
					   pDeviceConfigRecord->mac_address[ 1 ],
					   pDeviceConfigRecord->mac_address[ 2 ],
					   pDeviceConfigRecord->mac_address[ 3 ],
					   pDeviceConfigRecord->mac_address[ 4 ],
					   pDeviceConfigRecord->mac_address[ 5 ] );

	// No duplicates allowed
	// SPU3	Device MAC Address will be unique always (AFAIK)
	if( IsDuplicateDevice( MACAddress ) )
	{
		return 0; // Just return from this method 
	}

	CString IPAddress = _T( "" );
	IPAddress.Format( _T( "%d.%d.%d.%d" ),
					  pDeviceConfigRecord->ip_Addr[ 0 ],
					  pDeviceConfigRecord->ip_Addr[ 1 ],
					  pDeviceConfigRecord->ip_Addr[ 2 ],
					  pDeviceConfigRecord->ip_Addr[ 3 ] );

	CString deviceInfo = _T( "" );
	deviceInfo.Format( _T( "%s [%s]" ), IPAddress, MACAddress );

	// Get the Root Node
	HTREEITEM hRoot = m_wndDeviceTreeView.GetRootItem();

	// Add the Device Node
	TVINSERTSTRUCT tvInsert{ };
	tvInsert.hParent = hRoot;
	tvInsert.hInsertAfter = NULL;
	tvInsert.item.mask = ( TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE );
	tvInsert.item.pszText = deviceInfo.GetBuffer( deviceInfo.GetLength() );
	tvInsert.item.lParam = 0;
	tvInsert.item.iImage = 1;
	tvInsert.item.iSelectedImage = 1;

	HTREEITEM hItem = m_wndDeviceTreeView.InsertItem( &tvInsert );
	ASSERT( hItem );

	m_treeItemToDeviceMap[ hItem ] = std::pair< CString, CString >( MACAddress, IPAddress );

	// Found a SPU3 device
	CString messageText = _T( "" );
	messageText.Format( _T( "Found SPU3 Device with MAC = %s and IP Address = %s" ),
						MACAddress.GetString(),
						IPAddress.GetString() );

	CMainFrame *pMainWindow = ( CMainFrame* )AfxGetMainWnd();
	ASSERT( pMainWindow );
	pMainWindow->PostMessage( AM_LOG_MESSAGE, 0, ( LPARAM )( new CString( messageText ) ) );

	// Expand the Root Node always
	m_wndDeviceTreeView.Expand( hRoot, TVE_EXPAND );

	return 0;
}

/// <summary>
/// Gets the MAC address.
/// </summary>
/// <param name="hItem">The item.</param>
/// <returns></returns>
CString CDeviceTreeView::GetMACAddress( HTREEITEM hItem )
{
	CString macAddress = _T( "" );
	if( m_treeItemToDeviceMap.find( hItem ) != m_treeItemToDeviceMap.end() )
	{
		macAddress = m_treeItemToDeviceMap[ hItem ].first;
	}

	return macAddress;
}

/// <summary>
/// Gets the IP address.
/// </summary>
/// <param name="hItem">The item.</param>
/// <returns></returns>
CString CDeviceTreeView::GetIPAddress( HTREEITEM hItem )
{
	CString ipAddress = _T( "" );
	if( m_treeItemToDeviceMap.find( hItem ) != m_treeItemToDeviceMap.end() )
	{
		ipAddress = m_treeItemToDeviceMap[ hItem ].second;
	}

	return ipAddress;
}

/// <summary>
/// Gets the item data.
/// </summary>
/// <param name="hItem">The item.</param>
/// <returns></returns>
DWORD CDeviceTreeView::GetItemData( HTREEITEM hItem )
{
	return m_wndDeviceTreeView.GetItemData( hItem );
}

/// <summary>
/// Gets the selected tree item.
/// </summary>
/// <returns></returns>
HTREEITEM CDeviceTreeView::GetSelectedTreeItem()
{
	return m_wndDeviceTreeView.GetSelectedItem();
}

/// <summary>
/// Selects the first device.
/// </summary>
void CDeviceTreeView::SelectFirstDevice()
{
	if( m_wndDeviceTreeView.GetCount() >= 2 )
	{
		HTREEITEM hRootItem = m_wndDeviceTreeView.GetRootItem();
		ASSERT( hRootItem );
		if( m_wndDeviceTreeView.ItemHasChildren( hRootItem ) )
		{
			m_wndDeviceTreeView.SelectItem( m_wndDeviceTreeView.GetChildItem( hRootItem ) );
		}
	}
}

/// <summary>
/// Gets the tree item to device map.
/// </summary>
/// <param name="treeItemToDeviceMap">The tree item to device map.</param>
void CDeviceTreeView::GetTreeItemToDeviceMap( std::map< HTREEITEM, std::pair< CString, CString > >& treeItemToDeviceMap )
{
	treeItemToDeviceMap = m_treeItemToDeviceMap;
}

/// <summary>
/// Called when [update tree item image].
/// </summary>
/// <param name="wParam">The wParameter.</param>
/// <param name="lParam">The lParameter.</param>
/// <returns></returns>
LRESULT CDeviceTreeView::OnUpdateTreeItemImage( WPARAM wParam, LPARAM lParam )
{
	int imageIndex = static_cast< int >( wParam );

	HTREEITEM hTreeIem = ( HTREEITEM )lParam;
	ASSERT( hTreeIem );

	if( m_treeItemToDeviceMap.find( hTreeIem ) != m_treeItemToDeviceMap.end() )
	{
		m_wndDeviceTreeView.SetItemImage( hTreeIem, imageIndex, imageIndex );
	}

	return 0;
}

/// <summary>
/// Called when [update tree item data].
/// </summary>
/// <param name="wParam">The wParameter.</param>
/// <param name="lParam">The lParameter.</param>
/// <returns></returns>
LRESULT CDeviceTreeView::OnUpdateTreeItemData( WPARAM wParam, LPARAM lParam )
{
	DWORD itemData = static_cast< DWORD >( wParam );

	HTREEITEM hTreeIem = ( HTREEITEM )lParam;
	ASSERT( hTreeIem );

	if( m_treeItemToDeviceMap.find( hTreeIem ) != m_treeItemToDeviceMap.end() )
	{
		m_wndDeviceTreeView.SetItemData( hTreeIem, itemData );
	}

	return 0;
}
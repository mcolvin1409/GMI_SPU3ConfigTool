// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MainFrame.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

// MainFrm.cpp : implementation of the CMainFrame class
//

#include <SDKDDKVer.h>
#include "stdafx.h"
#include "SPU3AutoIPConfigTool.h"
#include "MainFrm.h"
#include "MCXMLArchive.h"
#include "AppException.h"
#include "NetBurnSocket.h"
#include "SPU3AutoIPConfigToolDoc.h"
#include "SPU3AutoIPConfigToolView.h"
#include "MCPress.h"
#include "MCSystem.h"
#include "MCSPU.h"
#include "resource.h"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE( CMainFrame, CFrameWndEx )

BEGIN_MESSAGE_MAP( CMainFrame, CFrameWndEx )
	ON_WM_CREATE()
	ON_REGISTERED_MESSAGE( AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew )
	ON_COMMAND( ID_VIEW_CAPTION_BAR, &CMainFrame::OnViewCaptionBar )
	ON_UPDATE_COMMAND_UI( ID_VIEW_CAPTION_BAR, &CMainFrame::OnUpdateViewCaptionBar )
	ON_WM_SETTINGCHANGE()
	ON_MESSAGE( AM_LOAD_SYSTEM_XML_FILE, &CMainFrame::OnLoadSystemXMLFile )
	ON_MESSAGE( AM_HANDLE_MESSAGE_RECEIVED, &CMainFrame::OnMessageReceived )
	ON_MESSAGE( AM_QUERY_SPU3_DEVICES, &CMainFrame::OnQuerySPU3Devices )
	ON_MESSAGE( AM_DEVICE_SEL_CHANGE_TREE, &CMainFrame::OnDeviceSelectionChange )
	ON_MESSAGE( AM_APPLY_DEVICE_SETTINGS, &CMainFrame::OnApplyDeviceConfigSettings )
	ON_MESSAGE( AM_APPLY_APP_SETTINGS, &CMainFrame::OnApplyAppConfigSettings )
	ON_MESSAGE( AM_START_TIMER_MSG, &CMainFrame::OnStartTimerMessage )
	ON_MESSAGE( AM_SELECT_FIRST_DEVICE, &CMainFrame::OnSelectFirstDevice )
	ON_MESSAGE( AM_BLINK_SPU3_LEDS, &CMainFrame::OnBlinkSPU3LEDs )
	ON_MESSAGE( AM_REBOOT_SPU3_DEVICE, &CMainFrame::OnRebootSPU3Device )
	ON_MESSAGE( AM_DISPLAY_ALL_DEVICES_VIEW, &CMainFrame::OnDisplayAllDevicesView )
	ON_MESSAGE( AM_DISPLAY_NO_DEVICES_VIEW, &CMainFrame::OnDisplayNoDevicesView )
	ON_MESSAGE( AM_DISPLAY_CAPTION_BAR_MSG, &CMainFrame::OnDisplayCaptionBarMessage )
	ON_MESSAGE( AM_CLEAR_CAPTION_BAR_MSG, &CMainFrame::OnClearCaptionBarMessage )
	ON_MESSAGE( AM_LOAD_HISTORY_FILE, &CMainFrame::OnLoadHistoryFile )
	ON_MESSAGE( AM_SCAN_DEVICE_TREE, &CMainFrame::OnScanDeviceTree )
	ON_MESSAGE( AM_LOG_MESSAGE, &CMainFrame::OnLogMessage )
	ON_MESSAGE( AM_UPDATE_STATUSBAR_TEXT, &CMainFrame::OnUpdateStatusbarText )
	ON_MESSAGE( AM_CREATE_HISTORY_FILE, &CMainFrame::OnCreateHistoryFile )
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_STATUS_BAR_MSG_PANE	// Display message according to the Tree item selection
};

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
/// <summary>
/// The idd
/// </summary>
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support

														  // Implementation
protected:
	DECLARE_MESSAGE_MAP()
private:
	CString m_appInfo;
};

/// <summary>
/// Initializes a new instance of the <see cref="CAboutDlg" /> class.
/// </summary>
/// <param name="pDX">The p dx.</param>
CAboutDlg::CAboutDlg() : CDialogEx( IDD_ABOUTBOX )
{ 
	CString appName = _T( "" );
	appName.LoadString( AFX_IDS_APP_TITLE );

	CString versionNum = _T( "" );
	versionNum.LoadString( IDS_APP_VERSION_NUM );

	m_appInfo = appName + _T( ", Version " ) + versionNum;
}

/// <summary>
/// Does the data exchange.
/// </summary>
/// <param name="pDX">The data exchange.</param>
void CAboutDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_STATIC_VERSION, m_appInfo );
}

BEGIN_MESSAGE_MAP( CAboutDlg, CDialogEx )
END_MESSAGE_MAP()

// App command to run the dialog
/// <summary>
/// Called when [application about].
/// </summary>
void CSPU3AutoIPConfigToolApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CMainFrame construction/destruction

/// <summary>
/// Initializes a new instance of the <see cref="CMainFrame"/> class.
/// </summary>
CMainFrame::CMainFrame()
{
	m_pSystem = std::unique_ptr< CMCSystem >( new CMCSystem() );
	ASSERT( m_pSystem );
	m_treeItemMap.clear();
	m_messageHandlerMap.clear();
	m_bSystemXMLFileLoaded = false;
	m_bHistoryFileLoaded = false;
	m_messageToTimerIdMap.clear();
}

/// <summary>
/// Finalizes an instance of the <see cref="CMainFrame"/> class.
/// </summary>
CMainFrame::~CMainFrame()
{ }

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support to discover the Netburner devices on 
///				a System with multiple Network Interfaces (NICs)
///
/// ]]>
/// <summary>
/// Called when [create].
/// </summary>
/// <param name="lpCreateStruct">The lp create structure.</param>
/// <returns></returns>
int CMainFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( CFrameWndEx::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus( FALSE );

	if( !m_wndStatusBar.Create( this ) )
	{
		TRACE0( "Failed to create status bar\n" );
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators( indicators, sizeof( indicators ) / sizeof( UINT ) );

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode( DT_SMART );

	// Create a caption bar:
	if( !CreateCaptionBar() )
	{
		TRACE0( "Failed to create caption bar\n" );
		return -1;      // fail to create
	}

	// create docking windows
	if( !CreateDockingWindows() )
	{
		TRACE0( "Failed to create docking windows\n" );
		return -1;
	}

	// create dockable browse pane
	if( !CreateBrowsePane() )
	{
		TRACE0( "Failed to create browse pane\n" );
		return -1;
	}

	DockPane( &m_wndTreeView );
	AddPane( &m_wndOutput );
	AddPane( &m_dialogBarPane );

	// set the visual manager used to draw all user interface elements
	CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerVS2008 ) );

	// Get all of the IPAddress/NIC on the System
	GetIPAddressesOfAllNetworkInterfaces();
	// Create the log file
	CreateLogFile();
	// Load Message Handlers
	LoadMessageHandlers();
	// Create Netburner socket (UDP)
	CreateDeviceSocket();
	// Create Application socket (UDP)
	CreateApplicationSocket();
	// Write Application Version and Build date info to the log file
	WriteApplicationInfo();
	CreateDeviceStatusMap();

	// Let's discover the SPU3 devices on the Network => Broadcast a Device Config message
	PostMessage( AM_QUERY_SPU3_DEVICES, 0, 0 );
	// Load the History file data
	PostMessage( AM_LOAD_HISTORY_FILE, 0, 0 );

	AddAboutBoxMenuItemToSystemMenu();

	return 0;
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support to discover the Netburner devices on 
///				a System with multiple Network Interfaces (NICs)
///
/// ]]>
/// <summary>
/// Gets the ip addresses of all network interfaces.
/// </summary>
void CMainFrame::GetIPAddressesOfAllNetworkInterfaces()
{ 
	m_sockAddressInList.clear();

	ULONG bytesReturned = 0;
	INTERFACE_INFO ifacelist[ 20 ]={};

	SOCKET sd = WSASocket( AF_INET, SOCK_DGRAM, 0, 0, 0, 0 );
	WSAIoctl( sd, SIO_GET_INTERFACE_LIST, 0, 0, &ifacelist, sizeof( ifacelist ), &bytesReturned, 0, 0 );

	int numInterfaces = bytesReturned / sizeof( INTERFACE_INFO );
	for( int interfaceIndex = 0; interfaceIndex < numInterfaces; ++interfaceIndex )
	{
		sockaddr_in *pAddress = nullptr;
		pAddress = ( sockaddr_in * ) & ( ifacelist[ interfaceIndex ].iiAddress );
		if( nullptr != pAddress )
		{
			m_sockAddressInList.push_back( *pAddress );
		}		
	}
}


/// <summary>
/// Adds the about box menu item to system menu.
/// </summary>
void CMainFrame::AddAboutBoxMenuItemToSystemMenu()
{
	CMenu* pSysMenu = GetSystemMenu( FALSE );
	if( NULL != pSysMenu )
	{
		pSysMenu->AppendMenu( MF_STRING, ID_APP_ABOUT, _T( "&About SPU3 Ethernet Auto Configuration Tool" ) );
	}
}

/// <summary>
/// Creates the log file.
/// </summary>
void CMainFrame::CreateLogFile()
{
	// Create History File 
	CString logFileFullPath = theApp.m_logFolderPath + _T( "\\" ) + SPU3EthernetIPConfigLogFileName;
	m_logFile.SetFileName( logFileFullPath );
}

/// <summary>
/// Writes the application information.
/// </summary>
void CMainFrame::WriteApplicationInfo()
{
	CString messageText = _T( "" );
	messageText.Format( _T( "SPU3 Ethernet Auto Configuration Tool Startup, Command line = %s" ), theApp.m_lpCmdLine );
	LogMessage( messageText );
	// Write Application Version and Build date
	messageText.Format( _T( "Version = %s  Build Date = %s \r\n" ), theApp.m_appVersion.GetString(), theApp.m_appBuildDate.GetString() );
	LogMessage( messageText );
}

/// <summary>
/// Pres the create window.
/// </summary>
/// <param name="cs">The cs.</param>
/// <returns></returns>
BOOL CMainFrame::PreCreateWindow( CREATESTRUCT& cs )
{
	if( !CFrameWndEx::PreCreateWindow( cs ) )
		return FALSE;

	cs.style &= ~FWS_ADDTOTITLE;	// No "Untitled" title
	cs.lpszClass = AppMainWindowClassName;	// Application Window Name

	// Let's remove main menu
	if( NULL != cs.hMenu )
	{
		::DestroyMenu( cs.hMenu );      // delete menu if loaded
		cs.hMenu = NULL;              // no menu for this window
	}

	cs.cx = 750;	// Pixels
	cs.cy = 800;	// Pixels
	cs.y = ( ( cs.cy * 2 ) - cs.cy ) / 2;
	cs.x = ( ( cs.cx * 2 ) - cs.cx ) / 2;

	return TRUE;
}

/// <summary>
/// Creates the docking windows.
/// </summary>
/// <returns></returns>
BOOL CMainFrame::CreateDockingWindows()
{
	// Create class view
	CString strClassView = _T( "" );
	BOOL bNameValid = strClassView.LoadString( IDS_CLASS_VIEW );
	ASSERT( bNameValid );
	if( !m_wndTreeView.Create( strClassView, this, CRect( 0, 0, 200, 200 ), TRUE, ID_VIEW_CLASSVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI ) )
	{
		TRACE0( "Failed to create Class View window\n" );
		return FALSE; // failed to create
	}

	DWORD dwClassWndStyle = m_wndTreeView.GetControlBarStyle();
	dwClassWndStyle &= ~( AFX_CBRS_CLOSE | AFX_CBRS_FLOAT | AFX_CBRS_AUTOHIDE );
	m_wndTreeView.SetControlBarStyle( dwClassWndStyle );

	// Create output window
	CString strOutputWnd = _T( "" );
	bNameValid = strOutputWnd.LoadString( IDS_OUTPUT_WND );
	ASSERT( bNameValid );
	if( !m_wndOutput.Create( strOutputWnd, this, CRect( 0, 0, 150, 150 ), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI ) )
	{
		TRACE0( "Failed to create Output window\n" );
		return FALSE; // failed to create
	}

	DWORD dwOutputWndStyle = m_wndOutput.GetControlBarStyle();
	dwOutputWndStyle &= ~( AFX_CBRS_CLOSE | AFX_CBRS_FLOAT | AFX_CBRS_AUTOHIDE );
	m_wndOutput.SetControlBarStyle( dwOutputWndStyle );

	SetDockingWindowIcons( theApp.m_bHiColorIcons );

	return TRUE;
}

/// <summary>
/// Sets the docking window icons.
/// </summary>
/// <param name="bHiColorIcons">The b hi color icons.</param>
void CMainFrame::SetDockingWindowIcons( BOOL bHiColorIcons )
{
	HICON hClassViewIcon = ( HICON ) ::LoadImage( ::AfxGetResourceHandle(), MAKEINTRESOURCE( bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW ), IMAGE_ICON, ::GetSystemMetrics( SM_CXSMICON ), ::GetSystemMetrics( SM_CYSMICON ), 0 );
	m_wndTreeView.SetIcon( hClassViewIcon, FALSE );

	HICON hOutputBarIcon = ( HICON ) ::LoadImage( ::AfxGetResourceHandle(), MAKEINTRESOURCE( bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND ), IMAGE_ICON, ::GetSystemMetrics( SM_CXSMICON ), ::GetSystemMetrics( SM_CYSMICON ), 0 );
	m_wndOutput.SetIcon( hOutputBarIcon, FALSE );
}

/// <summary>
/// Creates the caption bar.
/// </summary>
/// <returns></returns>
BOOL CMainFrame::CreateCaptionBar()
{
	if( !m_wndCaptionBar.Create( WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this, ID_VIEW_CAPTION_BAR, -1, TRUE ) )
	{
		TRACE0( "Failed to create caption bar\n" );
		return FALSE;
	}

	return TRUE;
}

/// <summary>
/// Creates the browse pane.
/// </summary>
/// <returns></returns>
BOOL CMainFrame::CreateBrowsePane()
{
	UINT style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_TOP;

	CString strTitle = _T( "Mercury" );
	if( !m_dialogBarPane.Create( strTitle, this, CRect( 0, 0, 200, 100 ), TRUE, IDD_DIALOGBAR, style ) )
	{
		TRACE0( "Failed to create dialog bar pane\n" );
		return FALSE;
	}

	DWORD dwWndStyle = m_dialogBarPane.GetControlBarStyle();
	dwWndStyle &= ~( AFX_CBRS_CLOSE | AFX_CBRS_FLOAT | AFX_CBRS_AUTOHIDE );
	m_dialogBarPane.SetControlBarStyle( dwWndStyle );
	m_dialogBarPane.ShowPane( TRUE, FALSE, TRUE );

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
/// <summary>
/// Asserts the valid.
/// </summary>
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

/// <summary>
/// Dumps the specified dc.
/// </summary>
/// <param name="dc">The dc.</param>
void CMainFrame::Dump( CDumpContext& dc ) const
{
	CFrameWndEx::Dump( dc );
}
#endif //_DEBUG


// CMainFrame message handlers

/// <summary>
/// Called when [toolbar create new].
/// </summary>
/// <param name="wp">The wp.</param>
/// <param name="lp">The lp.</param>
/// <returns></returns>
LRESULT CMainFrame::OnToolbarCreateNew( WPARAM wp, LPARAM lp )
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew( wp, lp );
	if( lres == 0 )
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = ( CMFCToolBar* )lres;
	ASSERT_VALID( pUserToolbar );

	return lres;
}

/// <summary>
/// Called when [view caption bar].
/// </summary>
void CMainFrame::OnViewCaptionBar()
{
	m_wndCaptionBar.ShowWindow( m_wndCaptionBar.IsVisible() ? SW_HIDE : SW_SHOW );
	RecalcLayout( FALSE );
}

/// <summary>
/// Called when [update view caption bar].
/// </summary>
/// <param name="pCmdUI">The p command UI.</param>
void CMainFrame::OnUpdateViewCaptionBar( CCmdUI* pCmdUI )
{
	pCmdUI->SetCheck( m_wndCaptionBar.IsVisible() );
}

/// <summary>
/// Loads the frame.
/// </summary>
/// <param name="nIDResource">The identifier resource.</param>
/// <param name="dwDefaultStyle">The dw default style.</param>
/// <param name="pParentWnd">The parent WND.</param>
/// <param name="pContext">The context.</param>
/// <returns></returns>
BOOL CMainFrame::LoadFrame( UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext )
{
	// base class does the real work
	if( !CFrameWndEx::LoadFrame( nIDResource, dwDefaultStyle, pParentWnd, pContext ) )
	{
		return FALSE;
	}

	return TRUE;
}

/// <summary>
/// Called when [setting change].
/// </summary>
/// <param name="uFlags">The flags.</param>
/// <param name="lpszSection">The LPSZ section.</param>
void CMainFrame::OnSettingChange( UINT uFlags, LPCTSTR lpszSection )
{
	CFrameWndEx::OnSettingChange( uFlags, lpszSection );
	m_wndOutput.UpdateFonts();
}

/// <summary>
/// Loads the message handlers.
/// </summary>
void CMainFrame::LoadMessageHandlers()
{
	try
	{
		m_messageHandlerMap[ NBAUTO_DEV_READ ] = std::bind( &CMainFrame::HandleDeviceConfigReadMessage, this, _1, _2 );
		m_messageHandlerMap[ NBAUTO_APP_CONFIG ] = std::bind( &CMainFrame::HandleApplicationConfigMessage, this, _1, _2 );
		m_messageHandlerMap[ NBAUTO_DEV_WRITE ] = std::bind( &CMainFrame::HandleBroadCastMessage, this, _1, _2 );
		m_messageHandlerMap[ NBAUTO_APP_WRITE ] = std::bind( &CMainFrame::HandleBroadCastMessage, this, _1, _2 );
		m_messageHandlerMap[ NBAUTO_BLINK_LEDS ] = std::bind( &CMainFrame::HandleBroadCastMessage, this, _1, _2 );
		m_messageHandlerMap[ NBAUTO_REBOOT ] = std::bind( &CMainFrame::HandleBroadCastMessage, this, _1, _2 );
	}
	catch( CException* ex )
	{
		ex->ReportError();
		ex->Delete();
	}
}

/// <summary>
/// Gets the system.
/// </summary>
/// <returns></returns>
CMCSystem* CMainFrame::GetSystem()
{
	return m_pSystem.get();
}

/// <summary>
/// Displays the notification.
/// </summary>
/// <param name="message">The message.</param>
void CMainFrame::DisplayNotification( const CString& message )
{
	m_wndCaptionBar.SetText( message, CMFCCaptionBar::ALIGN_LEFT );
	m_wndCaptionBar.Invalidate();
}


/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support to process messages to/from Netburner Devices on 
///				a System with multiple Network Interfaces (NICs)
///
/// ]]>
/// <summary>
/// Creates the device socket.
/// </summary>
void CMainFrame::CreateDeviceSocket()
{
	try
	{
		// For each NIC create a Netburner Device Socket (UDP)
		for( const sockaddr_in& sockAddressIn : m_sockAddressInList )
		{
			std::unique_ptr< NetBurnSocket > pDeviceSocket = std::make_unique< NetBurnSocket >( this );
			ASSERT( pDeviceSocket );
			pDeviceSocket->CreateDeviceSocket( sockAddressIn.sin_addr );
			// Add the Device Socket to the Device Socket List
			m_deviceSocketList.emplace_back( std::move( pDeviceSocket ) );
		}
	}
	catch( CException* ex )
	{
		ex->ReportError();
		ex->Delete();
	}
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support to process messages to/from Netburner Devices on 
///				a System with multiple Network Interfaces (NICs)
///
/// ]]>
/// <summary>
/// Creates the application socket.
/// </summary>
void CMainFrame::CreateApplicationSocket()
{
	try
	{
		// For each NIC create an Application Socket (UDP) for updating/writing the Application specific data
		for( const sockaddr_in& sockAddressIn : m_sockAddressInList )
		{
			std::unique_ptr< NetBurnSocket > pApplicationSocket = std::make_unique< NetBurnSocket >( this );
			ASSERT( pApplicationSocket );
			pApplicationSocket->CreateApplicationSocket( sockAddressIn.sin_addr );
			// Add the Application Socket to the Application Socket List
			m_applicationSocketList.emplace_back( std::move( pApplicationSocket ) );
		}
	}
	catch( CException* ex )
	{
		ex->ReportError();
		ex->Delete();
	}
}

/// <summary>
/// Called when [load system XML file].
/// </summary>
/// <param name="wParam">The wParameter.</param>
/// <param name="lParam">The lParameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnLoadSystemXMLFile( WPARAM wParam, LPARAM lParam )
{
	std::unique_ptr< CString > pXmlFileName( ( CString* )lParam );

	try
	{
		CMCXMLArchive xmlArchive( m_pSystem.get() );
		if( !xmlArchive.LoadSystemConfiguration( pXmlFileName->GetBuffer( pXmlFileName->GetLength() ) ) )
		{
			m_bSystemXMLFileLoaded = false;
			CString messageText = _T( "" );
			messageText.Format( _T( "Failed to load the System XML File = %s." ), *pXmlFileName );
			AfxMessageBox( messageText, MB_ICONERROR );
			LogMessage( messageText );
			return 0;
		}

		// System XML file loaded successfully
		m_bSystemXMLFileLoaded = true;

		// Generate the SPU to Inkers list
		CMCPress *pPress = m_pSystem->GetPress();
		if( NULL != pPress )
		{
			pPress->GenerateSPUToInkersList();
		}

		// Notify status => System XML file was loaded successfully
		CString strMessage = _T( "" );
		strMessage.Format( _T( "System XML File %s Loaded Successfully." ), *pXmlFileName );
		PostMessage( AM_DISPLAY_CAPTION_BAR_MSG, 0, ( LPARAM )( new CString( strMessage ) ) );
		LogMessage( strMessage );

		CView* pView = GetActiveView();
		if( NULL != pView )
		{
			pView->PostMessage( AM_UPDATE_SYS_SPU3_IPADDR, 0, 0 );
		}

		// Scan the SPU3 device tree and update the Tree item images accordingly
		PostMessage( AM_QUERY_SPU3_DEVICES, 0, 0 );
	}
	catch( CAppException& ex )
	{
		AfxMessageBox( ex.GetMessage().c_str(), MB_ICONERROR );
	}

	return 0;
}

/// <summary>
/// Called when [load history file].
/// </summary>
/// <param name="wParam">The wParameter.</param>
/// <param name="lParam">The lParameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnLoadHistoryFile( WPARAM wParam, LPARAM lParam )
{
	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	// History file name
	CString historyFileFullPath = theApp.m_logFolderPath + _T( "\\" ) + SPU3EthernetIPConfigHistoryFileName;
	// Is history file exists? If NO, just return from this method.
	if( !PathFileExists( historyFileFullPath ) )
	{
		CString messageText = _T( "" );
		messageText.Format( _T( "History File does NOT exists at %s." ), historyFileFullPath );
		LogMessage( messageText );

		return 0;
	}

	try
	{
		std::wifstream reportStream( historyFileFullPath );
		// failed to open the history file
		if( !reportStream.is_open() )
		{
			CString messageText = _T( "" );
			messageText.Format( _T( "Failed to open the History File %s." ), historyFileFullPath );
			AfxMessageBox( messageText, MB_ICONERROR );
			m_bHistoryFileLoaded = false;
			LogMessage( messageText );
			return 0;
		}

		m_spuHistoryFileMap.clear();

		while( !reportStream.eof() )
		{
			tstring lineText = TEXT( "" );
			if( std::getline( reportStream, lineText ) )
			{
				if( !lineText.empty() )
				{
					tstringstream streamLine( lineText );

					tstring macAddressKey = TEXT( "" );
					std::getline( streamLine, macAddressKey, TEXT( ',' ) );

					tstring spuIPAddressValue = TEXT( "" );
					std::getline( streamLine, spuIPAddressValue );

					if( !macAddressKey.empty() && !spuIPAddressValue.empty() )
					{
						m_spuHistoryFileMap[ CString( macAddressKey.c_str() ) ] = CString( spuIPAddressValue.c_str() );
					}
				}
			}
		}

		// close the file
		reportStream.close();

		// History file loaded successfully
		m_bHistoryFileLoaded = ( static_cast< int >( m_spuHistoryFileMap.size() ) > 0 );
	}
	catch( CException* ex )
	{
		ex->ReportError();
		ex->Delete();
	}

	return 0;
}

/// <summary>
/// Determines whether [is system XML file loaded].
/// </summary>
/// <returns>
///   <c>true</c> if [is system XML file loaded]; otherwise, <c>false</c>.
/// </returns>
bool CMainFrame::IsSystemXMLFileLoaded()
{
	return m_bSystemXMLFileLoaded;
}

/// <summary>
/// Gets all devices configuration settings.
/// </summary>
/// <param name="allDevConfigSettings">All dev configuration settings.</param>
void CMainFrame::GetAllDevicesConfigSettings( std::map< CString, TreeItemRecord >& allDevConfigSettings )
{
	allDevConfigSettings = m_treeItemMap;
}

/// <summary>
/// Gets the live spuip address list.
/// </summary>
/// <param name="spuIPAddressList">The spu ip address list.</param>
void CMainFrame::GetLiveSPUIPAddressList( std::vector< CString >& spuIPAddressList )
{
	//For each SPU3 Device 
	for( auto deviceSettingIter : m_treeItemMap )
	{
		CString spuIPAddress = ToIPAddress( deviceSettingIter.second.m_devConfigRecord.ip_Addr );
		if( !spuIPAddress.IsEmpty() )
		{
			spuIPAddressList.push_back( spuIPAddress );
		}
	}
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support to process messages to/from Netburner Devices on 
///				a System with multiple Network Interfaces (NICs)
///
/// ]]>
/// <summary>
/// Called when [query SPU3 devices].
/// </summary>
/// <param name="wParam">The wParameter.</param>
/// <param name="lParam">The lParameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnQuerySPU3Devices( WPARAM wParam, LPARAM lParam )
{
	try
	{
		// Clear the items
		m_treeItemMap.clear();
		m_wndTreeView.Refresh();

		// Notify status => Please wait... Querying the Network for SPU3 Devices...
		CString strMessage = _T( "Querying the Network for SPU3 Devices..." );
		LogMessage( strMessage );
		
		// Let's discover the SPU3 devices on the Network 
		//	Broadcast a Device Config Message and an Application Config Message

		// For each Device Socket 
		for( const auto& pDeviceSocket : m_deviceSocketList )
		{
			ASSERT( pDeviceSocket );
			pDeviceSocket->SendRequest_AutoReadDeviceConfig();
		}
		
		// For each Application Socket
		for( const auto& pApplicationSocket : m_applicationSocketList )
		{
			ASSERT( pApplicationSocket );
			pApplicationSocket->SendRequest_AutoReadApplicationConfig();
		}

		// Start a timer to select the first device from Tree
		PostMessage( AM_START_TIMER_MSG, ( WPARAM )AM_SELECT_FIRST_DEVICE, ( LPARAM )1000 );

		// Scan the SPU3 device tree and update the Tree item images accordingly
		PostMessage( AM_START_TIMER_MSG, ( WPARAM )AM_SCAN_DEVICE_TREE, ( LPARAM )1000 );
	}
	catch( CAppException& ex )
	{
		AfxMessageBox( ex.GetMessage().c_str(), MB_ICONERROR );
	}

	return 0;
}

/// <summary>
/// Called when [device selection change].
/// </summary>
/// <param name="wParam">The wParameter.</param>
/// <param name="lParam">The lParameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnDeviceSelectionChange( WPARAM wParam, LPARAM lParam )
{
	std::unique_ptr< CString > pMACAddress( ( CString * )lParam );
	ASSERT( pMACAddress );

	CString SPU3MACAddress = *pMACAddress;
	if( m_treeItemMap.find( SPU3MACAddress ) != m_treeItemMap.end() )
	{
		CView* pView = GetActiveView();
		if( NULL != pView )
		{
			TreeItemRecord treeItemRecord = m_treeItemMap[ SPU3MACAddress ];
			pView->PostMessage( AM_POPULATE_DEVICE_SETTINGS, 0, ( LPARAM )( new TreeItemRecord( treeItemRecord ) ) );
		}

		// Update Statusbar text 
		PostMessage( AM_UPDATE_STATUSBAR_TEXT, ( WPARAM )AM_POPULATE_DEVICE_SETTINGS, 0 );
	}

	return 0;
}

/// <summary>
/// Called when [display all devices view].
/// </summary>
/// <param name="wParam">The wParameter.</param>
/// <param name="lParam">The lParameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnDisplayAllDevicesView( WPARAM wParam, LPARAM lParam )
{
	CView* pView = GetActiveView();
	if( NULL != pView )
	{
		pView->PostMessage( AM_DISPLAY_ALL_DEVICES_VIEW, 0, 0 );
	}

	// Update Statusbar text 
	PostMessage( AM_UPDATE_STATUSBAR_TEXT, ( WPARAM )AM_DISPLAY_ALL_DEVICES_VIEW, 0 );

	return 0;
}

/// <summary>
/// Called when [display no devices view].
/// </summary>
/// <param name="wParam">The wParameter.</param>
/// <param name="lParam">The lParameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnDisplayNoDevicesView( WPARAM wParam, LPARAM lParam )
{
	CView* pView = GetActiveView();
	if( NULL != pView )
	{
		pView->PostMessage( AM_DISPLAY_NO_DEVICES_VIEW, 0, 0 );
	}

	// Update Statusbar text 
	PostMessage( AM_UPDATE_STATUSBAR_TEXT, ( WPARAM )AM_DISPLAY_NO_DEVICES_VIEW, 0 );

	return 0;
}

/// <summary>
/// Called when [message received].
/// </summary>
/// <param name="wParam">The wParameter.</param>
/// <param name="lParam">The lParameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnMessageReceived( WPARAM wParam, LPARAM lParam )
{
	int messageLength = static_cast< int >( wParam );
	ASSERT( messageLength );

	std::unique_ptr< BYTE > pBuffer( ( BYTE * )lParam );
	ASSERT( pBuffer );

	MsgHeader *pMsgHeader = ( MsgHeader * )( pBuffer.get() );
	ASSERT( pMsgHeader );

	try
	{
		if( m_messageHandlerMap.find( pMsgHeader->m_bAction ) != m_messageHandlerMap.end() )
		{
			try
			{
				// process the message received
				m_messageHandlerMap[ pMsgHeader->m_bAction ]( messageLength, pBuffer.get() );
			}
			catch( CAppException &exception )
			{
				CString messageText = _T( "" );
				messageText.Format( _T( "Invalid message format: message ID = %d, Reason = '%s'" ), pMsgHeader->m_bAction, exception.GetMessage() );
				LogMessage( messageText );
			}
		}
		else
		{
			// Notify of an unused message
			CString messageText = _T( "" );
			messageText.Format( _T( "Unrecognized message ID = %d " ), pMsgHeader->m_bAction );
			LogMessage( messageText );
		}
	}
	catch( CAppException &exception )
	{
		tstring messageText = exception.GetMessage();
		LogMessage( messageText.c_str() );
	}
	catch( CException* ex )
	{
		ex->ReportError();
		ex->Delete();
	}

	return 0;
}

/// <summary>
/// Handles the device configuration read message.
/// </summary>
/// <param name="messageLength">Length of the message.</param>
/// <param name="messageData">The message data.</param>
void CMainFrame::HandleDeviceConfigReadMessage( int messageLength, LPBYTE messageData )
{
	// Note:
	//	Here, the response message length will be more than the size of the DeviceConfigStruct. 
	//	The response message will include the Device Name and Application Name additionally.	
	if( messageLength < sizeof( DeviceConfigStruct ) )
	{
		return;
	}

	char Device_Name[ 80 ] = { '\0' };
	char Device_AppName[ 200 ] = { '\0' };

	int clen = messageLength - sizeof( DeviceConfigStruct );
	if( clen < 0 )
	{
		clen = 0;
	}

	if( clen > 79 )
	{
		clen = 79;
	}

	// Get the Device Name
	strncpy_s( Device_Name, ( char * )( messageData + sizeof( DeviceConfigStruct ) ), clen );
	unsigned char * cp = messageData + sizeof( DeviceConfigStruct );

	while( *cp )
	{
		cp++;
	}

	// Get the Device Application Name
	if( ( messageData + messageLength ) > cp + 2 )
	{
		cp++;
		strncpy_s( Device_AppName, ( char * )cp, 199 );
	}
	else
	{
		Device_AppName[ 0 ] = '\0';
	}

	CString deviceName = CString( Device_Name ).Trim();
	CString applicationName = CString( Device_AppName ).Trim();

	if( !deviceName.IsEmpty() && !applicationName.IsEmpty() )
	{
		// Is it a SPU3 Device?
		if( -1 != applicationName.Find( _T( "SPU3" ), 0 ) )
		{
			DeviceConfigStruct *pDeviceConfig = ( DeviceConfigStruct * )messageData;
			ASSERT( pDeviceConfig );

			CString deviceMACAddress = _T( "" );
			deviceMACAddress.Format( _T( "%02X-%02X-%02X-%02X-%02X-%02X" ),
									 pDeviceConfig->m_devConfigRecord.mac_address[ 0 ],
									 pDeviceConfig->m_devConfigRecord.mac_address[ 1 ],
									 pDeviceConfig->m_devConfigRecord.mac_address[ 2 ],
									 pDeviceConfig->m_devConfigRecord.mac_address[ 3 ],
									 pDeviceConfig->m_devConfigRecord.mac_address[ 4 ],
									 pDeviceConfig->m_devConfigRecord.mac_address[ 5 ] );

			// Let's add the Device Configuration to the Tree Items Map
			m_treeItemMap[ deviceMACAddress ].m_devConfigRecord = pDeviceConfig->m_devConfigRecord;

			// Add device configuration to the Device Tree View
			AddDevConfigToDeviceTreeView( pDeviceConfig->m_devConfigRecord );
		}
	}
}

/// <summary>
/// Handles the application configuration message.
/// </summary>
/// <param name="messageLength">Length of the message.</param>
/// <param name="messageData">The message data.</param>
void CMainFrame::HandleApplicationConfigMessage( int messageLength, LPBYTE messageData )
{
	if( messageLength != sizeof( AppConfigStruct ) )
	{
		return;
	}

	AppConfigStruct *pAppConfig = ( AppConfigStruct * )messageData;
	ASSERT( pAppConfig );

	CString deviceMACAddress = _T( "" );
	deviceMACAddress.Format( _T( "%02X-%02X-%02X-%02X-%02X-%02X" ),
							 pAppConfig->m_appConfigRecord.spuMACAddress[ 0 ],
							 pAppConfig->m_appConfigRecord.spuMACAddress[ 1 ],
							 pAppConfig->m_appConfigRecord.spuMACAddress[ 2 ],
							 pAppConfig->m_appConfigRecord.spuMACAddress[ 3 ],
							 pAppConfig->m_appConfigRecord.spuMACAddress[ 4 ],
							 pAppConfig->m_appConfigRecord.spuMACAddress[ 5 ] );

	// Let's add the Device Application Configuration to the Tree Items Map
	if( m_treeItemMap.find( deviceMACAddress ) != m_treeItemMap.end() )
	{
		m_treeItemMap[ deviceMACAddress ].m_appConfigRecord = pAppConfig->m_appConfigRecord;
	}
}

/// <summary>
/// Handles the broad cast message.
/// </summary>
/// <param name="messageLength">Length of the message.</param>
/// <param name="messageData">The message data.</param>
void CMainFrame::HandleBroadCastMessage( int messageLength, LPBYTE messageData )
{
	// Ignore self broadcast messages
	// Do nothing for now!
}

/// <summary>
/// Adds the dev configuration to device TreeView.
/// </summary>
/// <param name="deviceConfigRecord">The device configuration record.</param>
void CMainFrame::AddDevConfigToDeviceTreeView( DeviceConfigRecord& deviceConfigRecord )
{
	// Add the Device Config Record to the Device Tree View
	m_wndTreeView.PostMessage( AM_ADD_DEVICE_TO_TREE, ( WPARAM )0, ( LPARAM )( new DeviceConfigRecord( deviceConfigRecord ) ) );
}

/// <summary>
/// Gets the mac address and ip address of tree item.
/// </summary>
/// <param name="hItem">The h item.</param>
/// <param name="MACAddress">The mac address.</param>
/// <param name="IPAddress">The ip address.</param>
void CMainFrame::GetMACAddressAndIPAddressOfTreeItem( HTREEITEM hItem, CString& MACAddress, CString& IPAddress )
{
	ASSERT( hItem );

	MACAddress = m_wndTreeView.GetMACAddress( hItem );
	IPAddress = m_wndTreeView.GetIPAddress( hItem );
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="logMessage">The log message.</param>
void CMainFrame::LogMessage( const CString& logMessage )
{
	m_logFile.LogMessage( logMessage );
	// Display the log message text to the Log window
	m_wndOutput.PostMessage( AM_DISPLAY_LOG_WINDOW_MSG, ( WPARAM )0, ( LPARAM )( new CString( logMessage ) ) );
}

/// <summary>
/// Gets the device configuration record of selected item.
/// </summary>
/// <param name="deviceConfigRecord">The device configuration record.</param>
void CMainFrame::GetDeviceConfigRecordOfSelectedItem( DeviceConfigRecord& deviceConfigRecord )
{
	// Get the MAC address of the Tree item selected 
	CString deviceMACAddress = m_wndTreeView.GetMACAddress( m_wndTreeView.GetSelectedTreeItem() );
	// Get the DeviceRecord 
	if( !deviceMACAddress.IsEmpty() )
	{
		if( m_treeItemMap.find( deviceMACAddress ) != m_treeItemMap.end() )
		{
			deviceConfigRecord = m_treeItemMap[ deviceMACAddress ].m_devConfigRecord;
		}
	}
}

/// <summary>
/// Gets the application configuration record of selected item.
/// </summary>
/// <param name="appConfigRecord">The application configuration record.</param>
void CMainFrame::GetAppConfigRecordOfSelectedItem( AppConfigRecord& appConfigRecord )
{
	// Get the MAC address of the Tree item selected 
	CString deviceMACAddress = m_wndTreeView.GetMACAddress( m_wndTreeView.GetSelectedTreeItem() );
	// Get the AppConfigRecord
	if( !deviceMACAddress.IsEmpty() )
	{
		appConfigRecord = m_treeItemMap[ deviceMACAddress ].m_appConfigRecord;
	}
}

/// <summary>
/// Gets the device runtime status of selected item.
/// </summary>
/// <param name="deviceRuntimeStatus">The device runtime status.</param>
void CMainFrame::GetDeviceRuntimeStatusOfSelectedItem( DeviceRuntimeStatus& deviceRuntimeStatus )
{
	// Get the MAC address of the Tree item selected 
	CString deviceMACAddress = m_wndTreeView.GetMACAddress( m_wndTreeView.GetSelectedTreeItem() );
	// Get the Device Runtime Status
	if( !deviceMACAddress.IsEmpty() )
	{
		deviceRuntimeStatus = m_treeItemMap[ deviceMACAddress ].m_devRuntimeStatus;
	}
}

/// <summary>
/// Sets the device runtime status of selected item.
/// </summary>
/// <param name="deviceRuntimeStatus">The device runtime status.</param>
void CMainFrame::SetDeviceRuntimeStatusOfSelectedItem( DeviceRuntimeStatus& deviceRuntimeStatus )
{
	// Get the MAC address of the Tree item selected 
	CString deviceMACAddress = m_wndTreeView.GetMACAddress( m_wndTreeView.GetSelectedTreeItem() );
	// Get the Device Runtime Status
	if( !deviceMACAddress.IsEmpty() )
	{
		m_treeItemMap[ deviceMACAddress ].m_devRuntimeStatus = deviceRuntimeStatus;
	}
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support to process messages to/from Netburner Devices on 
///				a System with multiple Network Interfaces (NICs)
///
/// ]]>
/// <summary>
/// Called when [apply device configuration settings].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnApplyDeviceConfigSettings( WPARAM wParam, LPARAM lParam )
{
	try
	{		
		// This smart pointer will take care of releasing the heap memory
		std::unique_ptr< DeviceConfigRecord > pDeviceConfigRecord( ( DeviceConfigRecord * )lParam );
		ASSERT( pDeviceConfigRecord );

		for( const auto& pDeviceSocket : m_deviceSocketList )
		{
			ASSERT( pDeviceSocket );
			// Apply device config settings 
			pDeviceSocket->SendRequest_WriteDeviceConfig( pDeviceConfigRecord.get() );
		}
	}
	catch( CAppException& ex )
	{
		AfxMessageBox( ex.GetMessage().c_str(), MB_ICONERROR );
	}

	return 0;
}

/// <summary>
/// Called when [start timer message].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnStartTimerMessage( WPARAM wParam, LPARAM lParam )
{
	UINT messageID = static_cast< UINT >( wParam );
	UINT timeLapse = static_cast< UINT >( lParam );

	m_messageToTimerIdMap[ messageID ] = SetTimer( messageID, timeLapse, NULL );

	return 0;
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support to process messages to/from Netburner Devices on 
///				a System with multiple Network Interfaces (NICs)
///
/// ]]>
/// <summary>
/// Called when [apply application configuration settings].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnApplyAppConfigSettings( WPARAM wParam, LPARAM lParam )
{
	try
	{
		// This smart pointer will take care of releasing the heap memory
		std::unique_ptr< AppConfigRecord > pAppConfigRecord( ( AppConfigRecord * )lParam );
		ASSERT( pAppConfigRecord );

		for( const auto& pApplicationSocket : m_applicationSocketList )
		{
			ASSERT( pApplicationSocket );
			// Apply Application config settings 
			pApplicationSocket->SendRequest_WriteApplicationConfig( pAppConfigRecord.get() );
		}
	}
	catch( CAppException& ex )
	{
		AfxMessageBox( ex.GetMessage().c_str(), MB_ICONERROR );
	}

	return 0;
}

/// <summary>
/// Called when [select first device].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnSelectFirstDevice( WPARAM wParam, LPARAM lParam )
{
	m_wndTreeView.SelectFirstDevice();

	return 0;
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support to process messages to/from Netburner Devices on 
///				a System with multiple Network Interfaces (NICs)
///
/// ]]>
/// <summary>
/// Called when [blink SPU3 LEDS].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnBlinkSPU3LEDs( WPARAM wParam, LPARAM lParam )
{
	try
	{
		// This smart pointer will take care of releasing the heap memory
		std::unique_ptr< DeviceOperationRecord > pDeviceOperationRecord( ( DeviceOperationRecord * )lParam );
		ASSERT( pDeviceOperationRecord );

		for( const auto& pApplicationSocket : m_applicationSocketList )
		{
			ASSERT( pApplicationSocket );
			// Send Device operation = Blink LEDs
			pApplicationSocket->SendRequest_BlinkSPU3LEDs( pDeviceOperationRecord.get() );
		}
	}
	catch( CAppException& ex )
	{
		AfxMessageBox( ex.GetMessage().c_str(), MB_ICONERROR );
	}

	return 0;
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support to process messages to/from Netburner Devices on 
///				a System with multiple Network Interfaces (NICs)
///
/// ]]>
/// <summary>
/// Called when [reboot SPU3 device].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnRebootSPU3Device( WPARAM wParam, LPARAM lParam )
{
	try
	{
		// This smart pointer will take care of releasing the heap memory
		std::unique_ptr< DeviceOperationRecord > pDeviceOperationRecord( ( DeviceOperationRecord * )lParam );
		ASSERT( pDeviceOperationRecord );

		for( const auto& pApplicationSocket : m_applicationSocketList )
		{
			ASSERT( pApplicationSocket );
			// Send Device operation = Reboot SPU3 Device
			pApplicationSocket->SendRequest_RebootSPU3( pDeviceOperationRecord.get() );
		}
	}
	catch( CAppException& ex )
	{
		AfxMessageBox( ex.GetMessage().c_str(), MB_ICONERROR );
	}

	return 0;
}

/// <summary>
/// Called when [timer].
/// </summary>
/// <param name="nIDEvent">The n identifier event.</param>
void CMainFrame::OnTimer( UINT_PTR nIDEvent )
{
	PostMessage( nIDEvent, 0, 0 );

	if( m_messageToTimerIdMap.find( nIDEvent ) != m_messageToTimerIdMap.end() )
	{
		KillTimer( m_messageToTimerIdMap[ nIDEvent ] );
	}

	CFrameWndEx::OnTimer( nIDEvent );
}

/// <summary>
/// Called when [clear caption bar message].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnClearCaptionBarMessage( WPARAM wParam, LPARAM lParam )
{
	DisplayNotification( CString( _T( "" ) ) );

	return 0;
}

/// <summary>
/// Called when [display caption bar message].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnDisplayCaptionBarMessage( WPARAM wParam, LPARAM lParam )
{
	std::unique_ptr< CString > pMessageText( ( CString * )lParam );
	ASSERT( pMessageText );

	DisplayNotification( *pMessageText );

	// Clear the caption bar message after five seconds automatically
	PostMessage( AM_START_TIMER_MSG, ( WPARAM )AM_CLEAR_CAPTION_BAR_MSG, ( LPARAM )5000 );

	return 0;
}

/// <summary>
/// Called when [log message].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnLogMessage( WPARAM wParam, LPARAM lParam )
{
	std::unique_ptr< CString > pMessageText( ( CString * )lParam );
	LogMessage( *pMessageText );

	return 0;
}

/// <summary>
/// To the ip address.
/// </summary>
/// <param name="IPAddress">The ip address.</param>
/// <returns></returns>
CString CMainFrame::ToIPAddress( BYTE IPAddress[ IPADDRESS_SIZE ] )
{
	CString strIPAddress = _T( "" );
	strIPAddress.Format( _T( "%d.%d.%d.%d" ), IPAddress[ 0 ],
						 IPAddress[ 1 ],
						 IPAddress[ 2 ],
						 IPAddress[ 3 ] );

	return strIPAddress;
}

/// <summary>
/// Creates the device status map.
/// </summary>
void CMainFrame::CreateDeviceStatusMap()
{
	try
	{
		m_deviceStatusToDescriptionMap.clear();

		m_deviceStatusToDescriptionMap[ eSPU3DEV_DUPLICATE_IPADDRESS ] = _T( "Duplicate IP Address." );
		m_deviceStatusToDescriptionMap[ eSPU3DEV_FOUND_IN_SYSTEM_XML ] = _T( "IP Address is found in the System XML File." );
		m_deviceStatusToDescriptionMap[ eSPU3DEV_FOUND_IN_HISTORY_FILE ] = _T( "MAC is found in the History File." );
		m_deviceStatusToDescriptionMap[ eSPU3DEV_NOT_FOUND_IN_SYSTEM_XML ] = _T( "IP Address is NOT found in the System XML File." );
		m_deviceStatusToDescriptionMap[ eSPU3DEV_NOT_FOUND_IN_HISTORY_FILE ] = _T( "MAC is NOT found in the History File." );
	}
	catch( CException* ex )
	{
		ex->ReportError();
		ex->Delete();
	}
}

/// <summary>
/// Called when [scan device tree].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnScanDeviceTree( WPARAM wParam, LPARAM lParam )
{
	CMCSystem *pSystem = GetSystem();
	ASSERT( pSystem );

	CMCPress *pPress = pSystem->GetPress();
	ASSERT( pPress );

	// Get the list of SPUs from the System XML file
	std::vector< CString > spuXMLIPAddressList;
	pPress->GetSPUIPAddressList( spuXMLIPAddressList );

	// Get the list of SPUs live on the Network
	std::vector< CString > spuLiveIPAddressList;
	GetLiveSPUIPAddressList( spuLiveIPAddressList );

	std::map< HTREEITEM, std::pair< CString, CString > > treeItemToDeviceMap;
	m_wndTreeView.GetTreeItemToDeviceMap( treeItemToDeviceMap );

	for( auto treeItemIter : treeItemToDeviceMap )
	{
		HTREEITEM hItem = treeItemIter.first;
		ASSERT( hItem );

		CString MACAddress = treeItemIter.second.first;
		CString IPAddress = treeItemIter.second.second;

		if( MACAddress.IsEmpty() || IPAddress.IsEmpty() )
		{
			continue;
		}

		std::vector< int >::iterator::difference_type result =
			std::count( spuLiveIPAddressList.begin(), spuLiveIPAddressList.end(), IPAddress );

		bool duplicateSPU3 = ( result > 1 );
		if( duplicateSPU3 )
		{
			CString messageText = _T( "" );
			messageText.Format( _T( "Found SPU3 Device with duplicate IP Address = %s on the Network." ), IPAddress );
			LogMessage( messageText );
			// Post a message to the Tree view to update the duplicate SPU3 status
			m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_IMAGE, ( WPARAM )TREE_ITEM_IMAGE_RED, ( LPARAM )hItem );
			// Post a message to the Tree view to set the Tree item data 
			m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_DATA, ( WPARAM )eSPU3DEV_DUPLICATE_IPADDRESS, ( LPARAM )hItem );
			continue;
		}

		// Is System XML file and History file loaded?
		if( m_bSystemXMLFileLoaded && m_bHistoryFileLoaded )
		{
			bool foundInSystemXML = std::find( spuXMLIPAddressList.begin(), spuXMLIPAddressList.end(), IPAddress ) != spuXMLIPAddressList.end();
			bool foundInHistoryFile = m_spuHistoryFileMap.find( MACAddress ) != m_spuHistoryFileMap.end();

			// Post a message to the Tree view to update the status of SPU3
			if( foundInSystemXML && foundInHistoryFile )
			{
				CString messageText = _T( "" );
				messageText.Format( _T( "SPU3 Device with MAC = %s and IP Address = %s is found in the System XML File and also in the History File." ), MACAddress, IPAddress );
				LogMessage( messageText );

				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_IMAGE, ( WPARAM )TREE_ITEM_IMAGE_GREEN, ( LPARAM )hItem );
				// Post a message to the Tree view to set the Tree item data 
				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_DATA, ( WPARAM )( eSPU3DEV_FOUND_IN_SYSTEM_XML | eSPU3DEV_FOUND_IN_HISTORY_FILE ), ( LPARAM )hItem );
			}
			else if( foundInSystemXML && !foundInHistoryFile )
			{
				CString messageText = _T( "" );
				messageText.Format( _T( "SPU3 Device with MAC = %s and IP Address = %s is found in the System XML File. But NOT in the History File." ), MACAddress, IPAddress );
				LogMessage( messageText );

				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_IMAGE, ( WPARAM )TREE_ITEM_IMAGE_YELLOW, ( LPARAM )hItem );
				// Post a message to the Tree view to set the Tree item data 
				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_DATA, ( WPARAM )( eSPU3DEV_FOUND_IN_SYSTEM_XML | eSPU3DEV_NOT_FOUND_IN_HISTORY_FILE ), ( LPARAM )hItem );
			}
			else if( !foundInSystemXML && foundInHistoryFile )
			{
				CString messageText = _T( "" );
				messageText.Format( _T( "SPU3 Device with MAC = %s and IP Address = %s is NOT found in the System XML File. But found in the History File." ), MACAddress, IPAddress );
				LogMessage( messageText );

				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_IMAGE, ( WPARAM )TREE_ITEM_IMAGE_YELLOW, ( LPARAM )hItem );
				// Post a message to the Tree view to set the Tree item data 
				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_DATA, ( WPARAM )( eSPU3DEV_NOT_FOUND_IN_SYSTEM_XML | eSPU3DEV_FOUND_IN_HISTORY_FILE ), ( LPARAM )hItem );
			}
			else
			{
				CString messageText = _T( "" );
				messageText.Format( _T( "SPU3 Device with MAC = %s and IP Address = %s is NOT found in the System XML File and also in the Histiry File." ), MACAddress, IPAddress );
				LogMessage( messageText );

				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_IMAGE, ( WPARAM )TREE_ITEM_IMAGE_RED, ( LPARAM )hItem );
				// Post a message to the Tree view to set the Tree item data 
				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_DATA, ( WPARAM )( eSPU3DEV_NOT_FOUND_IN_SYSTEM_XML | eSPU3DEV_NOT_FOUND_IN_HISTORY_FILE ), ( LPARAM )hItem );
			}

			continue;
		}

		// Is System XML file loaded and History file NOT loaded?
		if( m_bSystemXMLFileLoaded )
		{
			bool foundInSystemXML = std::find( spuXMLIPAddressList.begin(), spuXMLIPAddressList.end(), IPAddress ) != spuXMLIPAddressList.end();
			int treeItemImage = ( foundInSystemXML ? TREE_ITEM_IMAGE_GREEN : TREE_ITEM_IMAGE_RED );

			if( foundInSystemXML )
			{
				CString messageText = _T( "" );
				messageText.Format( _T( "SPU3 Device with MAC = %s and IP Address = %s is found in the System XML File" ), MACAddress, IPAddress );
				LogMessage( messageText );
				// Post a message to the Tree view to set the Tree item data 
				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_DATA, ( WPARAM )eSPU3DEV_FOUND_IN_SYSTEM_XML, ( LPARAM )hItem );
			}
			else
			{
				CString messageText = _T( "" );
				messageText.Format( _T( "SPU3 Device with MAC = %s and IP Address = %s is NOT found in the System XML File" ), MACAddress, IPAddress );
				LogMessage( messageText );
				// Post a message to the Tree view to set the Tree item data 
				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_DATA, ( WPARAM )eSPU3DEV_NOT_FOUND_IN_SYSTEM_XML, ( LPARAM )hItem );
			}

			// Post a message to the Tree view to update the status of SPU3
			m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_IMAGE, ( WPARAM )treeItemImage, ( LPARAM )hItem );
		}

		// Is History file loaded and System XML file NOT loaded?
		if( m_bHistoryFileLoaded )
		{
			bool foundInHistoryFile = m_spuHistoryFileMap.find( MACAddress ) != m_spuHistoryFileMap.end();
			int treeItemImage = ( foundInHistoryFile ? TREE_ITEM_IMAGE_GREEN : TREE_ITEM_IMAGE_RED );

			if( foundInHistoryFile )
			{
				CString messageText = _T( "" );
				messageText.Format( _T( "SPU3 Device with MAC = %s and IP Address = %s is found in the History File" ), MACAddress, IPAddress );
				LogMessage( messageText );
				// Post a message to the Tree view to set the Tree item data 
				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_DATA, ( WPARAM )eSPU3DEV_FOUND_IN_HISTORY_FILE, ( LPARAM )hItem );
			}
			else
			{
				CString messageText = _T( "" );
				messageText.Format( _T( "SPU3 Device with MAC = %s and IP Address = %s is NOT found in the History File" ), MACAddress, IPAddress );
				LogMessage( messageText );
				// Post a message to the Tree view to set the Tree item data 
				m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_DATA, ( WPARAM )eSPU3DEV_NOT_FOUND_IN_HISTORY_FILE, ( LPARAM )hItem );
			}

			// Post a message to the Tree view to update the status of SPU3
			m_wndTreeView.SendMessage( AM_UPDATE_TREE_ITEM_IMAGE, ( WPARAM )treeItemImage, ( LPARAM )hItem );
		}
	}

	return 0;
}

/// <summary>
/// Called when [update statusbar text].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnUpdateStatusbarText( WPARAM wParam, LPARAM lParam )
{
	int viewSelected = static_cast< int >( wParam );

	CString statusBarText = _T( "" );
	if( AM_DISPLAY_ALL_DEVICES_VIEW == viewSelected )
	{
		statusBarText = GetStatusbarText_AllDevicesView();
	}
	else if( AM_DISPLAY_NO_DEVICES_VIEW == viewSelected )
	{
		statusBarText = _T( "No SPU3 Devices Found on the Network." );
	}
	else if( AM_POPULATE_DEVICE_SETTINGS == viewSelected )
	{
		statusBarText = GetSelectedDeviceStatus();
	}

	// Set the Statusbar text
	m_wndStatusBar.SetWindowText( statusBarText );

	// Let's log the message
	if( !statusBarText.IsEmpty() )
	{
		LogMessage( statusBarText );
	}

	return 0;
}

/// <summary>
/// Gets the statusbar text all devices view.
/// </summary>
/// <returns></returns>
CString CMainFrame::GetStatusbarText_AllDevicesView()
{
	CString statusBarText = _T( "List of SPU3 Devices Found on the Network." );

	// Get the #of SPU3s found on the Network
	int spuCountFromNetwork = static_cast< int >( m_treeItemMap.size() );

	if( m_bHistoryFileLoaded && !m_bSystemXMLFileLoaded )
	{
		// Get the #of SPU3s from the History File
		int spuCountFromHistFile = static_cast< int >( m_spuHistoryFileMap.size() );

		CString messageText = _T( "" );
		if( spuCountFromNetwork < spuCountFromHistFile )
		{
			messageText.Format( _T( "The History File contains data for %d SPU3 Devices. But there are only %d SPU3 Devices found on the Network. So, check the SPU3 Device connections." ),
								spuCountFromHistFile, spuCountFromNetwork );

			statusBarText = messageText;
		}
		else if( spuCountFromNetwork > spuCountFromHistFile )
		{
			messageText.Format( _T( "The History File contains data for %d SPU3 Devices. But there are %d SPU3 Devices found on the Network." ),
								spuCountFromHistFile, spuCountFromNetwork );

			statusBarText = messageText;
		}
	}
	else if( m_bSystemXMLFileLoaded )
	{
		CMCSystem *pSystem = GetSystem();
		ASSERT( pSystem );
		CMCPress *pPress = pSystem->GetPress();
		ASSERT( pPress );

		// Get the #of SPU3 devices from the System XML file
		int spuCountFromSysXML = pPress->GetSPU3DeviceCount();

		CString messageText = _T( "" );
		if( spuCountFromNetwork < spuCountFromSysXML )
		{
			messageText.Format( _T( "The System XML File is configured for %d SPU3 Devices. But there are only %d SPU3 Devices found on the Network. So, check the SPU3 Device connections." ),
								spuCountFromSysXML, spuCountFromNetwork );

			statusBarText = messageText;
		}
		else if( spuCountFromNetwork > spuCountFromSysXML )
		{
			messageText.Format( _T( "The System XML File is configured for %d SPU3 Devices. But there are %d SPU3 Devices found on the Network." ),
								spuCountFromSysXML, spuCountFromNetwork );

			statusBarText = messageText;
		}
	}

	return statusBarText;
}

/// <summary>
/// Gets the selected device status.
/// </summary>
/// <returns></returns>
CString CMainFrame::GetSelectedDeviceStatus()
{
	HTREEITEM hItem = m_wndTreeView.GetSelectedTreeItem();
	ASSERT( hItem );

	CString MACAddress = _T( "" );
	CString IPAddress = _T( "" );
	// Get MAC Address and IP Address of the selected Tree item
	GetMACAddressAndIPAddressOfTreeItem( hItem, MACAddress, IPAddress );

	CString statusBarText = _T( "" );
	// Get the Tree item data
	DWORD itemData = m_wndTreeView.GetItemData( hItem );
	statusBarText.Format( _T( "SPU3 : MAC = %s, IP Address = %s. %s" ),
						  MACAddress,
						  IPAddress,
						  GetDeviceStatusDescription( itemData ).GetString() );

	return statusBarText;
}

/// <summary>
/// Gets the device status description.
/// </summary>
/// <param name="treeItemData">The tree item data.</param>
/// <returns></returns>
CString CMainFrame::GetDeviceStatusDescription( DWORD treeItemData )
{
	CString description = _T( "" );

	if( treeItemData & eSPU3DEV_DUPLICATE_IPADDRESS )
	{
		if( m_deviceStatusToDescriptionMap.find( eSPU3DEV_DUPLICATE_IPADDRESS ) != m_deviceStatusToDescriptionMap.end() )
			description += _T( " " ) + m_deviceStatusToDescriptionMap[ eSPU3DEV_DUPLICATE_IPADDRESS ];
	}

	if( treeItemData & eSPU3DEV_FOUND_IN_SYSTEM_XML )
	{
		if( m_deviceStatusToDescriptionMap.find( eSPU3DEV_FOUND_IN_SYSTEM_XML ) != m_deviceStatusToDescriptionMap.end() )
			description += _T( " " ) + m_deviceStatusToDescriptionMap[ eSPU3DEV_FOUND_IN_SYSTEM_XML ];
	}

	if( treeItemData & eSPU3DEV_FOUND_IN_HISTORY_FILE )
	{
		if( m_deviceStatusToDescriptionMap.find( eSPU3DEV_FOUND_IN_HISTORY_FILE ) != m_deviceStatusToDescriptionMap.end() )
			description += _T( " " ) + m_deviceStatusToDescriptionMap[ eSPU3DEV_FOUND_IN_HISTORY_FILE ];
	}

	if( treeItemData & eSPU3DEV_NOT_FOUND_IN_SYSTEM_XML )
	{
		if( m_deviceStatusToDescriptionMap.find( eSPU3DEV_NOT_FOUND_IN_SYSTEM_XML ) != m_deviceStatusToDescriptionMap.end() )
			description += _T( " " ) + m_deviceStatusToDescriptionMap[ eSPU3DEV_NOT_FOUND_IN_SYSTEM_XML ];
	}

	if( treeItemData & eSPU3DEV_NOT_FOUND_IN_HISTORY_FILE )
	{
		if( m_deviceStatusToDescriptionMap.find( eSPU3DEV_NOT_FOUND_IN_HISTORY_FILE ) != m_deviceStatusToDescriptionMap.end() )
			description += _T( " " ) + m_deviceStatusToDescriptionMap[ eSPU3DEV_NOT_FOUND_IN_HISTORY_FILE ];
	}

	return description;
}

/// <summary>
/// Resets the file read only attribute.
/// </summary>
/// <param name="historyFileFullPath">The history file full path.</param>
void CMainFrame::ResetFileReadOnlyAttribute( const CString& historyFileFullPath )
{
	DWORD fileAttributes = GetFileAttributes( historyFileFullPath );
	if( fileAttributes != INVALID_FILE_ATTRIBUTES )
	{
		bool isReadOnly = ( fileAttributes & FILE_ATTRIBUTE_READONLY );
		if( isReadOnly )
		{
			fileAttributes = ( fileAttributes & ~FILE_ATTRIBUTE_READONLY );
			::SetFileAttributes( historyFileFullPath, fileAttributes );
		}
	}
}

/// <summary>
/// Called when [create history file].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CMainFrame::OnCreateHistoryFile( WPARAM wParam, LPARAM lParam )
{
	// History file name
	CString historyFileFullPath = theApp.m_logFolderPath + _T( "\\" ) + SPU3EthernetIPConfigHistoryFileName;

	// Reset Readonly file attributes, if it was set already
	ResetFileReadOnlyAttribute( historyFileFullPath );

	std::wofstream historyStream( historyFileFullPath.GetBuffer( historyFileFullPath.GetLength() ) );
	if( !historyStream.is_open() )
	{
		// failed to open the report file
		CString errorMessage = _T( "" );
		errorMessage.Format( _T( "Failed to open the History File = %s." ), historyFileFullPath );
		AfxMessageBox( errorMessage, MB_ICONERROR );
		return 0;
	}

	std::map< CString, TreeItemRecord > allDevConfigSettings;
	GetAllDevicesConfigSettings( allDevConfigSettings );

	try
	{
		std::wostringstream writeStream( _T( "" ) );
		//For each SPU3 Device write the MAC Address and IP Address ( separated by comma(,) ) to the history file
		for( auto deviceSettingIter : allDevConfigSettings )
		{
			CString spuMACAddress = deviceSettingIter.first;
			CString spuIPAddress = ToIPAddress( deviceSettingIter.second.m_devConfigRecord.ip_Addr );

			writeStream << spuMACAddress.GetString()
				<< _T( "," )
				<< spuIPAddress.GetString()
				<< std::endl;
		}

		historyStream << writeStream.str().c_str();
	}
	catch( CException* ex )
	{
		ex->ReportError();
		ex->Delete();
	}

	historyStream.close();

	// Display the history file location to the user
	CString historyResultMessage = _T( "" );
	historyResultMessage.Format( _T( "History File Created Successfully.\r\n\r\n%s" ), historyFileFullPath );
	AfxMessageBox( historyResultMessage, MB_ICONINFORMATION );

	// Post the message AM_LOAD_HISTORY_FILE to the Main Window to load the updated History file
	PostMessage( AM_LOAD_HISTORY_FILE, 0, 0 );

	return 0;
}

/// <summary>
/// Checks the and create history file.
/// </summary>
void CMainFrame::CheckAndCreateHistoryFile()
{
	// If the History file does NOT exists, then ask the user to create the History file before exiting.	
	CString historyFileFullPath = theApp.m_logFolderPath + _T( "\\" ) + SPU3EthernetIPConfigHistoryFileName;
	int spu3CountFromNetwork = static_cast< int >( m_treeItemMap.size() );

	// Is History file exists? 
	if( !PathFileExists( historyFileFullPath ) )
	{
		// Ensure that at least one SPU3 Device is found on the Network
		if( spu3CountFromNetwork > 0 )
		{
			CString messageText = _T( "" );
			messageText.Format( _T( "History File does NOT exists.\r\n\r\nDo you want to create the History File now?" ) );
			if( IDYES == AfxMessageBox( messageText, MB_YESNO | MB_ICONQUESTION ) )
			{
				PostMessage( AM_CREATE_HISTORY_FILE, 0, 0 );
			}
		}
	}

	if( m_bHistoryFileLoaded )
	{
		// Get the list of SPUs from the History File
		int spu3CountFromHistoryFile = static_cast< int >( m_spuHistoryFileMap.size() );
		if( spu3CountFromNetwork > 0 )
		{
			if( spu3CountFromHistoryFile != spu3CountFromNetwork )
			{
				CString messageText = _T( "" );
				messageText.Format( _T( "History File data is NOT in sync with the SPU3 Devices found on the Network.\r\n\r\nDo you want to update the History File now?" ), historyFileFullPath );
				if( IDYES == AfxMessageBox( messageText, MB_YESNO | MB_ICONQUESTION ) )
				{
					PostMessage( AM_CREATE_HISTORY_FILE, 0, 0 );
				}
			}
		}
	}
}

/// <summary>
/// Raises the Close event.
/// </summary>
void CMainFrame::OnClose()
{
	// Before closing the Applicaton, check for the History file.
	CheckAndCreateHistoryFile();

	CString messageText = _T( "" );
	messageText.LoadString( IDS_EXIT_APPLICATION );
	if( IDNO == AfxMessageBox( messageText, MB_YESNO | MB_ICONQUESTION ) )
	{
		return;
	}

	CFrameWndEx::OnClose();
}

/// <summary>
/// Called when [system command].
/// </summary>
/// <param name="nID">The n identifier.</param>
/// <param name="lParam">The l parameter.</param>
void CMainFrame::OnSysCommand( UINT nID, LPARAM lParam )
{
	if( ( nID & 0xFFF0 ) == ID_APP_ABOUT )
	{
		CAboutDlg aboutDlg;
		aboutDlg.DoModal();
	}

	CFrameWndEx::OnSysCommand( nID, lParam );
}

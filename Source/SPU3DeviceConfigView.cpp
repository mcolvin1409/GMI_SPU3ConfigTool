// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="SPU3DeviceConfigView.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

// SPU3AutoIPConfigToolView.cpp : implementation of the CSPU3AutoIPConfigToolView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SPU3AutoIPConfigTool.h"
#endif

#include <memory>
#include "SPU3AutoIPConfigToolDoc.h"
#include "SPU3DeviceConfigView.h"
#include "Defines.h"
#include "MainFrm.h"
#include "MCSystem.h"
#include "MCPress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSPU3AutoIPConfigToolView

IMPLEMENT_DYNCREATE(CSPU3DeviceConfigView, CDialogEx )

BEGIN_MESSAGE_MAP(CSPU3DeviceConfigView, CDialogEx )
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_MESSAGE( AM_POPULATE_DEVICE_SETTINGS, &CSPU3DeviceConfigView::OnPopulateDeviceSettings )
	ON_MESSAGE( AM_UPDATE_SYS_SPU3_IPADDR, &CSPU3DeviceConfigView::OnUpdateSystemSPU3IPAddresses )
	ON_CBN_SELCHANGE( IDC_COMBO_CHG_SPU3_IPADDR_TO, &CSPU3DeviceConfigView::OnSPUSelectionChanged )
	ON_BN_CLICKED( IDC_BUTTON_APPLY_CHANGES, &CSPU3DeviceConfigView::OnBnClickedButtonApplyChanges )	
	ON_BN_CLICKED( IDC_BUTTON_REBOOT_SPU3, &CSPU3DeviceConfigView::OnBnClickedButtonReboot )
	ON_CBN_SELCHANGE( IDC_COMBO_SPU3_COMM_TYPE, &CSPU3DeviceConfigView::OnCommTypeSelectionChange )
	ON_BN_CLICKED( IDC_CHECK__BLINK_SPU3_LEDS, &CSPU3DeviceConfigView::OnBnClickedButtonBlinkLEDS )
END_MESSAGE_MAP()

// CSPU3AutoIPConfigToolView construction/destruction

/// <summary>
/// Initializes a new instance of the <see cref="CSPU3DeviceConfigView"/> class.
/// </summary>
/// <param name="pParent">The p parent.</param>
CSPU3DeviceConfigView::CSPU3DeviceConfigView( CWnd* pParent /*=NULL*/ )
	: CDialogEx( CSPU3DeviceConfigView::IDD, pParent )
	, m_socketPort( 0 )	
	, m_portOrientation( Right_To_Left )
	, m_commType( 0 )
	, m_strSPU3FWVersion( _T( "" ) )
	, m_blinkSPU3LEDs( FALSE )		
	, m_spuFWPerformanceModeOption( 0 )
{
	Create( CSPU3DeviceConfigView::IDD, pParent );
}

/// <summary>
/// Finalizes an instance of the <see cref="CSPU3DeviceConfigView"/> class.
/// </summary>
CSPU3DeviceConfigView::~CSPU3DeviceConfigView()
{
	if( NULL != m_hBlinkNotActiveBitmap )
		DeleteObject( m_hBlinkNotActiveBitmap );

	if( NULL != m_hBlinkActiveBitmap )
		DeleteObject( m_hBlinkActiveBitmap );
}

/// <summary>
/// Does the data exchange.
/// </summary>
/// <param name="pDX">The data exchange.</param>
void CSPU3DeviceConfigView::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT_SOCK_PORT, m_socketPort );
	DDX_CBIndex( pDX, IDC_COMBO_SPU3_PORT_ORIENTATION, m_portOrientation );
	DDV_MinMaxInt( pDX, m_portOrientation, 0, 1 );
	DDX_CBIndex( pDX, IDC_COMBO_SPU3_COMM_TYPE, m_commType );
	DDV_MinMaxInt( pDX, m_commType, 0, 1 );
	DDX_Text( pDX, IDC_EDIT_SPU3_FW_VERSION, m_strSPU3FWVersion );
	DDX_Control( pDX, IDC_IPADDRESS_MERCURY_SERVER, m_serverIPAddressCtrl );
	DDX_Control( pDX, IDC_IPADDRESS_SPU3, m_spu3IPAddressCtrl );
	DDX_Control( pDX, IDC_SUBNET_MASK_SPU3, m_spu3SubnetMaskCtrl );
	DDX_Control( pDX, IDC_COMBO_CHG_SPU3_IPADDR_TO, m_comboChangeIPAddressToCtrl );
	DDX_Control( pDX, IDC_CHECK_CHG_SPU3_IPADDR_TO, m_checkboxChangeIPAddressToCtrl );
	DDX_Control( pDX, IDC_LIST_PORT_INKERS, m_portAndInkerNameListCtrl );
	DDX_Control( pDX, IDC_EDIT_SOCK_PORT, m_socketPortCtrl );
	DDX_Control( pDX, IDC_COMBO_SPU3_COMM_TYPE, m_commTypeCtrl );
	DDX_Control( pDX, IDC_COMBO_SPU3_PORT_ORIENTATION, m_portOrientationCtrl );
	DDX_Check( pDX, IDC_CHECK__BLINK_SPU3_LEDS, m_blinkSPU3LEDs );
	DDX_Control( pDX, IDC_STATIC_SPU3_DEVICE_SETTINGS, m_SPU3DeviceSettingsGroupBox );
	DDX_Control( pDX, IDC_STATIC_SERVER_SETTINGS, m_MercuryServerSettingsGroupBox );
	DDX_Control( pDX, IDC_BUTTON_APPLY_CHANGES, m_applyButton );
	DDX_Control( pDX, IDC_CHECK__BLINK_SPU3_LEDS, m_blinkLEDSButton );
	DDX_Control( pDX, IDC_BUTTON_REBOOT_SPU3, m_rebootSPU3Button );
	DDX_Control( pDX, IDC_STATIC_SPU_NAME, m_SPU3NameFromSystemXML );
	DDX_Control( pDX, IDC_CHECK_ENABLE_PERFORMANCE_MODE, m_spuFWPerformanceModeOptionCtrl );
	DDX_Check( pDX, IDC_CHECK_ENABLE_PERFORMANCE_MODE, m_spuFWPerformanceModeOption );
}

/// <summary>
/// Called when [initialize dialog].
/// </summary>
/// <returns></returns>
BOOL CSPU3DeviceConfigView::OnInitDialog()
{
	CDialogEx::OnInitDialog();
				
	// Load Initial Settings
	LoadInitialSettings();	

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/// <summary>
/// Loads the initial settings.
/// </summary>
void CSPU3DeviceConfigView::LoadInitialSettings()
{	
	CreateToolTips();
	
	// Load Blink bitmap image
	m_hBlinkActiveBitmap = ( HBITMAP )LoadImage(
		AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE( IDB_BITMAP_BLINK_LED ),
		IMAGE_BITMAP,
		0, 0, // use actual size
		LR_DEFAULTCOLOR
	);

	ASSERT( m_hBlinkActiveBitmap );

	m_hBlinkNotActiveBitmap = ( HBITMAP )LoadImage(
		AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE( IDB_BITMAP_BLINK_LED_NOTACTIVE ),
		IMAGE_BITMAP,
		0, 0, // use actual size
		LR_DEFAULTCOLOR
	);

	ASSERT( m_hBlinkNotActiveBitmap );

	// Set Blink LED button image to NotActive status
	m_blinkLEDSButton.SetBitmap( m_hBlinkNotActiveBitmap );

	AddHeaderToListControl();			
	ShowControls( false );
}

/// <summary>
/// Creates the tool tips.
/// </summary>
void CSPU3DeviceConfigView::CreateToolTips()
{
	// Create Tool tip control
	if( m_toolTipControl.Create( this ) )
	{
		// Add Tool tip control info
		m_toolTipControl.AddTool( &m_blinkLEDSButton, _T( "Blink SPU3 Device LEDs." ) );
		m_toolTipControl.AddTool( &m_rebootSPU3Button, _T( "Reboot SPU3 Device." ) );
		m_toolTipControl.AddTool( &m_applyButton, _T( "Save SPU3 Device and Application Settings." ) );
	}
}

/// <summary>
/// Adds the header to list control.
/// </summary>
void CSPU3DeviceConfigView::AddHeaderToListControl()
{
	m_portAndInkerNameListCtrl.SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES );

	CRect rect;
	m_portAndInkerNameListCtrl.GetClientRect( &rect );
	int nColInterval = rect.Width() / 3;

	m_portAndInkerNameListCtrl.InsertColumn( 0, _T( "Port" ), LVCFMT_LEFT, nColInterval );
	m_portAndInkerNameListCtrl.InsertColumn( 1, _T( "Inker Name" ), LVCFMT_LEFT, nColInterval * 2 );	
}

/// <summary>
/// Shows the controls.
/// </summary>
/// <param name="bShow">if set to <c>true</c> [b show].</param>
void CSPU3DeviceConfigView::ShowControls( bool bShow )
{
	int nCmdShow = ( bShow ? SW_SHOW : SW_HIDE );

	m_comboChangeIPAddressToCtrl.ShowWindow( nCmdShow );
	m_checkboxChangeIPAddressToCtrl.ShowWindow( nCmdShow );	
	m_portAndInkerNameListCtrl.ShowWindow( nCmdShow );
	m_SPU3NameFromSystemXML.ShowWindow( nCmdShow );	
}

/// <summary>
/// Enables the button controls.
/// </summary>
/// <param name="bEnable">if set to <c>true</c> [b enable].</param>
void CSPU3DeviceConfigView::EnableButtonControls( bool bEnable )
{	
	m_rebootSPU3Button.EnableWindow( bEnable );
	m_blinkLEDSButton.EnableWindow( bEnable );
	m_applyButton.EnableWindow( bEnable );
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support for Performance Mode option
///
/// ]]>
/// <summary>
/// Called when [populate device settings].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CSPU3DeviceConfigView::OnPopulateDeviceSettings( WPARAM wParam, LPARAM lParam )
{
	std::unique_ptr< TreeItemRecord > pTreeItem( ( TreeItemRecord * )lParam );
	ASSERT( pTreeItem );

	// Populate the Device Settings of the selected Device Item from the Tree View
	DeviceConfigRecord deviceConfigRecord = pTreeItem->m_devConfigRecord;
	AppConfigRecord	appConfigRecord = pTreeItem->m_appConfigRecord;
	DeviceRuntimeStatus devRuntimeStatus = pTreeItem->m_devRuntimeStatus;

	m_spu3IPAddressCtrl.SetAddress( deviceConfigRecord.ip_Addr[ 0 ],
								deviceConfigRecord.ip_Addr[ 1 ],
								deviceConfigRecord.ip_Addr[ 2 ],
								deviceConfigRecord.ip_Addr[ 3 ] );

	m_spu3SubnetMaskCtrl.SetAddress( deviceConfigRecord.ip_Mask[ 0 ],
								 deviceConfigRecord.ip_Mask[ 1 ],
								 deviceConfigRecord.ip_Mask[ 2 ],
								 deviceConfigRecord.ip_Mask[ 3 ] );

	m_strSPU3FWVersion.Format( _T( "%d.%d    %d %d %d" ),
							   appConfigRecord.spuVersionMajor,
							   appConfigRecord.spuVersionMinor,
							   appConfigRecord.spuFWBuildYear,
							   appConfigRecord.spuFWBuildMonth,
							   appConfigRecord.spuFWBuildDay );

	m_commType = appConfigRecord.commType;
	m_portOrientation = appConfigRecord.portOrientation;
	// Reset the status of the 'Change IP Address To?' check box 
	m_checkboxChangeIPAddressToCtrl.SetCheck( BST_UNCHECKED );

	// Mercury App Server Settings
	m_serverIPAddressCtrl.SetAddress( appConfigRecord.serverIPAddress[ 0 ],
								  appConfigRecord.serverIPAddress[ 1 ],
								  appConfigRecord.serverIPAddress[ 2 ],
								  appConfigRecord.serverIPAddress[ 3 ] );

	m_socketPort = appConfigRecord.serverSockPort;
	m_spuFWPerformanceModeOption = appConfigRecord.spuFWPerformanceModeOption;		

	m_serverIPAddressCtrl.EnableWindow( Ethernet_COMM == m_commType );
	m_socketPortCtrl.EnableWindow( Ethernet_COMM == m_commType );
	m_spuFWPerformanceModeOptionCtrl.EnableWindow( Ethernet_COMM == m_commType );

	m_blinkSPU3LEDs = devRuntimeStatus.m_blinkLEDs;
	// Load bitmap of Blink LED button as per the Blink LED status
	LoadBlinkLEDButtonBitmap();
	
	UpdateData( FALSE );

	EnableButtonControls( true );	

	return 0;
}

/// <summary>
/// Called when [update system SPU3 ip addresses].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CSPU3DeviceConfigView::OnUpdateSystemSPU3IPAddresses( WPARAM wParam, LPARAM lParam )
{
	// Populate SPU3 IP Address list
	PopulateSPU3IPAddressesList();

	int itemCount = m_comboChangeIPAddressToCtrl.GetCount();
	// Select the first SPU IPAddress form the list
	if( itemCount > 0 )
	{
		m_comboChangeIPAddressToCtrl.SetCurSel( 0 );
		OnSPUSelectionChanged();
	}

	// If there are SPUs configured over Ethernet, then show the Change IP Address check box and Combo box
	ShowControls( itemCount > 0 );

	return 0;
}

/// <summary>
/// Populates the SPU3 ip addresses list.
/// </summary>
void CSPU3DeviceConfigView::PopulateSPU3IPAddressesList()
{
	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	CMCSystem *pSystem = pMainWnd->GetSystem();
	ASSERT( pSystem );

	CMCPress* pPress = pSystem->GetPress();
	ASSERT( pPress );

	// Let's clear the items, when System XML file is loaded 
	m_comboChangeIPAddressToCtrl.ResetContent();

	std::vector< CString > spuIPAddressList;
	pPress->GetSPUIPAddressList( spuIPAddressList );

	// Add the SPU3 IP Addresses from the System XML file
	for( CString spuIPAddress : spuIPAddressList )
	{
		m_comboChangeIPAddressToCtrl.AddString( spuIPAddress );
	}
}

/// <summary>
/// Populates the inkers list.
/// </summary>
/// <param name="spuIPAddress">The SPU IP address.</param>
void CSPU3DeviceConfigView::PopulateInkersList( const CString& spuIPAddress )
{
	if( spuIPAddress.IsEmpty() )
	{
		return;
	}

	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	CMCSystem *pSystem = pMainWnd->GetSystem();
	ASSERT( pSystem );

	CMCPress* pPress = pSystem->GetPress();
	ASSERT( pPress );

	std::vector< CMCInker > inkersList;
	pPress->GetInkersListBySPUIPAddress( spuIPAddress, inkersList );

	int row = 0;
	for( CMCInker inker : inkersList )
	{		
		for( auto spuToPortIter : inker.GetRailsList() )
		{			
			for( auto portIter : spuToPortIter.second )
			{
				CString portText = _T( "" );
				portText.Format( _T("%d"), portIter );
				m_portAndInkerNameListCtrl.InsertItem( row, portText );
				m_portAndInkerNameListCtrl.SetItemText( row, 1, inker.GetName() );	
				row++;
			}
		}		
	}
}

/// <summary>
/// Populates the name of the SPU3.
/// </summary>
/// <param name="spuIPAddress">The SPU IP address.</param>
void CSPU3DeviceConfigView::PopulateSPU3Name( const CString& spuIPAddress )
{
	if( spuIPAddress.IsEmpty() )
	{
		return;
	}

	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	CMCSystem *pSystem = pMainWnd->GetSystem();
	ASSERT( pSystem );

	CMCPress* pPress = pSystem->GetPress();
	ASSERT( pPress );

	CString spuName = _T( "" );
	spuName.Format( _T( "SPU Name : %s" ), pPress->GetSPUNameByIPAddress( spuIPAddress ) );

	m_SPU3NameFromSystemXML.SetWindowText( spuName );
}

/// <summary>
/// Called when [spu selection changed].
/// </summary>
void CSPU3DeviceConfigView::OnSPUSelectionChanged()
{
	// Clear the Port and Inkers List control items
	m_portAndInkerNameListCtrl.DeleteAllItems();

	CString spuIPAddress = _T( "" );
	m_comboChangeIPAddressToCtrl.GetLBText( m_comboChangeIPAddressToCtrl.GetCurSel(), spuIPAddress );

	PopulateSPU3Name( spuIPAddress );

	PopulateInkersList( spuIPAddress );
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Changed the order of saving the Application Settings and Device Settings
///
/// ]]>
/// <summary>
/// Called when [bn clicked button apply changes].
/// </summary>
void CSPU3DeviceConfigView::OnBnClickedButtonApplyChanges()
{
	UpdateData( TRUE );

	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );
	
	// Important Note:
	//		Updating the Device settings would reboot the Device (SPU3). 
	//		So, apply the Application settings first, before applying the Device settings. 
	
	bool validAppSettings = false;	
	if( validAppSettings = ApplyApplicationSettings() )
	{
		// Process all of the Windows queued messages 
		ProcessQueuedMessages();
		// Let's give SPU3 Device ample time to process the Application configuration changes
		Sleep( 5000 );	// 5 Seconds
	}

	bool validSPU3DevSettings = false;
	if( validSPU3DevSettings = ApplyDeviceSettings() )
	{
		// Process all of the Windows queued messages
		ProcessQueuedMessages();
	}
	
	if( validSPU3DevSettings || validAppSettings )
	{
		// Start timer to update the Device and Application settings
		pMainWnd->PostMessage( AM_START_TIMER_MSG, ( WPARAM )AM_QUERY_SPU3_DEVICES, ( LPARAM )10000 );
	}
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Apply Device settings
///
/// ]]>
/// <summary>
/// Applies the device settings.
/// </summary>
/// <returns></returns>
bool CSPU3DeviceConfigView::ApplyDeviceSettings()
{
	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	bool systemXMLFileLoaded = pMainWnd->IsSystemXMLFileLoaded();

	// Validate Device configuration changes
	DeviceConfigRecord deviceConfigRecord{ };
	pMainWnd->GetDeviceConfigRecordOfSelectedItem( deviceConfigRecord );
	bool validSPU3DevSettings = UpdateSPU3DeviceSettings( systemXMLFileLoaded, &deviceConfigRecord );

	// Apply Device configuration changes
	if( validSPU3DevSettings )
	{
		EnableButtonControls( false );		

		// Save SPU3 Device settings
		pMainWnd->PostMessage( AM_APPLY_DEVICE_SETTINGS, ( WPARAM )0, ( LPARAM )( new DeviceConfigRecord( deviceConfigRecord ) ) );

		// Display the current operation message on to the caption bar
		CString strMessage = _T( "" );
		strMessage.Format( _T( "Please wait... Applying new settings to the SPU3 Device @ IP Address = %s" ), pMainWnd->ToIPAddress( deviceConfigRecord.ip_Addr ) );

		pMainWnd->PostMessage( AM_DISPLAY_CAPTION_BAR_MSG, ( WPARAM )0, ( LPARAM )( new CString( strMessage ) ) );
		pMainWnd->PostMessage( AM_LOG_MESSAGE, ( WPARAM )0, ( LPARAM )( new CString( strMessage ) ) );
	}

	return validSPU3DevSettings;
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Process the Windows queued messages immediately
///
/// ]]>
/// <summary>
/// Processes the queued messages.
/// </summary>
void CSPU3DeviceConfigView::ProcessQueuedMessages()
{
	MSG message{ };
	while( ::PeekMessage( &message, NULL, 0, 0, PM_REMOVE ) )
	{
		::AfxPreTranslateMessage( &message );
		::TranslateMessage( &message );
		::DispatchMessage( &message );
	}
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Apply Application settings
///
/// ]]>
/// <summary>
/// Applies the application settings.
/// </summary>
/// <returns></returns>
bool CSPU3DeviceConfigView::ApplyApplicationSettings()
{
	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	bool systemXMLFileLoaded = pMainWnd->IsSystemXMLFileLoaded();

	// Validate Application configuration changes
	AppConfigRecord appConfigRecord{ };
	pMainWnd->GetAppConfigRecordOfSelectedItem( appConfigRecord );
	bool validAppSettings = UpdateApplicationSettings( systemXMLFileLoaded, &appConfigRecord );

	// Apply Application configuration changes 		
	if( validAppSettings )
	{
		EnableButtonControls( false );
		// Save Application Settings
		pMainWnd->PostMessage( AM_APPLY_APP_SETTINGS, ( WPARAM )0, ( LPARAM )( new AppConfigRecord( appConfigRecord ) ) );
	}

	return validAppSettings;
}


/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support for Peformance Mode option
///
/// ]]>
/// <summary>
/// Called when [comm type selection change].
/// </summary>
void CSPU3DeviceConfigView::OnCommTypeSelectionChange()
{
	UpdateData( TRUE );
		
	bool enableControls = ( Ethernet_COMM == m_commType );
	m_serverIPAddressCtrl.EnableWindow( enableControls );
	m_socketPortCtrl.EnableWindow( enableControls );
	m_spuFWPerformanceModeOptionCtrl.EnableWindow( enableControls );
}

/// <summary>
/// Loads the blink led button bitmap.
/// </summary>
void CSPU3DeviceConfigView::LoadBlinkLEDButtonBitmap()
{
	// Load bitmap of Blink LED button as per the Blink LED status
	HBITMAP hBitmap = ( m_blinkSPU3LEDs ? m_hBlinkActiveBitmap : m_hBlinkNotActiveBitmap );
	ASSERT( hBitmap );

	m_blinkLEDSButton.SetBitmap( hBitmap );
}

/// <summary>
/// Updates the SPU3 device settings.
/// </summary>
/// <param name="systemXMLFileLoaded">if set to <c>true</c> [system XML file loaded].</param>
/// <param name="pDeviceConfigRecord">The device configuration record.</param>
/// <returns></returns>
bool CSPU3DeviceConfigView::UpdateSPU3DeviceSettings( bool systemXMLFileLoaded, DeviceConfigRecord *pDeviceConfigRecord )
{	
	// Let's validate the SPU3 IP Address and Subnet Mask
	CString spuIPAddress = _T( "" );
	m_spu3IPAddressCtrl.GetWindowText( spuIPAddress );
	// Is it a valid IP Address?
	if( 0 == spuIPAddress.CompareNoCase( _T( "0.0.0.0" ) ) )
	{
		CString messageText = _T( "" );
		messageText.Format( _T( "Invalid SPU3 IP Address %s.\r\n\r\nPlease input a valid IP Address." ), spuIPAddress );
		AfxMessageBox( messageText, MB_ICONERROR );
		return false;
	}

	CString subnetMask = _T( "" );
	m_spu3SubnetMaskCtrl.GetWindowText( subnetMask );
	// Is it a valid Subnet mask Address?
	if( 0 == subnetMask.CompareNoCase( _T( "0.0.0.0" ) ) )
	{
		CString messageText = _T( "" );
		messageText.Format( _T( "Invalid SPU3 Subnet Mask Address %s.\r\n\r\nPlease input a valid Subnet Mask Address." ), subnetMask );
		AfxMessageBox( messageText, MB_ICONERROR );
		return false;
	}

	bool applyChanges = true;

	if( systemXMLFileLoaded )
	{
		// Get the SPU3 IPAddress from the combo box, if "Change IP Address To?" is selected
		CString spuIPAddressComboBox = _T( "" );
		m_comboChangeIPAddressToCtrl.GetLBText( m_comboChangeIPAddressToCtrl.GetCurSel(), spuIPAddressComboBox );

		if( BST_CHECKED == m_checkboxChangeIPAddressToCtrl.GetCheck() )
		{
			if( !spuIPAddressComboBox.IsEmpty() )
			{
				std::vector< BYTE > byteIPAddress;
				GetIPAddress( spuIPAddressComboBox, byteIPAddress );
				// Is a valid IP Address?
				if( IPADDRESS_SIZE == static_cast< int >( byteIPAddress.size() ) )
				{
					m_spu3IPAddressCtrl.SetAddress( byteIPAddress[ 0 ],
													byteIPAddress[ 1 ],
													byteIPAddress[ 2 ],
													byteIPAddress[ 3 ] );
				}
			}
		}
		else
		{
			if( Ethernet_COMM == m_commType )
			{
				// If the configured SPU IPAddress does NOT match with the System XML data, then warn the user about the mismatch
				if( CB_ERR == m_comboChangeIPAddressToCtrl.FindStringExact( 0, spuIPAddress ) )
				{
					CString messageText = _T( "" );
					messageText.Format( _T( "Configured SPU3 IP Address %s does NOT match with the System XML data.\r\n\r\nDo you still want to Save the SPU3 Device Settings?" ), spuIPAddress );
					applyChanges = ( IDYES == AfxMessageBox( messageText, MB_YESNO | MB_ICONQUESTION ) );
				}
			}
		}	
	}

	// Update the SPU3 parameters
	if( applyChanges )
	{
		// SPU3 IP Address
		m_spu3IPAddressCtrl.GetAddress( pDeviceConfigRecord->ip_Addr[ 0 ],
										pDeviceConfigRecord->ip_Addr[ 1 ],
										pDeviceConfigRecord->ip_Addr[ 2 ],
										pDeviceConfigRecord->ip_Addr[ 3 ] );

		// SPU3 subnet mask
		m_spu3SubnetMaskCtrl.GetAddress( pDeviceConfigRecord->ip_Mask[ 0 ],
										 pDeviceConfigRecord->ip_Mask[ 1 ],
										 pDeviceConfigRecord->ip_Mask[ 2 ],
										 pDeviceConfigRecord->ip_Mask[ 3 ] );
	}	

	return applyChanges;
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support for Peformance Mode option
///
/// ]]>
/// <summary>
/// Updates the application settings.
/// </summary>
/// <param name="systemXMLFileLoaded">if set to <c>true</c> [system XML file loaded].</param>
/// <param name="pAppConfigRecord">The application configuration record.</param>
/// <returns></returns>
bool CSPU3DeviceConfigView::UpdateApplicationSettings( bool systemXMLFileLoaded, AppConfigRecord* pAppConfigRecord )
{
	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	CMCSystem *pSystem = pMainWnd->GetSystem();
	ASSERT( pSystem );

	// Server IP Address from the UI
	CString serverIPAddressUI = _T( "" );
	m_serverIPAddressCtrl.GetWindowText( serverIPAddressUI );

	// Let's validate the Mercury Server IP Address and Socket Port
	if( !systemXMLFileLoaded && ( Ethernet_COMM == m_commType ) )
	{
		// Is it a valid IP Address?
		if( 0 == serverIPAddressUI.CompareNoCase( _T( "0.0.0.0" ) ) )
		{
			CString messageText = _T( "" );
			messageText.Format( _T( "Invalid Mercury Server IP Address %s.\r\n\r\nPlease input a valid IP Address." ), _T( "0.0.0.0" ) );
			AfxMessageBox( messageText, MB_ICONERROR );
			return false;
		}

		// Is Socket Port in valid range?
		if( ( m_socketPort < 1024 ) || ( m_socketPort > 49151 ) )
		{
			CString messageText = _T( "" );
			messageText.Format( _T( "Invalid Socket Port %d.\r\n\r\nPlease input a valid Socket Port ( Valid Range : 1024 to 49151 )." ), m_socketPort );
			AfxMessageBox( messageText, MB_ICONERROR );
			return false;
		}
	}

	bool applyChanges = true;

	// Server IP Address and socket port from the System XML 
	CString serverIPAddressSystemXML = pSystem->GetServerIPAddress();
	UINT socketPortSystemXML = pSystem->GetServerSockPort();

	// Validate Mercury App Server IP Address and Port when System XML is loaded 
	if( systemXMLFileLoaded )
	{
		if( Ethernet_COMM == m_commType )
		{
			if( !serverIPAddressSystemXML.IsEmpty() && !serverIPAddressUI.IsEmpty() )
			{
				// Compare the configured Mercury Server IP Address with the System XML data,
				// If the IP Address does NOT match, then prompt user to update the Mercury Server IP Address to match the System XML data			
				if( 0 != serverIPAddressSystemXML.CompareNoCase( serverIPAddressUI ) )
				{
					CString messageText = _T( "" );
					messageText.Format( _T( "Configured Mercury Server IP Address %s does NOT match with the Mercury System XML data.\r\n\r\nDo you want to change the Mercury Server IP Address to %s to match with the System XML data?" ),
										serverIPAddressUI, serverIPAddressSystemXML );
					bool yesMatchWithSysXMLData = ( IDYES == AfxMessageBox( messageText, MB_YESNO | MB_ICONQUESTION ) );
					if( yesMatchWithSysXMLData )
					{
						std::vector< BYTE > byteIPAddressXML;
						GetIPAddress( serverIPAddressSystemXML, byteIPAddressXML );
						// Is it a valid IP Address?
						if( 4 == static_cast< int >( byteIPAddressXML.size() ) )
						{
							m_serverIPAddressCtrl.SetAddress( byteIPAddressXML[ 0 ],
															  byteIPAddressXML[ 1 ],
															  byteIPAddressXML[ 2 ],
															  byteIPAddressXML[ 3 ] );
						}
					}
				}
			}

			// Compare the Socket Port with the System XML data
			if( m_socketPort != socketPortSystemXML )
			{
				CString messageText = _T( "" );
				messageText.Format( _T( "Configured Socket Port %d does NOT match with the Mercury System XML data.\r\n\r\nDo you want to change the Socket Port to %d to match with the System XML data?" ),
									m_socketPort, socketPortSystemXML );
				bool yesMatchWithSysXMLData = ( IDYES == AfxMessageBox( messageText, MB_YESNO | MB_ICONQUESTION ) );
				if( yesMatchWithSysXMLData )
				{
					m_socketPort = socketPortSystemXML;
				}
			}
		}
	}

	// Finally, update the Application Config Settings
	if( applyChanges )
	{
		// Mercury Server IP Address
		m_serverIPAddressCtrl.GetAddress( pAppConfigRecord->serverIPAddress[ 0 ],
										  pAppConfigRecord->serverIPAddress[ 1 ],
										  pAppConfigRecord->serverIPAddress[ 2 ],
										  pAppConfigRecord->serverIPAddress[ 3 ] );

		// Mercury Server socket port
		pAppConfigRecord->serverSockPort = m_socketPort;
		pAppConfigRecord->commType = m_commType;
		pAppConfigRecord->portOrientation = m_portOrientation;		

		// SPU3 FW Performance Mode option
		pAppConfigRecord->spuFWPerformanceModeOption = m_spuFWPerformanceModeOption;
	}

	return applyChanges;
}

/// <summary>
/// Called when [bn clicked button blink LEDS].
/// </summary>
void CSPU3DeviceConfigView::OnBnClickedButtonBlinkLEDS()
{
	UpdateData( TRUE );

	// Load bitmap of Blink LED button as per the Blink LED status
	LoadBlinkLEDButtonBitmap();
	
	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );
	
	DeviceConfigRecord deviceConfigRecord{ };
	pMainWnd->GetDeviceConfigRecordOfSelectedItem( deviceConfigRecord );

	DeviceOperationRecord *pDOR = new DeviceOperationRecord();
	pDOR->operationData = ( m_blinkSPU3LEDs ? 1 : 0 );

	DeviceRuntimeStatus deviceRuntimeStatus{ };		
	deviceRuntimeStatus.m_blinkLEDs = ( m_blinkSPU3LEDs ? true : false );
	// Update the Device runtime status
	pMainWnd->SetDeviceRuntimeStatusOfSelectedItem( deviceRuntimeStatus );

	int size = sizeof( deviceConfigRecord.mac_address ) / sizeof( deviceConfigRecord.mac_address[ 0 ] );
	std::copy( deviceConfigRecord.mac_address, ( deviceConfigRecord.mac_address + size ), pDOR->spuMACAddress );
	
	pMainWnd->PostMessage( AM_BLINK_SPU3_LEDS, 0, ( LPARAM )pDOR );

	// Display the current operation message on to the caption bar
	CString strMessage = _T( "" );
	strMessage.Format( _T( "%s blinking LEDs message sent to the SPU3 Device @ IP Address = %s" ),
		( m_blinkSPU3LEDs ? _T( "Start" ) : _T( "Stop" ) ),
					   pMainWnd->ToIPAddress( deviceConfigRecord.ip_Addr ) );

	pMainWnd->PostMessage( AM_DISPLAY_CAPTION_BAR_MSG, ( WPARAM )0, ( LPARAM )( new CString( strMessage ) ) );
	pMainWnd->PostMessage( AM_LOG_MESSAGE, ( WPARAM )0, ( LPARAM )( new CString( strMessage ) ) );
}

/// <summary>
/// Called when [bn clicked button reboot].
/// </summary>
void CSPU3DeviceConfigView::OnBnClickedButtonReboot()
{
	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	DeviceConfigRecord deviceConfigRecord{ };
	pMainWnd->GetDeviceConfigRecordOfSelectedItem( deviceConfigRecord );

	DeviceOperationRecord *pDOR = new DeviceOperationRecord();
	pDOR->operationData = 1; // Always TRUE for Reboot operation

	int size = sizeof( deviceConfigRecord.mac_address ) / sizeof( deviceConfigRecord.mac_address[ 0 ] );
	std::copy( deviceConfigRecord.mac_address, ( deviceConfigRecord.mac_address + size ), pDOR->spuMACAddress );

	pMainWnd->PostMessage( AM_REBOOT_SPU3_DEVICE, 0, ( LPARAM )pDOR );

	EnableButtonControls( false );

	// Display the current operation message on to the caption bar
	CString strMessage = _T( "" );
	strMessage.Format( _T( "Reboot message sent to the SPU3 Device @ IP Address = %s" ), pMainWnd->ToIPAddress( deviceConfigRecord.ip_Addr ) );

	pMainWnd->PostMessage( AM_DISPLAY_CAPTION_BAR_MSG, ( WPARAM )0, ( LPARAM )( new CString( strMessage ) ) );
	pMainWnd->PostMessage( AM_LOG_MESSAGE, ( WPARAM )0, ( LPARAM )( new CString( strMessage ) ) );

	// Start timer to update the Device and Application settings
	pMainWnd->PostMessage( AM_START_TIMER_MSG, ( WPARAM )AM_QUERY_SPU3_DEVICES, ( LPARAM )5000 );
}

/// <summary>
/// Gets the ip address.
/// </summary>
/// <param name="strIPAddress">The string ip address.</param>
/// <param name="byteIPAddress">The byte ip address.</param>
void CSPU3DeviceConfigView::GetIPAddress( const CString& strIPAddress, std::vector< BYTE >& byteIPAddress )
{
	CString strToken = strIPAddress;
	int curPos = 0;		
	while( !strToken.IsEmpty() )
	{
		strToken = strIPAddress.Tokenize( _T( "." ), curPos );
		if( !strToken.IsEmpty() )
			byteIPAddress.push_back( _ttoi( strToken ) );
	}
}

/// <summary>
/// translate message.
/// </summary>
/// <param name="pMsg">The MSG.</param>
/// <returns></returns>
BOOL CSPU3DeviceConfigView::PreTranslateMessage( MSG* pMsg )
{
	if( ( pMsg->message == WM_LBUTTONDOWN )||
		( pMsg->message == WM_LBUTTONUP ) ||
		( pMsg->message == WM_MOUSEMOVE ) )
	{
		m_toolTipControl.RelayEvent( pMsg );
	}		

	return CDialogEx::PreTranslateMessage( pMsg );
}

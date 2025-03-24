// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="SPU3AllDevicesView.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

// SPU3AllDevicesView.cpp : implementation file
//

#include "stdafx.h"
#include "SPU3AutoIPConfigTool.h"
#include "SPU3AllDevicesView.h"
#include "MainFrm.h"
#include "Defines.h"
#include "MCSystem.h"
#include "MCPress.h"
#include <fstream>
#include <sstream>
#include <iomanip>

// List control column headers
const CString MACAddressColHeader = _T( "MAC Address" );
const CString SPUIPAddrColHeader = _T( "SPU3 IP Address" );
const CString SPUFWVersionColHeader = _T( "FW Version" );
const CString SPUSubnetMaskColHeader = _T( "Subnet Mask" );
const CString SPUCommTypeColHeader = _T( "COMM Type" );
const CString SPUPortOrientationColHeader = _T( "Port Orientation" );
const CString MercuryServerIPAddrColHeader = _T( "Mercury Server IP Address" );
const CString SocketPortColHeader = _T( "Server Port" );

const int MACAddressColIndex = 0;
const int SPUIPAddrColIndex = 1;
const int SPUSubnetMaskColIndex = 2;
const int SPUFWVersionColIndex = 3;
const int SPUCommTypeColIndex = 4;
const int SPUPortOrientationColIndex = 5;
const int MercuryServerIPAddrColIndex = 6;
const int SocketPortColIndex = 7;

// CSPU3AllDevicesView

IMPLEMENT_DYNCREATE(CSPU3AllDevicesView, CDialogEx )

/// <summary>
/// Initializes a new instance of the <see cref="CSPU3AllDevicesView"/> class.
/// </summary>
/// <param name="pParent">The p parent.</param>
CSPU3AllDevicesView::CSPU3AllDevicesView( CWnd* pParent /*=NULL*/ )
	: CDialogEx( CSPU3AllDevicesView::IDD, pParent )
{
	Create( CSPU3AllDevicesView::IDD, pParent );
}

/// <summary>
/// Finalizes an instance of the <see cref="CSPU3AllDevicesView"/> class.
/// </summary>
CSPU3AllDevicesView::~CSPU3AllDevicesView()
{ }

/// <summary>
/// Does the data exchange.
/// </summary>
/// <param name="pDX">The p dx.</param>
void CSPU3AllDevicesView::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_LIST_SPU3_DEVICES, m_allDevicesConfigListCtrl );
	DDX_Control( pDX, IDC_REPORT_FOLDER_BROWSE, m_folderBrowseCtrl );
	DDX_Control( pDX, IDC_BTN_CREATE_HIST_FILE, m_createHistFileBtnCtrl );
	DDX_Control( pDX, IDC_BTN_GENERATE_REPORT, m_generateReportBtnCtrl );
}

BEGIN_MESSAGE_MAP( CSPU3AllDevicesView, CDialogEx )
	ON_MESSAGE( AM_POPULATE_ALL_DEV_REPORT, &CSPU3AllDevicesView::OnPopulateAllDevicesReport )
	ON_BN_CLICKED( IDC_BTN_GENERATE_REPORT, &CSPU3AllDevicesView::OnGenerateReport )
	ON_BN_CLICKED( IDC_BTN_CREATE_HIST_FILE, &CSPU3AllDevicesView::OnCreateHistoryFile )
END_MESSAGE_MAP()

// CSPU3AllDevicesView message handlers

/// <summary>
/// Called when [initialize dialog].
/// </summary>
/// <returns></returns>
BOOL CSPU3AllDevicesView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	LoadInitialSettings();

	return TRUE;  // return TRUE unless you set the focus to a control				  
}

/// <summary>
/// Loads the initial settings.
/// </summary>
void CSPU3AllDevicesView::LoadInitialSettings()
{
	CreateToolTipControl();
	AddHeaderToListControl();
	SetReportFolderPath();
}

/// <summary>
/// Creates the tool tip control.
/// </summary>
void CSPU3AllDevicesView::CreateToolTipControl()
{
	if( m_toolTipControl.Create( this ) )
	{
		m_toolTipControl.AddTool( &m_createHistFileBtnCtrl, _T( "Creates a History File." ) ); // Which includes MAC Address and IP Address of each SPU3 Device found on the Network.
		m_toolTipControl.AddTool( &m_generateReportBtnCtrl, _T( "Generates a Report File." ) ); // Which includes Device Settings and Mercury Server Settings of each SPU3 Found on the Network.
	}
}

/// <summary>
/// Adds the header to list control.
/// </summary>
void CSPU3AllDevicesView::AddHeaderToListControl()
{
	m_allDevicesConfigListCtrl.SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES );

	CRect rect;
	m_allDevicesConfigListCtrl.GetClientRect( &rect );

	int nColInterval = rect.Width() / 7;

	m_allDevicesConfigListCtrl.InsertColumn( MACAddressColIndex, MACAddressColHeader, LVCFMT_LEFT, ( int )( nColInterval * 1.6 ) );
	m_allDevicesConfigListCtrl.InsertColumn( SPUIPAddrColIndex, SPUIPAddrColHeader, LVCFMT_LEFT, ( int )( nColInterval * 1.2 ) );
	m_allDevicesConfigListCtrl.InsertColumn( SPUSubnetMaskColIndex, SPUSubnetMaskColHeader, LVCFMT_LEFT, ( int )( nColInterval * 1.2 ) );
	m_allDevicesConfigListCtrl.InsertColumn( SPUFWVersionColIndex, SPUFWVersionColHeader, LVCFMT_LEFT, nColInterval );
	m_allDevicesConfigListCtrl.InsertColumn( SPUCommTypeColIndex, SPUCommTypeColHeader, LVCFMT_LEFT, ( int )( nColInterval * 1.2 ) );
	m_allDevicesConfigListCtrl.InsertColumn( SPUPortOrientationColIndex, SPUPortOrientationColHeader, LVCFMT_LEFT, ( int )( nColInterval * 1.2 ) );
	m_allDevicesConfigListCtrl.InsertColumn( MercuryServerIPAddrColIndex, MercuryServerIPAddrColHeader, LVCFMT_LEFT, ( int )( nColInterval * 1.2 ) );
	m_allDevicesConfigListCtrl.InsertColumn( SocketPortColIndex, SocketPortColHeader, LVCFMT_LEFT, nColInterval );
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Corrected the Port Orientation values to match with SPU3 HTML Page values
///
/// ]]>
/// <summary>
/// Called when [populate all devices report].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CSPU3AllDevicesView::OnPopulateAllDevicesReport( WPARAM wParam, LPARAM lParam )
{
	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	std::map< CString, TreeItemRecord > allDevConfigSettings;
	pMainWnd->GetAllDevicesConfigSettings( allDevConfigSettings );

	// Clear the list 
	m_allDevicesConfigListCtrl.DeleteAllItems();

	// Add items to the list
	int rowIndex = 0;
	for( auto deviceSettingIter : allDevConfigSettings )
	{
		m_allDevicesConfigListCtrl.InsertItem( rowIndex, deviceSettingIter.first );
		m_allDevicesConfigListCtrl.SetItemText( rowIndex, SPUIPAddrColIndex, pMainWnd->ToIPAddress( deviceSettingIter.second.m_devConfigRecord.ip_Addr ) );
		m_allDevicesConfigListCtrl.SetItemText( rowIndex, SPUSubnetMaskColIndex, pMainWnd->ToIPAddress( deviceSettingIter.second.m_devConfigRecord.ip_Mask ) );

		CString spu3FWVersion = _T( "" );
		spu3FWVersion.Format( _T( "%d.%d" ),
							  deviceSettingIter.second.m_appConfigRecord.spuVersionMajor,
							  deviceSettingIter.second.m_appConfigRecord.spuVersionMinor );

		m_allDevicesConfigListCtrl.SetItemText( rowIndex, SPUFWVersionColIndex, spu3FWVersion );
		m_allDevicesConfigListCtrl.SetItemText( rowIndex, SPUCommTypeColIndex,
			( 0 == deviceSettingIter.second.m_appConfigRecord.commType ) ? _T( "Serial" ) : _T( "Ethernet" ) );
		m_allDevicesConfigListCtrl.SetItemText( rowIndex, SPUPortOrientationColIndex,
			( Right_To_Left == deviceSettingIter.second.m_appConfigRecord.portOrientation ) ? _T( "Right To Left" ) : _T( "Left To Right" ) );
		m_allDevicesConfigListCtrl.SetItemText( rowIndex, MercuryServerIPAddrColIndex, pMainWnd->ToIPAddress( deviceSettingIter.second.m_appConfigRecord.serverIPAddress ) );

		CString socketPort = _T( "" );
		socketPort.Format( _T( "%d" ), deviceSettingIter.second.m_appConfigRecord.serverSockPort );
		m_allDevicesConfigListCtrl.SetItemText( rowIndex, SocketPortColIndex, socketPort );

		rowIndex++;
	}

	return 0;
}

/// <summary>
/// Sets the report folder path.
/// </summary>
void CSPU3AllDevicesView::SetReportFolderPath()
{
	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	CString logFolderPath = theApp.m_logFolderPath;
	if( !logFolderPath.IsEmpty() )
	{
		m_folderBrowseCtrl.SetWindowText( logFolderPath );
	}
}

/// <summary>
/// Gets the name of the report file.
/// </summary>
/// <returns></returns>
CString CSPU3AllDevicesView::GetReportFileName()
{
	// get the current system time
	SYSTEMTIME systemTime;
	GetLocalTime( &systemTime );

	CString timeStamp = _T( "" );
	timeStamp.Format( _T( "%4.4d_%2.2d_%2.2d_%2.2d_%2.2d_%2.2d_%3.3d" ),
					  systemTime.wYear, systemTime.wMonth, systemTime.wDay,
					  systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds );

	CString reportFileName = _T( "" );
	reportFileName.Format( SPU3EthernetIPConfigReportFileName, timeStamp.GetString() );

	return reportFileName;
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Corrected the Port Orientation values to match with SPU3 HTML Page values
///
/// ]]>
/// <summary>
/// Called when [generate report].
/// </summary>
void CSPU3AllDevicesView::OnGenerateReport()
{
	CString reportFolderPath = _T( "" );
	m_folderBrowseCtrl.GetWindowText( reportFolderPath );

	if( reportFolderPath.IsEmpty() )
	{
		AfxMessageBox( _T( "Report Folder Path is empty. Please select a valid Folder." ), MB_ICONERROR );
		return;
	}

	// Is report folder exists?
	if( !PathFileExists( reportFolderPath ) )
	{
		CString errorMessage = _T( "" );
		errorMessage.Format( _T( "Report Folder Path = %s does NOT exists. Please select a valid Folder." ), reportFolderPath );
		AfxMessageBox( errorMessage, MB_ICONERROR );
		return;
	}

	CString reportFileName = GetReportFileName();
	CString reportFileFullPath = reportFolderPath + _T( "\\" ) + reportFileName;

	std::wofstream reportStream( reportFileFullPath.GetBuffer( reportFileFullPath.GetLength() ) );
	if( !reportStream.is_open() )
	{
		// failed to open the report file
		CString errorMessage = _T( "" );
		errorMessage.Format( _T( "Failed to open the report File = %s." ), reportFileFullPath );
		AfxMessageBox( errorMessage, MB_ICONERROR );
		return;
	}

	// get the current system time
	SYSTEMTIME systemTime;
	GetLocalTime( &systemTime );

	CString timeStamp = _T( "" );
	timeStamp.Format( _T( "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d" ),
					  systemTime.wYear, systemTime.wMonth, systemTime.wDay,
					  systemTime.wHour, systemTime.wMinute, systemTime.wSecond );

	std::wostringstream writeStream( _T( "" ) );
	// Write header
	writeStream << _T( "Date = " ) << timeStamp.GetString() << std::endl;
	// Get the App Version and build date
	writeStream << _T( "Version = " ) << theApp.m_appVersion.GetString()
		<< _T( " Build Date = " ) << theApp.m_appBuildDate.GetString() << std::endl;

	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );
	CMCSystem *pSystem = pMainWnd->GetSystem();
	ASSERT( pSystem );
	CMCPress *pPress = pSystem->GetPress();
	ASSERT( pPress );

	// Write Press Name
	writeStream << _T( "Press Name = " ) << pPress->GetPressName().GetString() << std::endl;
	// Write Site Name
	writeStream << _T( "Site Name = " ) << pSystem->GetSiteName().GetString() << std::endl;
	// Write Site Number		
	writeStream << _T( "Site Number = " ) << pSystem->GetSiteNumber() << std::endl;

	writeStream << std::endl << std::endl;

	//			SPU3	SPU3		    SPU3			SPU3			SPU3		SPU3				Mercury Server		
	// Header:	MAC		IPAddress		FW Version		Subnet mask		Comm Type	Port Orientation	IPAddress		Socket Port
	//						
	writeStream << _T( "------------------------------------------------------------------------------------------------------------------------" ) << std::endl;
	writeStream << _T( "SPU3											    Mercury Server" ) << std::endl;
	writeStream << _T( "MAC Address    	    IP Address     FW Version  Subnet mask   Comm Type	 Port Orientation   IP Address	    Server Port" ) << std::endl;
	writeStream << _T( "------------------------------------------------------------------------------------------------------------------------" ) << std::endl;

	std::map< CString, TreeItemRecord > allDevConfigSettings;
	pMainWnd->GetAllDevicesConfigSettings( allDevConfigSettings );

	try
	{
		// For each SPU, write data to report file 
		for( auto deviceSettingIter : allDevConfigSettings )
		{
			CString spuMACAddress = deviceSettingIter.first;
			CString spuIPAddress = pMainWnd->ToIPAddress( deviceSettingIter.second.m_devConfigRecord.ip_Addr );
			CString spuMaskAddress = pMainWnd->ToIPAddress( deviceSettingIter.second.m_devConfigRecord.ip_Mask );

			CString spu3FWVersion = _T( "" );
			spu3FWVersion.Format( _T( "%d.%d" ),
								  deviceSettingIter.second.m_appConfigRecord.spuVersionMajor,
								  deviceSettingIter.second.m_appConfigRecord.spuVersionMinor );

			CString commType = ( ( 0 == deviceSettingIter.second.m_appConfigRecord.commType ) ? _T( "Serial" ) : _T( "Ethernet" ) );
			CString portOrientaion = ( ( Right_To_Left == deviceSettingIter.second.m_appConfigRecord.portOrientation ) ? _T( "Right To Left" ) : _T( "Left To Right" ) );
			CString serverIPAddress = pMainWnd->ToIPAddress( deviceSettingIter.second.m_appConfigRecord.serverIPAddress );

			CString socketPort = _T( "" );
			socketPort.Format( _T( "%d" ), deviceSettingIter.second.m_appConfigRecord.serverSockPort );

			writeStream << std::left << std::setw( 20 ) << spuMACAddress.GetString()
				<< std::left << std::setw( 15 ) << spuIPAddress.GetString()
				<< std::left << std::setw( 12 ) << spu3FWVersion.GetString()
				<< std::left << std::setw( 14 ) << spuMaskAddress.GetString()
				<< std::left << std::setw( 12 ) << commType.GetString()
				<< std::left << std::setw( 19 ) << portOrientaion.GetString()
				<< std::left << std::setw( 16 ) << serverIPAddress.GetString()
				<< std::left << std::setw( 6 ) << socketPort.GetString()
				<< std::endl;
		}

		writeStream << _T( "------------------------------------------------------------------------------------------------------------------------" ) << std::endl;
		reportStream << writeStream.str().c_str();

		// close the report file
		reportStream.close();
	}
	catch( CException* ex )
	{
		ex->ReportError();
		ex->Delete();
	}

	// Display the report location to the user
	CString reportResultMessage = _T( "" );
	reportResultMessage.Format( _T( "Report File Created Successfully.\r\n\r\n%s" ), reportFileFullPath );
	AfxMessageBox( reportResultMessage, MB_ICONINFORMATION );
}

/// <summary>
/// Determines whether this instance [can create history file].
/// </summary>
/// <returns>
///   <c>true</c> if this instance [can create history file]; otherwise, <c>false</c>.
/// </returns>
bool CSPU3AllDevicesView::CanCreateHistoryFile()
{
	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	// Is the System XML file loaded?
	if( !pMainWnd->IsSystemXMLFileLoaded() )
	{
		// If the System XML file is NOT loaded, then allow the user to create the History file.
		return true;
	}

	CMCSystem *pSystem = pMainWnd->GetSystem();
	ASSERT( pSystem );

	CMCPress *pPress = pSystem->GetPress();
	ASSERT( pPress );

	// Get the list of SPUs from the System XML file
	std::vector< CString > spuIPAddressXMLList;
	pPress->GetSPUIPAddressList( spuIPAddressXMLList );

	// Let's find each SPU from the System XML file on the Network, 
	// if all of the SPUs are found on the Network, then it's time 
	// to generate the System XML file
	std::vector< CString > spuIPAddressNetworkList;
	pMainWnd->GetLiveSPUIPAddressList( spuIPAddressNetworkList );

	bool createHistoryFile = true;
	for( CString spuIPAddress : spuIPAddressXMLList )
	{
		if( std::find( spuIPAddressNetworkList.begin(), spuIPAddressNetworkList.end(), spuIPAddress ) == spuIPAddressNetworkList.end() )
		{
			createHistoryFile = false;
			break;
		}
	}

	return createHistoryFile;
}

/// <summary>
/// Called when [create history file].
/// </summary>
void CSPU3AllDevicesView::OnCreateHistoryFile()
{
	// If the System XML file is loaded, then validate the list of SPUs on the Network with 
	// the System XML file. If all of the SPUs from the System XML file are found on the Network then 
	// it is time to generate the history file, i.e. "SPU3EthernetIPConfig.history". 

	// The history file would serve as a base config file to detect any new SPU3 devices on the Network, 
	//  thus helping the future additions/replacements/changes of the SPU3 devices to the Network.
	bool readyToCreateHistoryFile = CanCreateHistoryFile();
	if( !readyToCreateHistoryFile )
	{
		CString messageText = _T( "One or more SPUs from the System XML file are NOT found on the Network.\r\n\r\nDo you still want to create the History File?" );
		if( IDNO == AfxMessageBox( messageText, MB_YESNO | MB_ICONQUESTION ) )
		{
			return;
		}
	}

	CMainFrame *pMainWnd = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pMainWnd );

	// Create the History File
	pMainWnd->PostMessage( AM_CREATE_HISTORY_FILE, 0, 0 );
}

/// <summary>
/// translate message.
/// </summary>
/// <param name="pMsg">The MSG.</param>
/// <returns></returns>
BOOL CSPU3AllDevicesView::PreTranslateMessage( MSG* pMsg )
{
	if( ( pMsg->message == WM_LBUTTONDOWN ) ||
		( pMsg->message == WM_LBUTTONUP ) ||
		( pMsg->message == WM_MOUSEMOVE ) )
	{
		m_toolTipControl.RelayEvent( pMsg );
	}

	return CDialogEx::PreTranslateMessage( pMsg );
}

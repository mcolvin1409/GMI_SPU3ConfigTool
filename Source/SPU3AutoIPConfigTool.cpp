// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="SPU3AutoIPConfigTool.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

// SPU3AutoIPConfigTool.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "SPU3AutoIPConfigTool.h"
#include "MainFrm.h"

#include "SPU3AutoIPConfigToolDoc.h"
#include "SPU3AutoIPConfigToolView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Processor for the command line (sets app members)
class CCommandProcessor : public CCommandLineInfo
{
private:
	/// <summary>
	/// The m p application
	/// </summary>
	CSPU3AutoIPConfigToolApp* m_pApp;

public:
	/// <summary>
	/// Initializes a new instance of the <see cref="CCommandProcessor"/> class.
	/// </summary>
	/// <param name="pApp">The p application.</param>
	CCommandProcessor( CSPU3AutoIPConfigToolApp* pApp )
		: m_pApp( pApp )
	{ }

	/// <summary>
	/// Parses the parameter.
	/// </summary>
	/// <param name="lpszParam">The LPSZ parameter.</param>
	/// <param name="bFlag">The b flag.</param>
	/// <param name="bLast">The b last.</param>
	virtual void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast )
	{
		// Attempt to parse the command line options (if any)
		if( !m_pApp->ParseCmdOptions( lpszParam ) )
		{
			CCommandLineInfo::ParseParam( lpszParam, bFlag, bLast );
		}
	}
};

// CSPU3AutoIPConfigToolApp

BEGIN_MESSAGE_MAP( CSPU3AutoIPConfigToolApp, CWinAppEx )
END_MESSAGE_MAP()

// CSPU3AutoIPConfigToolApp construction

/// <summary>
/// Initializes a new instance of the <see cref="CSPU3AutoIPConfigToolApp"/> class.
/// </summary>
CSPU3AutoIPConfigToolApp::CSPU3AutoIPConfigToolApp()
{
	m_bHiColorIcons = TRUE;
	m_bSaveState = FALSE;
	m_bForceDockStateLoad = FALSE;
	m_bLoadWindowPlacement = FALSE;

	m_appBuildDate = _T( "" );
	m_appVersion = _T( "" );
	m_appExeFilePath = _T( "" );
	m_logFolderPath = _T( "" );

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID( _T( "SPU3AutoIPConfigTool.AppID.NoVersion" ) );

	// Place all significant initialization in InitInstance
}

// The one and only CSPU3AutoIPConfigToolApp object
/// <summary>
/// The application
/// </summary>
CSPU3AutoIPConfigToolApp theApp;


// CSPU3AutoIPConfigToolApp initialization
/// <summary>
/// Initializes the instance.
/// </summary>
/// <returns></returns>
BOOL CSPU3AutoIPConfigToolApp::InitInstance()
{
	CWinAppEx::InitInstance();

	// Only single instance allowed
	if( !FirstInstance() )
	{
		TRACE( "An instance of this Application running already\n" );
		return FALSE;
	}

	// Register the new class and exit if registration fails
	if( !RegisterWindowClass() )
	{
		TRACE( "Class Registration Failed\n" );
		return FALSE;
	}

	if( !AfxSocketInit() )
	{
		AfxMessageBox( IDP_SOCKETS_INIT_FAILED, MB_ICONERROR );
		return FALSE;
	}

	HRESULT hRes = CoInitializeEx( NULL, COINIT_MULTITHREADED );
	if( FAILED( hRes ) )
	{
		AfxMessageBox( _T( "Failed to Initialize COM Library." ), MB_ICONERROR );
		return FALSE;
	}

	AfxEnableControlContainer();
	EnableTaskbarInteraction( FALSE );

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey( _T( "Advanced Vision Technology, Ltd." ) );

	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();

	// get application version
	m_appVersion.LoadString( IDS_APP_VERSION_NUM );

	SetAppBuildDate();
	SetExecutableFolderPath();

	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams( AFX_TOOLTIP_TYPE_ALL,
												  RUNTIME_CLASS( CMFCToolTipCtrl ), &ttParams );

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS( CSPU3AutoIPConfigToolDoc ),
		RUNTIME_CLASS( CMainFrame ),       // main SDI frame window
		RUNTIME_CLASS( CSPU3AutoIPConfigToolView ) );
	if( !pDocTemplate )
		return FALSE;
	AddDocTemplate( pDocTemplate );

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine( cmdInfo );

	// Process command line parameters
	if( !CString( m_lpCmdLine ).Trim().IsEmpty() )
	{
		CCommandProcessor process( this );
		ParseCommandLine( process );

		// Set the File Name to empty string and shell cmd to FileNew
		//	if the cmd line does NOT start with '/' character, 
		//	then the entire command line is considered as a filename by ProcessShellCommand(cmdInfo)
		cmdInfo.m_nShellCommand = cmdInfo.FileNew;
		cmdInfo.m_strFileName = _T( "" );
	}

	// Set the Log folder path as Application executable path
	//	if there is NO command line switch "/LOGPATH" with Log folder path
	//  or if the log folder path is empty for some reason
	if( m_logFolderPath.IsEmpty() )
		m_logFolderPath = m_appExeFilePath;

	// Ensure Log Folder exists? If the folder does NOT exists, create it.
	CreateLogsFolder();

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if( !ProcessShellCommand( cmdInfo ) )
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow( SW_SHOW );
	m_pMainWnd->CenterWindow();
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

/// <summary>
/// Registers the window class.
/// </summary>
/// <returns></returns>
BOOL CSPU3AutoIPConfigToolApp::RegisterWindowClass()
{
	// Register your unique class name that you wish to use
	WNDCLASS wndcls;
	memset( &wndcls, 0, sizeof( WNDCLASS ) );

	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hInstance = AfxGetInstanceHandle();
	wndcls.hIcon = LoadIcon( IDR_MAINFRAME );
	wndcls.hCursor = LoadCursor( IDC_ARROW );

	// Specify your own class name for using FindWindow later
	wndcls.lpszClassName = AppMainWindowClassName;

	return AfxRegisterClass( &wndcls );
}

/// <summary>
/// Firsts the instance.
/// </summary>
/// <returns></returns>
BOOL CSPU3AutoIPConfigToolApp::FirstInstance()
{
	try
	{
		m_hSingleInstanceMutex = CreateMutex( NULL, TRUE, _T( "16A12D4C-E680-4550-AB57-48F1C5F027B5" ) );

		if( ( NULL == m_hSingleInstanceMutex ) ||
			( ERROR_ALREADY_EXISTS == GetLastError() ) )
		{
			CWnd *pWndPrev = NULL, *pWndChild = NULL;
			if( pWndPrev = CWnd::FindWindow( AppMainWindowClassName, NULL ) )
			{
				if( ( NULL != pWndPrev ) && ::IsWindow( pWndPrev->m_hWnd ) )
				{
					// If iconic, restore the main window
					if( ::IsWindow( pWndPrev->m_hWnd ) && ( pWndPrev->IsIconic() ) )
						pWndPrev->ShowWindow( SW_RESTORE );

					pWndChild = pWndPrev->GetLastActivePopup();
					// Bring the main window or its popup to
					// the foreground
					if( ( NULL != pWndChild ) && ::IsWindow( pWndChild->m_hWnd ) )
						pWndChild->SetForegroundWindow();

					return FALSE;
				}
			}

			return FALSE; // Exit the app
		}
	}
	catch( ... )
	{
		return FALSE;
	}

	return TRUE;
}

/// <summary>
/// Sets the executable folder path.
/// </summary>
void CSPU3AutoIPConfigToolApp::SetExecutableFolderPath()
{
	// Get the module path ( where executable is running from )
	TCHAR exePath[ MAX_PATH ];
	if( 0 == GetModuleFileName( NULL, exePath, MAX_PATH ) )
	{
		AfxMessageBox( _T( "Failed to get the Executable Folder Path." ) );
	}

	PathRemoveFileSpec( exePath );
	m_appExeFilePath = exePath;
}

/// <summary>
/// Creates the logs folder.
/// </summary>
void CSPU3AutoIPConfigToolApp::CreateLogsFolder()
{
	// Check whether the Log Folder exists? 
	if( PathFileExists( m_logFolderPath ) )
	{
		return;
	}

	// Create Log Folder, if it does NOT exists.
	if( 0 == CreateDirectory( m_logFolderPath, NULL ) )
	{
		CString messageText = _T( "" );
		messageText.Format( _T( "Failed to create the Log Folder = %s" ), m_logFolderPath );
		AfxMessageBox( messageText, MB_OK );
	}
}

/// <summary>
/// Parses the command options.
/// </summary>
/// <param name="lpszParam">The LPSZ parameter.</param>
/// <returns></returns>
BOOL CSPU3AutoIPConfigToolApp::ParseCmdOptions( LPCTSTR lpszParam )
{
	CString strParam = lpszParam;
	strParam.MakeUpper().Trim();

	int curPos = 0;
	if( strParam.Find( _T( "LOGPATH" ), 0 ) == 0 )
	{
		// Extract only the Log File Path
		CString logFilePath = strParam.Right( strParam.GetLength() - ( strParam.Find( _T( '=' ) ) + 1 ) );
		if( !logFilePath.IsEmpty() )
		{
			m_logFolderPath = logFilePath;
		}
	}

	return FALSE;
}

/// <summary>
/// Sets the application build date.
/// </summary>
void CSPU3AutoIPConfigToolApp::SetAppBuildDate()
{
	TCHAR exePath[ MAX_PATH ];
	if( 0 != GetModuleFileName( NULL, exePath, MAX_PATH ) )
	{
		CFileStatus rStatus;
		CFile::GetStatus( exePath, rStatus );
		CTime tm = rStatus.m_mtime; // last modification date/time of file
		m_appBuildDate = tm.Format( "%B %d, %Y, %H:%M:%S" );
	}
}

/// <summary>
/// Exits the instance.
/// </summary>
/// <returns></returns>
int CSPU3AutoIPConfigToolApp::ExitInstance()
{
	CoUninitialize();

	::UnregisterClass( AppMainWindowClassName, AfxGetInstanceHandle() );

	return CWinAppEx::ExitInstance();
}
// CSPU3AutoIPConfigToolApp message handlers

// CSPU3AutoIPConfigToolApp customization load/save methods

/// <summary>
/// Pres the state of the load.
/// </summary>
void CSPU3AutoIPConfigToolApp::PreLoadState()
{ }

/// <summary>
/// Loads the state of the custom.
/// </summary>
void CSPU3AutoIPConfigToolApp::LoadCustomState()
{ }

/// <summary>
/// Saves the state of the custom.
/// </summary>
void CSPU3AutoIPConfigToolApp::SaveCustomState()
{ }

// CSPU3AutoIPConfigToolApp message handlers




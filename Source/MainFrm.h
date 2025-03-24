
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MainFrm.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include "DeviceTreeView.h"
#include "OutputWnd.h"
#include "DockableDialogBarPane.h"
#include "MCSystem.h"
#include "LogUtility.h"
#include "Defines.h"
#include <memory>
#include <functional>
#include <vector>
#include <map>

class CMCSystem;
class NetBurnSocket;
struct TreeItemRecord;
struct DeviceConfigRecord;
struct AppConfigRecord;

enum eSPU3DeviceScanStatus
{
	eSPU3DEV_DUPLICATE_IPADDRESS = 1,
	eSPU3DEV_FOUND_IN_SYSTEM_XML = 2,
	eSPU3DEV_FOUND_IN_HISTORY_FILE = 4,
	eSPU3DEV_NOT_FOUND_IN_SYSTEM_XML = 8,
	eSPU3DEV_NOT_FOUND_IN_HISTORY_FILE = 16
};

const CString AppMainWindowClassName = _T( "SPU3EthernetAutoConfigTool" );

class CMainFrame : public CFrameWndEx
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

public:
	CMCSystem* GetSystem();
	void GetDeviceConfigRecordOfSelectedItem( DeviceConfigRecord& deviceConfigRecord );
	void GetAppConfigRecordOfSelectedItem( AppConfigRecord& appConfigRecord );
	void GetDeviceRuntimeStatusOfSelectedItem( DeviceRuntimeStatus& deviceRuntimeStatus );
	void SetDeviceRuntimeStatusOfSelectedItem( DeviceRuntimeStatus& deviceRuntimeStatus );
	bool IsSystemXMLFileLoaded();
	void GetMACAddressAndIPAddressOfTreeItem( HTREEITEM hItem, CString& MACAddress, CString& IPAddress );
	void GetAllDevicesConfigSettings( std::map< CString, TreeItemRecord >& allDevSettings );
	CString ToIPAddress( BYTE IPAddress[ IPADDRESS_SIZE ] );
	void GetLiveSPUIPAddressList( std::vector< CString >& spuIPAddressList );	

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members		
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;	
	CDeviceTreeView   m_wndTreeView;
	COutputWnd        m_wndOutput;		
	CMFCCaptionBar    m_wndCaptionBar;
	CDockableDialogBarPane m_dialogBarPane;

// Attributes
private:
	bool m_bSystemXMLFileLoaded;
	bool m_bHistoryFileLoaded;
	CLogUtility m_logFile;
	std::unique_ptr< CMCSystem > m_pSystem;
	std::vector< std::unique_ptr< NetBurnSocket > > m_deviceSocketList;
	std::vector< std::unique_ptr< NetBurnSocket > > m_applicationSocketList;
	std::map< BYTE, std::function< void( int messageLength, LPBYTE messageData ) > > m_messageHandlerMap;
	std::map< CString, TreeItemRecord > m_treeItemMap;	
	std::map< UINT, UINT > m_messageToTimerIdMap;
	std::map< CString, CString > m_spuHistoryFileMap;
	std::map< eSPU3DeviceScanStatus, CString > m_deviceStatusToDescriptionMap;
	std::vector< sockaddr_in > m_sockAddressInList;
	
// Operations
private:
	BOOL CreateDockingWindows();
	void SetDockingWindowIcons( BOOL bHiColorIcons );
	BOOL CreateCaptionBar();
	BOOL CreateBrowsePane();
	void DisplayNotification( const CString& message );
	void CreateDeviceSocket();
	void CreateApplicationSocket();
	void LoadMessageHandlers();
	void AddDevConfigToDeviceTreeView( DeviceConfigRecord& deviceConfigRecord );
	void HandleDeviceConfigReadMessage( int messageLength, LPBYTE messageData );
	void HandleApplicationConfigMessage( int messageLength, LPBYTE messageData );	
	void HandleBroadCastMessage( int messageLength, LPBYTE messageData );
	void CreateLogFile();
	void LogMessage( const CString& logMessage );
	void WriteApplicationInfo();
	CString GetDeviceStatusDescription( DWORD treeItemData );
	void CreateDeviceStatusMap();
	CString GetSelectedDeviceStatus();
	void CheckAndCreateHistoryFile();
	void ResetFileReadOnlyAttribute( const CString& historyFileFullPath );
	CString GetStatusbarText_AllDevicesView();
	void AddAboutBoxMenuItemToSystemMenu();
	void GetIPAddressesOfAllNetworkInterfaces();
	
// Generated message map functions
private:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);	
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnViewCaptionBar();
	afx_msg void OnUpdateViewCaptionBar(CCmdUI* pCmdUI);		
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg LRESULT OnLoadSystemXMLFile( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnLoadHistoryFile( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnMessageReceived( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnQuerySPU3Devices( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDeviceSelectionChange( WPARAM wParam, LPARAM lParam );	
	afx_msg LRESULT OnApplyDeviceConfigSettings( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnApplyAppConfigSettings( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSelectFirstDevice( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnStartTimerMessage( WPARAM wParam, LPARAM lParam );		
	afx_msg LRESULT OnBlinkSPU3LEDs( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnRebootSPU3Device( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDisplayAllDevicesView( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDisplayNoDevicesView( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDisplayCaptionBarMessage( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnClearCaptionBarMessage( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnScanDeviceTree( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnLogMessage( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUpdateStatusbarText( WPARAM wParam, LPARAM lParam );	
	afx_msg LRESULT OnCreateHistoryFile( WPARAM wParam, LPARAM lParam );
	afx_msg void OnClose();
	afx_msg void OnSysCommand( UINT nID, LPARAM lParam );

	DECLARE_MESSAGE_MAP()	
};



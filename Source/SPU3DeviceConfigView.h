
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="SPU3DeviceConfigView.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

// CSPU3DeviceConfigView.h : interface of the CSPU3DeviceConfigView class
//
#pragma once

#include "afxwin.h"
#include "afxcmn.h"

#include <vector>
#include "afxbutton.h"
using namespace std;

struct DeviceConfigRecord;
struct AppConfigRecord;

class CSPU3DeviceConfigView : public CDialogEx
{
public: // create from serialization only
	CSPU3DeviceConfigView( CWnd* pParent =NULL );
	virtual ~CSPU3DeviceConfigView();

	DECLARE_DYNCREATE(CSPU3DeviceConfigView)

	enum { IDD = IDD_SPU3_DEVICE_FORM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog(); // called first time after construct

private:
	void LoadInitialSettings();
	void ShowControls( bool bShow );
	void EnableButtonControls( bool bEnable );
	void AddHeaderToListControl();
	void PopulateInkersList( const CString& spuIPAddress );
	void PopulateSPU3IPAddressesList();	
	bool UpdateSPU3DeviceSettings( bool systemXMLFileLoaded, DeviceConfigRecord *pDeviceConfigRecord );
	bool UpdateApplicationSettings( bool systemXMLFileLoaded, AppConfigRecord* pAppConfigRecord );
	void GetIPAddress( const CString& strIPAddress, std::vector< BYTE >& byteIPAddress );
	void PopulateSPU3Name( const CString& spuIPAddress );
	void CreateToolTips();
	void LoadBlinkLEDButtonBitmap();
	bool ApplyDeviceSettings();
	bool ApplyApplicationSettings();
	void ProcessQueuedMessages();
	
// Generated message map functions
private:
	afx_msg void OnSPUSelectionChanged();
	afx_msg void OnBnClickedButtonApplyChanges();
	afx_msg void OnBnClickedButtonBlinkLEDS();
	afx_msg void OnBnClickedButtonReboot();
	afx_msg void OnCommTypeSelectionChange();
	afx_msg LRESULT OnPopulateDeviceSettings( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUpdateSystemSPU3IPAddresses( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()

private:
	UINT m_socketPort;
	int m_portOrientation;
	enum CommType { Serial_COMM, Ethernet_COMM };
	int m_commType;	
	CString m_strSPU3FWVersion;
	CIPAddressCtrl m_serverIPAddressCtrl;
	CIPAddressCtrl m_spu3IPAddressCtrl;
	CIPAddressCtrl m_spu3SubnetMaskCtrl;
	CComboBox m_comboChangeIPAddressToCtrl;
	CButton m_checkboxChangeIPAddressToCtrl;	
	CListCtrl m_portAndInkerNameListCtrl;
	CEdit m_socketPortCtrl;
	CComboBox m_commTypeCtrl;
	CComboBox m_portOrientationCtrl;
	CStatic m_SPU3DeviceSettingsGroupBox;
	CStatic m_MercuryServerSettingsGroupBox;
	CMFCButton m_applyButton;
	CButton m_blinkLEDSButton;
	CMFCButton m_rebootSPU3Button;
	CStatic m_SPU3NameFromSystemXML;
	BOOL m_blinkSPU3LEDs;
	HBITMAP m_hBlinkActiveBitmap;
	HBITMAP m_hBlinkNotActiveBitmap;	
	CToolTipCtrl m_toolTipControl;
public:
	virtual BOOL PreTranslateMessage( MSG* pMsg );
private:
	CButton m_spuFWPerformanceModeOptionCtrl;	
	int m_spuFWPerformanceModeOption;
};



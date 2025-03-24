
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="SPU3AutoIPConfigToolView.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include "afxwin.h"
#include <memory>

// CSPU3AutoIPConfigToolView form view

class CSPU3DeviceConfigView;
class CSPU3AllDevicesView;

class CSPU3AutoIPConfigToolView : public CFormView
{
	DECLARE_DYNCREATE(CSPU3AutoIPConfigToolView)

protected:
	CSPU3AutoIPConfigToolView();           // protected constructor used by dynamic creation
	virtual ~CSPU3AutoIPConfigToolView();

private:
	afx_msg LRESULT OnPopulateDeviceSettings( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUpdateSystemSPU3IPAddresses( WPARAM wParam, LPARAM lParam );	
	afx_msg LRESULT OnDisplayAllDevicesView( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDisplayNoDevicesView( WPARAM wParam, LPARAM lParam );

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_FORM };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:		
	std::unique_ptr< CSPU3DeviceConfigView > m_pDeviceConfigView;
	std::unique_ptr< CSPU3AllDevicesView > m_pAllDevicesView;

private:
	virtual void OnInitialUpdate();	

};



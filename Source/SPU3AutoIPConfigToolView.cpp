// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="SPU3AutoIPConfigToolView.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

// SPU3DeviceConfigView.cpp : implementation file
//

#include "stdafx.h"
#include "SPU3AutoIPConfigTool.h"
#include "SPU3AutoIPConfigToolView.h"
#include "SPU3AllDevicesView.h"
#include "SPU3DeviceConfigView.h"
#include "Defines.h"

// CSPU3AutoIPConfigToolView

IMPLEMENT_DYNCREATE(CSPU3AutoIPConfigToolView, CFormView)

/// <summary>
/// Initializes a new instance of the <see cref="CSPU3AutoIPConfigToolView"/> class.
/// </summary>
CSPU3AutoIPConfigToolView::CSPU3AutoIPConfigToolView()
	: CFormView(IDD_MAIN_FORM)
{
	m_pDeviceConfigView = NULL;
	m_pAllDevicesView = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="CSPU3AutoIPConfigToolView"/> class.
/// </summary>
CSPU3AutoIPConfigToolView::~CSPU3AutoIPConfigToolView()
{
}

/// <summary>
/// Does the data exchange.
/// </summary>
/// <param name="pDX">The data exchange.</param>
void CSPU3AutoIPConfigToolView::DoDataExchange( CDataExchange* pDX )
{
	CFormView::DoDataExchange( pDX );	
}

BEGIN_MESSAGE_MAP(CSPU3AutoIPConfigToolView, CFormView)
	ON_MESSAGE( AM_POPULATE_DEVICE_SETTINGS, &CSPU3AutoIPConfigToolView::OnPopulateDeviceSettings )
	ON_MESSAGE( AM_UPDATE_SYS_SPU3_IPADDR, &CSPU3AutoIPConfigToolView::OnUpdateSystemSPU3IPAddresses )
	ON_MESSAGE( AM_DISPLAY_ALL_DEVICES_VIEW, &CSPU3AutoIPConfigToolView::OnDisplayAllDevicesView )
	ON_MESSAGE( AM_DISPLAY_NO_DEVICES_VIEW, &CSPU3AutoIPConfigToolView::OnDisplayNoDevicesView )
END_MESSAGE_MAP()


// CSPU3DeviceConfigView diagnostics

#ifdef _DEBUG
/// <summary>
/// Asserts the valid.
/// </summary>
void CSPU3AutoIPConfigToolView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
/// <summary>
/// Dumps the specified dc.
/// </summary>
/// <param name="dc">The dc.</param>
void CSPU3AutoIPConfigToolView::Dump( CDumpContext& dc ) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CSPU3DeviceConfigView message handlers

/// <summary>
/// Called when [initial update].
/// </summary>
void CSPU3AutoIPConfigToolView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
		
	// Create views
	m_pAllDevicesView = std::unique_ptr< CSPU3AllDevicesView >( new CSPU3AllDevicesView( this ) );
	ASSERT( m_pAllDevicesView );

	m_pDeviceConfigView = std::unique_ptr< CSPU3DeviceConfigView >( new CSPU3DeviceConfigView( this ) );
	ASSERT( m_pDeviceConfigView );	

	m_pAllDevicesView->ShowWindow( SW_HIDE );
	m_pDeviceConfigView->ShowWindow( SW_HIDE );
}

/// <summary>
/// Called when [populate device settings].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CSPU3AutoIPConfigToolView::OnPopulateDeviceSettings( WPARAM wParam, LPARAM lParam )
{
	ASSERT( m_pDeviceConfigView );
	ASSERT( m_pAllDevicesView );
		
	m_pAllDevicesView->ShowWindow( SW_HIDE );
	m_pDeviceConfigView->ShowWindow( SW_SHOW );
	// Populate the SPU3 device settings
	m_pDeviceConfigView->PostMessage( AM_POPULATE_DEVICE_SETTINGS, wParam, lParam );

	return 0;
}

/// <summary>
/// Called when [update system SPU3 ip addresses].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CSPU3AutoIPConfigToolView::OnUpdateSystemSPU3IPAddresses( WPARAM wParam, LPARAM lParam )
{
	ASSERT( m_pDeviceConfigView );
	m_pDeviceConfigView->PostMessage( AM_UPDATE_SYS_SPU3_IPADDR, wParam, lParam );

	return 0;
}

/// <summary>
/// Called when [display all devices view].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CSPU3AutoIPConfigToolView::OnDisplayAllDevicesView( WPARAM wParam, LPARAM lParam )
{
	ASSERT( m_pDeviceConfigView );
	ASSERT( m_pAllDevicesView );

	// Show only the "All Devices View"
	m_pAllDevicesView->ShowWindow( SW_SHOW );	
	m_pDeviceConfigView->ShowWindow( SW_HIDE );

	// Populate all devices report
	m_pAllDevicesView->PostMessage( AM_POPULATE_ALL_DEV_REPORT, 0, 0 );

	return 0;
}

/// <summary>
/// Called when [display no devices view].
/// </summary>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
LRESULT CSPU3AutoIPConfigToolView::OnDisplayNoDevicesView( WPARAM wParam, LPARAM lParam )
{
	ASSERT( m_pDeviceConfigView );
	ASSERT( m_pAllDevicesView );

	// Hide both views when "No devices found"
	m_pAllDevicesView->ShowWindow( SW_HIDE );
	m_pDeviceConfigView->ShowWindow( SW_HIDE );

	return 0;
}


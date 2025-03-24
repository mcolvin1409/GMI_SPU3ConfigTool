
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="NetBurnSocket.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include "afxsock.h"
#include "Defines.h"

class NetBurnSocket : public CAsyncSocket
{
public:
	NetBurnSocket( CFrameWndEx* pMainWnd );
	virtual ~NetBurnSocket();

// Overrides
public:	
	virtual void OnReceive( int nErrorCode );

public:
	void CreateDeviceSocket( IN_ADDR ifaceIP );
	void CreateApplicationSocket( IN_ADDR ifaceIP );
	void SendRequest_AutoReadDeviceConfig();
	void SendRequest_AutoReadApplicationConfig();	
	void SendRequest_BlinkSPU3LEDs( DeviceOperationRecord *pDOR );
	void SendRequest_RebootSPU3( DeviceOperationRecord *pDOR );
	void SendRequest_WriteDeviceConfig( DeviceConfigRecord *pDeviceConfigRecord );
	void SendRequest_WriteApplicationConfig( AppConfigRecord *pAppConfigRecord );

private:
	CArray<UCHAR, UCHAR> m_DataBuffer;			// receiving buffer
	sockaddr_in m_BcastReadAddr;				// To discover NetBurner devices on the Network
	sockaddr_in m_LocalHostApp;					// To discover Devices with SPU3 Application connected to the System (using loopback address/local host)
	sockaddr_in m_LocalHostDevice;				// To discover NetBurner device connected to the System (using loopback address/local host)
	sockaddr_in m_BcastWriteAddr;				// To send Application configuration changes to SPU3 device(s)
	CFrameWndEx* m_pMainWnd;	
};



// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="NetBurnSocket.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#include "stdafx.h"
#include "NetBurnSocket.h"
#include "AppException.h"
#include "InvalidAccessException.h"
#include <memory>

using namespace std;

/// <summary>
/// The maximum socket buffer length
/// </summary>
const int MAX_SOCK_BUFFER_LENGTH = ( 2048 );	// Per NetBurner IPConfig Application
/// <summary>
/// The maximum receive message length
/// </summary>
const int MAX_RECEIVE_MSG_LENGTH = ( 1024 );	// Per NetBurner IPConfig Application

/// <summary>
/// Initializes a new instance of the <see cref="NetBurnSocket"/> class.
/// </summary>
/// <param name="pMainWnd">The main window</param>
NetBurnSocket::NetBurnSocket( CFrameWndEx* pMainWnd )
	:m_pMainWnd( pMainWnd )
{
	m_DataBuffer.SetSize( MAX_SOCK_BUFFER_LENGTH );
}

/// <summary>
/// Finalizes an instance of the <see cref="NetBurnSocket"/> class.
/// </summary>
NetBurnSocket::~NetBurnSocket()
{
	m_DataBuffer.RemoveAll();
}

/// <![CDATA[        
/// Author: Mark C
/// 
/// History:  12-Jun-19, Mark C, WI188957: Add support to process messages to/from Netburner Devices on 
///				a System with multiple Network Interfaces (NICs)
///
/// ]]>
/// <summary>
/// Creates the socket.
/// </summary>
void NetBurnSocket::CreateDeviceSocket( IN_ADDR ifaceIP )
{
	// UDP Socket to read Device configuration
	memset( &m_BcastReadAddr, 0, sizeof( m_BcastReadAddr ) );
	m_BcastReadAddr.sin_family = AF_INET;
	m_BcastReadAddr.sin_port = htons( UDP_NB_DEVICE_NETBURNERID_PORT );
	m_BcastReadAddr.sin_addr.S_un.S_addr = INADDR_BROADCAST;	
	
	// UDP Socket to read/write Device configuration using loop back address
	memset( &m_LocalHostDevice, 0, sizeof( m_LocalHostDevice ) );
	m_LocalHostDevice.sin_family = AF_INET;
	m_LocalHostDevice.sin_port = htons( UDP_NB_DEVICE_LOCAL_HOST_PORT );
	m_LocalHostDevice.sin_addr.S_un.S_addr = htonl( INADDR_LOOPBACK );

	try
	{	
		// Uncomment this piece of code when we STOP supporting Windows XP
		//char szIPAddress[ INET_ADDRSTRLEN ] = { };
		//inet_ntop( AF_INET, &ifaceIP, szIPAddress, INET_ADDRSTRLEN );

		// The API call "inet_ntoa()" is deprecated. However, this tool needs to run on Windows XP as well.
		// So, let's continue using the deprecated APIs :(
		char *szIPAddress = inet_ntoa( ifaceIP );
		CString strIPAddress( szIPAddress );

		if( Create( UDP_NB_DEVICE_NETBURNERID_PORT, SOCK_DGRAM, FD_READ, strIPAddress ) == 0 )
		{
			throw InvalidAccessException( _T( "Failed to create UDP Read Socket." ) );
		}

		BOOL bVal = 1;
		if( SetSockOpt( SO_BROADCAST, &bVal, sizeof( BOOL ) ) == 0 )
		{
			throw InvalidAccessException( _T( "Failed to set UDP Read Socket Option." ) );
		}
	}
	catch( CAppException& ex )
	{
		AfxMessageBox( ex.GetMessage().c_str(), MB_ICONERROR );
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
/// <param name="ifaceIP">The iface ip.</param>
void NetBurnSocket::CreateApplicationSocket( IN_ADDR ifaceIP )
{
	memset( &m_BcastWriteAddr, 0, sizeof( m_BcastWriteAddr ) );
	m_BcastWriteAddr.sin_family = AF_INET;
	m_BcastWriteAddr.sin_port = htons( UDP_NB_APP_BROADCAST_PORT );	
	m_BcastWriteAddr.sin_addr.S_un.S_addr = INADDR_BROADCAST;	

	// UDP Socket to read/write Application configuration using loop back address
	memset( &m_LocalHostApp, 0, sizeof( m_LocalHostApp ) );
	m_LocalHostApp.sin_family = AF_INET;
	m_LocalHostApp.sin_port = htons( UDP_NB_APP_LOCAL_HOST_PORT );
	m_LocalHostApp.sin_addr.S_un.S_addr = htonl( INADDR_LOOPBACK );

	try
	{
		// Uncomment this piece of code when we STOP supporting Windows XP
		//char szIPAddress[ INET_ADDRSTRLEN ] = { };
		//inet_ntop( AF_INET, &ifaceIP, szIPAddress, INET_ADDRSTRLEN );

		// The API call "inet_ntoa()" is deprecated. However, this tool needs to run on Windows XP as well.
		// So, let's continue using the deprecated APIs :(
		char *szIPAddress = inet_ntoa( ifaceIP );
		CString strIPAddress( szIPAddress );

		if( Create( UDP_NB_APP_BROADCAST_PORT, SOCK_DGRAM, FD_READ, strIPAddress ) == 0 )
		{
			throw InvalidAccessException( _T( "Failed to create UDP Write Socket." ) );
		}

		BOOL bVal = 1;
		if( SetSockOpt( SO_BROADCAST, &bVal, sizeof( BOOL ) ) == 0 )
		{
			throw InvalidAccessException( _T( "Failed to set UDP Write Socket Option." ) );
		}
	}
	catch( CAppException& ex )
	{
		AfxMessageBox( ex.GetMessage().c_str(), MB_ICONERROR );
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
/// Called when [receive].
/// </summary>
/// <param name="nErrorCode">The error code.</param>
void NetBurnSocket::OnReceive( int nErrorCode )
{
	try
	{
		BYTE *pBuffer = m_DataBuffer.GetData();
		ASSERT( pBuffer );

		sockaddr_in SockAddr;
		int SockAddrLen = sizeof( SockAddr );
		int messageLength = ReceiveFrom( ( void * )pBuffer, MAX_RECEIVE_MSG_LENGTH, ( SOCKADDR * )&SockAddr, &SockAddrLen );
		if( messageLength > 0 )
		{
			( ( unsigned long * )( pBuffer + messageLength ) )[ 0 ] = SockAddr.sin_addr.S_un.S_addr;
			SendTo( pBuffer, messageLength + 4, ( SOCKADDR * )&m_LocalHostDevice, sizeof( m_LocalHostDevice ) );
			SendTo( pBuffer, messageLength + 4, ( SOCKADDR * )&m_LocalHostApp, sizeof( m_LocalHostApp ) );
			ASSERT( m_pMainWnd );

			// Post the received message to the process thread
			BYTE *pByteArray = new BYTE[ messageLength ];
			if( NULL != pByteArray )
			{
				memcpy( pByteArray, pBuffer, messageLength );
				// Post the received message to the Main Window
				m_pMainWnd->PostMessage( AM_HANDLE_MESSAGE_RECEIVED, ( WPARAM )messageLength, ( LPARAM )pByteArray );
			}
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
/// Sends the request automatic read device configuration.
/// </summary>
void NetBurnSocket::SendRequest_AutoReadDeviceConfig()
{
	try
	{
		DeviceConfigStruct deviceConfig;
		deviceConfig.m_msgHeader.m_dwKeyValue = htonl( VERIFY_FROM_PC_TO_NDK );
		deviceConfig.m_msgHeader.m_bAction = NBAUTO_DEV_READ;

		SendTo( ( void * )&deviceConfig, sizeof( deviceConfig ), ( SOCKADDR * )&m_BcastReadAddr, sizeof( m_BcastReadAddr ) );
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
/// Sends the request automatic read application configuration.
/// </summary>
void NetBurnSocket::SendRequest_AutoReadApplicationConfig()
{
	try
	{	
		AppConfigStruct appConfig;
		appConfig.m_msgHeader.m_dwKeyValue = htonl( VERIFY_FROM_PC_TO_NDK );
		appConfig.m_msgHeader.m_bAction = NBAUTO_APP_CONFIG;

		SendTo( ( void * )&appConfig, sizeof( appConfig ), ( SOCKADDR * )&m_BcastWriteAddr, sizeof( m_BcastWriteAddr ) );
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
/// Sends the request write device configuration.
/// </summary>
/// <param name="pDCR">The DeviceConfigRecord.</param>
void NetBurnSocket::SendRequest_WriteDeviceConfig( DeviceConfigRecord *pDCR )
{
	try
	{
		ASSERT( pDCR );

		int messageLength = ( sizeof( DeviceConfigStruct ) - sizeof( DeviceConfigRecord ) ) + htonl( pDCR->m_Len );

		// This smart pointer will take care of releasing the heap memory
		std::unique_ptr< DeviceConfigStruct > pDeviceConfig( ( DeviceConfigStruct * ) new BYTE[ messageLength ] );
		ASSERT( pDeviceConfig );

		pDeviceConfig->m_msgHeader.m_dwKeyValue = htonl( VERIFY_FROM_PC_TO_NDK );
		pDeviceConfig->m_msgHeader.m_bAction = NBAUTO_DEV_WRITE;

		memcpy( &( pDeviceConfig->m_devConfigRecord ), pDCR, htonl( pDCR->m_Len ) );

		SendTo( ( void * )pDeviceConfig.get(), messageLength, ( SOCKADDR * )&m_BcastReadAddr, sizeof( m_BcastReadAddr ) );
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
/// Sends the request write application configuration.
/// </summary>
/// <param name="pACR">The AppConfigRecord.</param>
void NetBurnSocket::SendRequest_WriteApplicationConfig( AppConfigRecord *pACR )
{
	try
	{
		ASSERT( pACR );

		int messageLength = sizeof( AppConfigStruct );

		// This smart pointer will take care of releasing the heap memory
		std::unique_ptr< AppConfigStruct > pAppConfig( ( AppConfigStruct * ) new BYTE[ messageLength ] );
		ASSERT( pAppConfig );

		pAppConfig->m_msgHeader.m_dwKeyValue = htonl( VERIFY_FROM_PC_TO_NDK );
		pAppConfig->m_msgHeader.m_bAction = NBAUTO_APP_WRITE;
		pAppConfig->m_appConfigRecord.recordLength = messageLength;

		memcpy( &( pAppConfig->m_appConfigRecord ), pACR, sizeof( AppConfigRecord ) );

		SendTo( ( void * )pAppConfig.get(), messageLength, ( SOCKADDR * )&m_BcastWriteAddr, sizeof( m_BcastWriteAddr ) );
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
/// Sends the request blink SPU3 LEDS.
/// </summary>
/// <param name="pDOR">The DeviceOperationRecord.</param>
void NetBurnSocket::SendRequest_BlinkSPU3LEDs( DeviceOperationRecord *pDOR )
{
	try
	{
		ASSERT( pDOR );

		int messageLength = sizeof( DeviceOperationStruct );

		// This smart pointer will take care of releasing the heap memory
		std::unique_ptr< DeviceOperationStruct > pDeviceOperation( ( DeviceOperationStruct * ) new BYTE[ messageLength ] );
		ASSERT( pDeviceOperation );

		pDeviceOperation->m_msgHeader.m_dwKeyValue = htonl( VERIFY_FROM_PC_TO_NDK );
		pDeviceOperation->m_msgHeader.m_bAction = NBAUTO_BLINK_LEDS;
		pDeviceOperation->m_operationRecord.recordLength = messageLength;

		memcpy( &( pDeviceOperation->m_operationRecord ), pDOR, sizeof( DeviceOperationRecord ) );

		SendTo( ( void * )pDeviceOperation.get(), messageLength, ( SOCKADDR * )&m_BcastWriteAddr, sizeof( m_BcastWriteAddr ) );
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
/// Sends the request reboot SPU3.
/// </summary>
/// <param name="pDOR">The DeviceOperationRecord.</param>
void NetBurnSocket::SendRequest_RebootSPU3( DeviceOperationRecord *pDOR )
{
	try
	{
		ASSERT( pDOR );

		int messageLength = sizeof( DeviceOperationStruct );

		// This smart pointer will take care of releasing the heap memory
		std::unique_ptr< DeviceOperationStruct > pDeviceOperation( ( DeviceOperationStruct * ) new BYTE[ messageLength ] );
		ASSERT( pDeviceOperation );

		pDeviceOperation->m_msgHeader.m_dwKeyValue = htonl( VERIFY_FROM_PC_TO_NDK );
		pDeviceOperation->m_msgHeader.m_bAction = NBAUTO_REBOOT;
		pDeviceOperation->m_operationRecord.recordLength = messageLength;

		memcpy( &( pDeviceOperation->m_operationRecord ), pDOR, sizeof( DeviceOperationRecord ) );

		SendTo( ( void * )pDeviceOperation.get(), messageLength, ( SOCKADDR * )&m_BcastWriteAddr, sizeof( m_BcastWriteAddr ) );
	}
	catch( CException* ex )
	{
		ex->ReportError();
		ex->Delete();
	}
}

// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="Defines.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#pragma pack(1)

// Application Messages (AM_XXX)
#define AM_LOAD_SYSTEM_XML_FILE		( WM_APP + 101 )
#define AM_HANDLE_MESSAGE_RECEIVED	( WM_APP + 102 )
#define AM_QUERY_SPU3_DEVICES		( WM_APP + 103 )
#define AM_ADD_DEVICE_TO_TREE		( WM_APP + 104 )
#define AM_DEVICE_SEL_CHANGE_TREE	( WM_APP + 105 )
#define AM_POPULATE_DEVICE_SETTINGS	( WM_APP + 106 )
#define AM_UPDATE_SYS_SPU3_IPADDR	( WM_APP + 107 )
#define AM_APPLY_DEVICE_SETTINGS	( WM_APP + 108 )
#define AM_APPLY_APP_SETTINGS		( WM_APP + 109 )
#define AM_BLINK_SPU3_LEDS			( WM_APP + 110 )
#define AM_REBOOT_SPU3_DEVICE		( WM_APP + 111 )
#define AM_SELECT_FIRST_DEVICE		( WM_APP + 112 )
#define AM_START_TIMER_MSG			( WM_APP + 113 )
#define AM_DISPLAY_ALL_DEVICES_VIEW	( WM_APP + 114 )
#define AM_DISPLAY_NO_DEVICES_VIEW	( WM_APP + 115 )
#define AM_POPULATE_ALL_DEV_REPORT	( WM_APP + 116 )
#define AM_DISPLAY_CAPTION_BAR_MSG	( WM_APP + 117 )
#define AM_CLEAR_CAPTION_BAR_MSG	( WM_APP + 118 )
#define AM_LOAD_HISTORY_FILE		( WM_APP + 119 )
#define AM_SCAN_DEVICE_TREE			( WM_APP + 120 )
#define AM_UPDATE_TREE_ITEM_IMAGE	( WM_APP + 121 )
#define AM_LOG_MESSAGE				( WM_APP + 122 )
#define AM_DISPLAY_LOG_WINDOW_MSG	( WM_APP + 123 )
#define AM_UPDATE_TREE_ITEM_DATA	( WM_APP + 124 )
#define AM_UPDATE_STATUSBAR_TEXT	( WM_APP + 125 )
#define AM_CREATE_HISTORY_FILE		( WM_APP + 126 )


#define VERIFY_FROM_PC_TO_NDK		(0x4255524E) //BURN
#define VERIFY_FROM_PC_TO_NDKV2		(0x42555232) //BUR2
#define VERIFY_FROM_NDK_TO_PC		(0x4E455442) //NETB

#define UDP_NB_DEVICE_NETBURNERID_PORT	(0x4E42)	 //NB
#define UDP_NB_DEVICE_LOCAL_HOST_PORT	( UDP_NB_DEVICE_NETBURNERID_PORT + 1 ) 
#define UDP_NB_APP_BROADCAST_PORT		( UDP_NB_DEVICE_NETBURNERID_PORT + 2 )
#define UDP_NB_APP_LOCAL_HOST_PORT		( UDP_NB_APP_BROADCAST_PORT + 1 )

#define NBAUTO_DEV_READ				('R')	// Read Device Configuration
#define NBAUTO_DEV_WRITE			('W')	// Write Device Configuration

#define NBAUTO_APP_CONFIG			('C')	// Read Application Configuration
#define NBAUTO_APP_WRITE			('S')	// Write Application Configuration
#define NBAUTO_BLINK_LEDS			('L')	// Blink SPU3 Device LEDs
#define NBAUTO_REBOOT				('B')	// Reboot SPU3 Device 

const int IPADDRESS_SIZE = 4;
const int MACADDRESS_SIZE = 6;

const int TREE_ITEM_IMAGE_WHITE = 1;		// Normal
const int TREE_ITEM_IMAGE_YELLOW = 2;		// MAC Address match found and IP Address match NOT found
const int TREE_ITEM_IMAGE_GREEN = 3;		// IP Address match found in the System XML file and MAC Address match found in the History file 
const int TREE_ITEM_IMAGE_RED = 4;			// Duplicate IP Address OR MAC Address match NOT found in the History file ( regardless of IP Address match )

// SPU3 Ethernet Report File Name
const CString SPU3EthernetIPConfigReportFileName = _T( "SPU3EthernetIPConfigReport_%s.txt" );
const CString SPU3EthernetIPConfigHistoryFileName = _T( "SPU3EthernetIPConfig.history" );
const CString SPU3EthernetIPConfigLogFileName = _T( "SPU3EthernetIPConfig.log" );

enum PortOrientation
{
	Right_To_Left = 0, Left_To_Right
};

struct DeviceConfigRecord
{
	DWORD m_Len;							// length of record
	BYTE ip_Addr[ IPADDRESS_SIZE ];			// The device IP Address
	BYTE ip_Mask[ IPADDRESS_SIZE ];			// The IP Address Mask 
	BYTE ip_GateWay[ IPADDRESS_SIZE ];		// The address of the P gateway 
	BYTE ip_TftpServer[ IPADDRESS_SIZE ];	// The address of the TFTP server to load data from for debugging 
	DWORD baud_rate;						// The initial system Baud rate 
	unsigned char wait_seconds;				// The number of seconds to wait before booting 
	unsigned char bBoot_To_Application;		// True if we boot to the application, not the monitor 
	unsigned char bException_Action;		// What should we do when we have an exception? 
	unsigned char m_FileName[ 80 ];			// The file name of the TFTP file to load 
	unsigned char mac_address[ MACADDRESS_SIZE ];			// The Ethernet MAC address 
	unsigned char ser_port;
	unsigned char ip_DNS[ IPADDRESS_SIZE ];
	unsigned char core_mac_address[ MACADDRESS_SIZE ];	// The Base unit MAC address 
	unsigned char typeof_if;
	unsigned char direct_tx;
	unsigned char m_dwIP_used[ IPADDRESS_SIZE ];
	unsigned long m_dwMASK_used;
	unsigned long m_dwGate_used;
	unsigned long m_dwDNS_used;
	unsigned char m_bUnused[ 3 ];
	unsigned char m_q_boot;					// True to boot without messages 
	unsigned short checksum;				// A Checksum for this structure 
};

struct MsgHeader
{
	DWORD m_dwKeyValue;
	BYTE m_bAction;
};

struct DeviceConfigStruct
{
	MsgHeader m_msgHeader{ };
	DeviceConfigRecord m_devConfigRecord{ };
};

struct AppConfigRecord
{
	DWORD recordLength;						// Length of the AppConfigRecord    
	BYTE spuMACAddress[ MACADDRESS_SIZE ];	// The SPU3 Device MAC address 
	BYTE commType;							// Communication Interface Type = 0 ( Serial ), 1 ( Ethernet )
	BYTE portOrientation;					// Port Orientation , 0 ( right to left ), 1 ( left to right )		
	BYTE spuVersionMajor;					// SPU FW Major version
	BYTE spuVersionMinor;					// SPU FW Minor version
	WORD spuFWBuildYear;					// SPU FW Build Year
	BYTE spuFWBuildMonth;					// SPU FW Build Month
	BYTE spuFWBuildDay;						// SPU FW Build Day
	BYTE serverIPAddress[ IPADDRESS_SIZE ];	// The Mercury Server IP Address ( stored in the SPU3 flash RAM	)
	DWORD serverSockPort;					// Socket Port to communicate with Mercury App Server over Ethernet ( stored in the SPU3 flash RAM )
	BYTE spuFWPerformanceModeOption;		// SPU Performance Mode option, 0 ( Disabled ), 1 ( Enabled ) 
	BYTE spuFWOptions[ 19 ];				// Reserved for future use
	unsigned short  checksum;				// A Checksum for this structure ( not being used, don't have info on how Netburner calculates the checksum )
};

struct AppConfigStruct
{
	MsgHeader m_msgHeader{ };
	AppConfigRecord m_appConfigRecord{ };
};

// To hold SPU3 Device runtime status
struct DeviceRuntimeStatus
{
	bool m_blinkLEDs;
};

struct TreeItemRecord
{
	DeviceConfigRecord m_devConfigRecord{ };
	AppConfigRecord m_appConfigRecord{ };
	DeviceRuntimeStatus m_devRuntimeStatus{ };
};

struct DeviceOperationRecord
{
	DWORD recordLength;						// Length of the AppConfigRecord    
	BYTE spuMACAddress[ MACADDRESS_SIZE ];	// The SPU3 Device MAC address 
	BYTE operationData;						// Data to perform the requested operation, it is being used only for Blink LEDs, 1 = true, 0 = false
	unsigned short checksum;				// A Checksum for this structure ( not being used, don't have info on how Netburner calculates the checksum )
};

struct DeviceOperationStruct
{
	MsgHeader m_msgHeader{ };
	DeviceOperationRecord m_operationRecord{ };
};
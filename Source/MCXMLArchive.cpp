// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MCXMLArchive.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#include "stdafx.h"
#include "MCXMLArchive.h"
#include "MCXMLTagsDefines.h"
#include "MCSystem.h"
#include "MCPress.h"

const CString SERIAL_COMM_TYPE = TEXT( "SERIAL" );
const CString ETHERNET_COMM_TYPE = TEXT( "ETHERNET" );

/// <summary>
/// Initializes a new instance of the <see cref="CMCXMLArchive"/> class.
/// </summary>
/// <param name="pSystem">The p system.</param>
CMCXMLArchive::CMCXMLArchive( CMCSystem* pSystem )
{
	ASSERT( pSystem );
	m_pSystem = pSystem;
	m_pPress = m_pSystem->GetPress();
}

/// <summary>
/// Finalizes an instance of the <see cref="CMCXMLArchive"/> class.
/// </summary>
CMCXMLArchive::~CMCXMLArchive()
{ }

/// <summary>
/// Loads the system configuration.
/// </summary>
/// <param name="strXMLPath">The string XML path.</param>
/// <returns></returns>
BOOL CMCXMLArchive::LoadSystemConfiguration( const CString& strXMLPath )
{
	ASSERT( m_pSystem );

	bool bResult = false;
	CString strValue = _T( "" );

	bResult = this->Load_XML_Document( strXMLPath );
	if( !bResult )
		return FALSE;

	// Clear the press data, if it was loaded already
	m_pPress->Clear();

	this->Go_to_Child( XT_MCSYSTEM_CONFIG );
	CString strVersion = this->Get_CurrentName();
	if( _tcsicmp( strVersion, XT_MCSYSTEM_CONFIG ) == 0 )
	{
		bResult = this->Get_Attribute_Value( XT_MCSYSTEMCONFIG_VERSION, strValue );
	}

	bResult = this->Go_to_Child();
	if( !bResult )
		return FALSE;

	do
	{
		CString strTagName = this->Get_CurrentName();

		// Site Name
		if( _tcsicmp( strTagName, XT_SYSTEM_SITENAME ) == 0 )
		{
			bResult = this->Get_Attribute_Value( VALUE, strValue );
			if( bResult )
				m_pSystem->SetSiteName( strValue );
		}
		// Site Number
		else if( _tcsicmp( strTagName, XT_SITE_NUMBER ) == 0 )
		{
			bResult = this->Get_Attribute_Value( VALUE, strValue );
			if( bResult )
				m_pSystem->SetSiteNumber( _ttoi( ( LPCTSTR )strValue ) );
		}
		// Press
		else if( _tcsicmp( strTagName, XT_MCSYSTEM_PRESSES ) == 0 )
		{
			// Get Total Press count
			bResult = this->Get_Attribute_Value( ARRAY_SIZE, strValue );
			if( !bResult )
				continue;

			bResult = this->Go_to_Child();
			if( !bResult )
				continue;

			BYTE byPressIdx = 0;
			do
			{
				// READ PRESS DETAILS
				if( _tcsicmp( this->Get_CurrentName(), XT_SYSTEM_PRESS ) == 0 )
				{
					ReadSystemPressConfiguration();
					byPressIdx++;
				}

			} while( ( bResult = this->Go_Forward() ) && ( byPressIdx < 1 ) );

			bResult = this->Go_to_Parent();
		}
		// Mercury Server Config
		else if( _tcsicmp( strTagName, XT_MERCURY_SERVER_CONFIG ) == 0 )
		{
			ReadMercuryServerConfig();
		}

	} while( bResult = this->Go_Forward() );

	return TRUE;
}

/// <summary>
/// Reads the system press configuration.
/// </summary>
/// <returns></returns>
BOOL CMCXMLArchive::ReadSystemPressConfiguration()
{
	ASSERT( m_pPress );

	bool bResult = false;
	CString strValue = _T( "" );

	bResult = this->Get_Attribute_Value( NAME, strValue );
	if( bResult )
	{
		// Press Name
		m_pPress->SetPressName( strValue );
	}

	bResult = this->Go_to_Child();
	if( !bResult )
		return FALSE;

	do
	{
		CString strTagName = this->Get_CurrentName();

		// READ PRESS UNITS DETAILS
		if( _tcsicmp( strTagName, XT_PRESS_UNITS ) == 0 )
		{
			// Get Total Press Units count
			bResult = this->Get_Attribute_Value( ARRAY_SIZE, strValue );
			if( !bResult )
				continue;

			int byUnitsCnt = _ttoi( strValue );

			if( ( byUnitsCnt <= 0 ) || ( byUnitsCnt >= UCHAR_MAX ) )
				continue;

			bResult = this->Go_to_Child();
			if( !bResult )
				continue;

			// READ PRESS UNIT DETAILS
			BYTE byUnitIdx = 0;
			do
			{
				if( _tcsicmp( this->Get_CurrentName(), XT_PRESS_UNIT ) == 0 )
				{
					ReadConfig_PressUnit();
					byUnitIdx++;
				}

			} while( ( bResult = this->Go_Forward() ) && ( byUnitIdx < byUnitsCnt ) );

			bResult = this->Go_to_Parent();
		}
		else if( _tcsicmp( strTagName, XT_MCPRESS_SPUS ) == 0 )
		{
			bResult = this->Get_Attribute_Value( ARRAY_SIZE, strValue );
			if( !bResult )
				continue;

			BYTE byCount = _ttoi( strValue );
			if( byCount <= 0 )
				continue;

			bResult = this->Go_to_Child();
			if( !bResult )
				continue;

			BYTE byIdx = 0;
			do
			{
				if( _tcsicmp( this->Get_CurrentName(), XT_MCPRESS_SPU ) == 0 )
				{
					CMCSPU spuObject;

					bResult = this->Get_Attribute_Value( NAME, strValue );
					if( bResult )
						spuObject.SetSPUName( strValue );

					bResult = this->Get_Attribute_Value( XT_MCPRESS_SPU_COMPORT, strValue );
					if( bResult )
						spuObject.SetCOMPort( strValue );

					bResult = this->Get_Attribute_Value( XT_IP_ADDRESS, strValue );
					if( bResult )
						spuObject.SetIPAddress( strValue );

					bResult = this->Get_Attribute_Value( XT_COMM_TYPE, strValue );
					if( bResult )
						spuObject.SetSerialInterface( _tcsicmp( strValue, SERIAL_COMM_TYPE ) == 0 );

					// Add SPU data to SPUs list
					m_pPress->AddToSPUsList( spuObject );

					byIdx++;
				}

			} while( ( bResult = this->Go_Forward() ) && ( byIdx < byCount ) );

			bResult = this->Go_to_Parent();
		}

	} while( bResult = this->Go_Forward() );

	bResult = this->Go_to_Parent();

	return TRUE;
}

/// <summary>
/// Reads the configuration press unit.
/// </summary>
/// <returns></returns>
BOOL CMCXMLArchive::ReadConfig_PressUnit()
{
	BYTE byUnitFountainCnt = 0;
	bool bResult = false;
	CString strValue = _T( "" );

	bResult = this->Get_Attribute_Value( NAME, strValue );
	bResult = this->Go_to_Child();
	if( !bResult )
		return FALSE;

	do
	{
		CString strTagName = this->Get_CurrentName();

		if( _tcsicmp( strTagName, XT_INKERS ) == 0 )
		{
			// Get Total UnitFountain count
			bResult = this->Get_Attribute_Value( ARRAY_SIZE, strValue );
			if( !bResult )
				continue;

			byUnitFountainCnt = _ttoi( strValue );

			if( ( byUnitFountainCnt <= 0 ) || ( byUnitFountainCnt >= UCHAR_MAX ) )
				continue;

			bResult = this->Go_to_Child();
			if( !bResult )
				continue;

			// READ PRESS UNIT INKERS DETAILS
			BYTE byFountainIdx = 0;
			do
			{
				if( _tcsicmp( this->Get_CurrentName(), XT_INKER ) == 0 )
				{
					CMCInker inker;
					if( ReadConfig_UnitInker( &inker ) )
					{
						// Add Inker to the Press
						m_pPress->AddToInkersList( inker );
					}

					byFountainIdx++;
				}

			} while( ( bResult = this->Go_Forward() ) && ( byFountainIdx < byUnitFountainCnt ) );

			bResult = this->Go_to_Parent();
		}

	} while( bResult = this->Go_Forward() );

	bResult = this->Go_to_Parent();

	return TRUE;
}


/// <summary>
/// Reads the configuration unit inker.
/// </summary>
/// <param name="pInker">The inker.</param>
/// <returns></returns>
BOOL CMCXMLArchive::ReadConfig_UnitInker( CMCInker *pInker )
{
	ASSERT( pInker );

	BYTE byUnitKeysCnt = 0;
	bool bResult = false;
	CString strValue = _T( "" );

	bResult = this->Get_Attribute_Value( NAME, strValue );
	bResult = this->Go_to_Child();
	if( !bResult )
		return FALSE;

	// Set Inker Name
	pInker->SetName( strValue );

	do
	{
		CString sFountainTagName = this->Get_CurrentName();

		if( _tcsicmp( sFountainTagName, XT_MCUNIT_INKER_SERVO_BANKS ) == 0 )
		{
			bResult = this->Get_Attribute_Value( ARRAY_SIZE, strValue );
			if( !bResult )
				continue;

			BYTE byBankCnt = _ttoi( strValue );
			if( ( byBankCnt <= 0 ) || ( byBankCnt >= UCHAR_MAX ) )
				continue;

			bResult = this->Go_to_Child();
			if( !bResult )
				return FALSE;

			BYTE byIndex = 0;
			do
			{
				if( _tcsicmp( this->Get_CurrentName(), XT_MCUNIT_INKER_SERVO_BANK ) == 0 )
				{
					ReadConfig_ServoBank( pInker );

					byIndex++;
				}

			} while( ( bResult = this->Go_Forward() ) && ( byIndex < byBankCnt ) );

			bResult = this->Go_to_Parent();
		}

	} while( bResult = this->Go_Forward() );

	bResult = this->Go_to_Parent();

	return TRUE;
}

/// <summary>
/// Reads the configuration servo bank.
/// </summary>
/// <param name="pInker">The inker.</param>
/// <returns></returns>
BOOL CMCXMLArchive::ReadConfig_ServoBank( CMCInker *pInker )
{
	ASSERT( pInker );

	bool bResult = this->Go_to_Child();
	if( !bResult )
		return FALSE;

	do
	{
		CString strTagName = this->Get_CurrentName();

		if( _tcsicmp( strTagName, XT_MCINKER_SERVO_ADDRESS ) == 0 )
		{
			CString strSPUName = _T( "" ), strPort = _T( "" );
			bResult = this->Get_Attribute_Value( XT_MCSERVO_BANK_SPU_NAME, strSPUName );
			if( bResult )
			{
				bResult = this->Get_Attribute_Value( XT_MCSERVO_BANK_SPU_PORT, strPort );
				if( bResult )
				{
					// Add SPU Name and Port# to the Rail list
					pInker->AddToRailList( strSPUName, _ttoi( strPort ) );
				}
			}
		}

	} while( bResult = this->Go_Forward() );

	bResult = this->Go_to_Parent();

	return TRUE;
}

/// <summary>
/// Reads the mercury server configuration.
/// </summary>
/// <returns></returns>
BOOL CMCXMLArchive::ReadMercuryServerConfig()
{
	ASSERT( m_pSystem );

	bool bResult = FALSE;

	CString strTagName = this->Get_CurrentName();
	if( _tcsicmp( strTagName, XT_MERCURY_SERVER_CONFIG ) == 0 )
	{
		bResult = this->Go_to_Child();	//MERCURY_SERVER
		if( !bResult )
		{
			return FALSE;
		}

		CString strValue = _T( "" );

		do
		{
			if( _tcsicmp( this->Get_CurrentName(), XT_MERCURY_SERVER ) == 0 )
			{
				// Mercury Server's IP Address
				bResult = this->Get_Attribute_Value( XT_IP_ADDRESS, strValue );
				if( bResult )
				{
					// Set Mercury Server IP Address
					m_pSystem->SetServerIPAddress( strValue );
				}

				// Port number for Ethernet Comm with SPU3
				bResult = this->Get_Attribute_Value( XT_PORT, strValue );
				if( bResult )
				{
					unsigned int port = static_cast< UINT >( _ttoi( strValue ) );
					m_pSystem->SetServerSockPort( port );
				}
			}

		} while( bResult = this->Go_Forward() );

		// Move to the root MERCURY_SERVER_CONFIG
		bResult = this->Go_to_Parent();
	}

	return bResult;
}
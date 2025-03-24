// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MCPress.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#include "stdafx.h"
#include "MCPress.h"

/// <summary>
/// Initializes a new instance of the <see cref="CMCPress"/> class.
/// </summary>
CMCPress::CMCPress()
{
	InitParameters();
}

/// <summary>
/// Finalizes an instance of the <see cref="CMCPress"/> class.
/// </summary>
CMCPress::~CMCPress()
{ }

/// <summary>
/// Initializes the parameters.
/// </summary>
void CMCPress::InitParameters()
{
	m_pressName = _T( "" );
	m_spuList.clear();
	m_inkerList.clear();
	m_spuIPAddressToInkersMap.clear();
	m_spuIPAddressToNameMap.clear();
}

/// <summary>
/// Gets the name of the press.
/// </summary>
/// <returns></returns>
CString& CMCPress::GetPressName()
{
	return m_pressName;
}

/// <summary>
/// Sets the name of the press.
/// </summary>
/// <param name="pressName">Name of the press.</param>
void CMCPress::SetPressName( const CString& pressName )
{
	m_pressName = pressName;
}

/// <summary>
/// Adds to SPUs list.
/// </summary>
/// <param name="spu">The spu.</param>
void CMCPress::AddToSPUsList( const CMCSPU& spu )
{
	m_spuList.push_back( spu );
}

/// <summary>
/// Adds to inkers list.
/// </summary>
/// <param name="inker">The inker.</param>
void CMCPress::AddToInkersList( const CMCInker& inker )
{
	m_inkerList.push_back( inker );
}

/// <summary>
/// Clears this instance.
/// </summary>
void CMCPress::Clear()
{
	InitParameters();
}

/// <summary>
/// Generates the SPU to inkers list.
/// </summary>
void CMCPress::GenerateSPUToInkersList()
{
	// First pass, generate SPU Name to Inkers List
	std::map< CString, std::vector< CMCInker > > spuNameToInkersMap;
	for( CMCSPU spu : m_spuList )
	{
		// Consider SPU3 devices ( regardless of Ethernet or Serial interface )
		if( spu.IsSPU3Device() )
		{
			std::vector< CMCInker > inkerList;
			CString spuName = spu.GetSPUName();

			GetInkersListBySPUName( spuName, inkerList );
			if( static_cast< int >( inkerList.size() ) > 0 )
			{
				spuNameToInkersMap[ spuName ] = inkerList;
			}
		}
	}

	// Seconds pass, let's generate SPU IP Address to Inkers List
	m_spuIPAddressToInkersMap.clear();
	m_spuIPAddressToNameMap.clear();

	for( CMCSPU spu : m_spuList )
	{
		// Consider SPU3 devices ( regardless of Ethernet or Serial interface )
		if( spu.IsSPU3Device() )
		{
			CString spuName = spu.GetSPUName();
			CString spuIPAddress = spu.GetIPAddress();

			m_spuIPAddressToNameMap[ spuIPAddress ] = spuName;

			if( spuNameToInkersMap.find( spuName ) != spuNameToInkersMap.end() )
			{
				m_spuIPAddressToInkersMap[ spuIPAddress ] = spuNameToInkersMap[ spuName ];
			}
		}
	}
}

/// <summary>
/// Gets the name of the inkers list by SPU.
/// </summary>
/// <param name="spuName">Name of the SPU.</param>
/// <param name="inkersList">The inkers list.</param>
void CMCPress::GetInkersListBySPUName( const CString& spuName, std::vector< CMCInker >& inkersList )
{
	for( CMCInker inker : m_inkerList )
	{
		std::map< CString, std::vector<BYTE> > spuToPortsMap = inker.GetRailsList();
		for( auto supToPortIter : spuToPortsMap )
		{
			if( _tcsicmp( supToPortIter.first, spuName ) == 0 )
			{
				inkersList.push_back( inker );
			}
		}
	}
}

/// <summary>
/// Gets the inkers list by SPU IP address.
/// </summary>
/// <param name="spuIPAddress">The SPU IP address.</param>
/// <param name="inkersList">The inkers list.</param>
void CMCPress::GetInkersListBySPUIPAddress( const CString& spuIPAddress, std::vector< CMCInker >& inkersList )
{
	if( m_spuIPAddressToInkersMap.find( spuIPAddress ) != m_spuIPAddressToInkersMap.end() )
	{
		inkersList = m_spuIPAddressToInkersMap[ spuIPAddress ];
	}
}

/// <summary>
/// Gets the SPU name by IP address.
/// </summary>
/// <param name="spuIPAddress">The SPU IP address.</param>
/// <returns></returns>
CString CMCPress::GetSPUNameByIPAddress( const CString& spuIPAddress )
{
	CString spuName = _T( "" );

	if( m_spuIPAddressToNameMap.find( spuIPAddress ) != m_spuIPAddressToNameMap.end() )
	{
		spuName = m_spuIPAddressToNameMap[ spuIPAddress ];
	}

	return spuName;
}

/// <summary>
/// Gets the spuip address list.
/// </summary>
/// <param name="spuIPAddressList">The SPU IP address list.</param>
void CMCPress::GetSPUIPAddressList( std::vector< CString >& spuIPAddressList )
{
	for( CMCSPU spu : m_spuList )
	{
		// Consider SPU3 devices ONLY ( regardless of Ethernet or Serial interface )
		if( spu.IsSPU3Device() )
		{
			CString ipAddress = spu.GetIPAddress();
			if( !ipAddress.IsEmpty() )
			{
				spuIPAddressList.push_back( ipAddress );
			}
		}
	}
}

/// <summary>
/// Gets the SPU3 device count.
/// </summary>
/// <returns></returns>
int CMCPress::GetSPU3DeviceCount()
{
	int spuCount = 0;

	for( CMCSPU spu : m_spuList )
	{
		// Consider SPU3 devices ONLY ( regardless of Ethernet or Serial interface )
		if( spu.IsSPU3Device() )
		{
			++spuCount;
		}
	}

	return spuCount;
}


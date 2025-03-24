// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MCSystem.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#include "stdafx.h"
#include "MCPress.h"
#include "MCSystem.h"

/// <summary>
/// Initializes a new instance of the <see cref="CMCSystem"/> class.
/// </summary>
CMCSystem::CMCSystem()
{
	InitParameters();
}

/// <summary>
/// Finalizes an instance of the <see cref="CMCSystem"/> class.
/// </summary>
CMCSystem::~CMCSystem()
{ }

/// <summary>
/// Initializes the parameters.
/// </summary>
void CMCSystem::InitParameters()
{
	m_siteName = _T( "" );
	m_siteNumber = 0;
	m_serverSockPort = 0;
	m_serverIPAddress = _T( "" );
	m_pPress = std::unique_ptr< CMCPress >( new CMCPress() );
	ASSERT( m_pPress );
}

/// <summary>
/// Gets the name of the site.
/// </summary>
/// <returns></returns>
CString& CMCSystem::GetSiteName()
{
	return m_siteName;
}

/// <summary>
/// Gets the site number.
/// </summary>
/// <returns></returns>
UINT CMCSystem::GetSiteNumber()
{
	return m_siteNumber;
}

/// <summary>
/// Gets the server sock port.
/// </summary>
/// <returns></returns>
UINT CMCSystem::GetServerSockPort()
{
	return m_serverSockPort;
}

/// <summary>
/// Gets the server ip address.
/// </summary>
/// <returns></returns>
CString& CMCSystem::GetServerIPAddress()
{
	return m_serverIPAddress;
}

/// <summary>
/// Sets the name of the site.
/// </summary>
/// <param name="siteName">Name of the site.</param>
void CMCSystem::SetSiteName( const CString& siteName )
{
	m_siteName = siteName;
}

/// <summary>
/// Sets the site number.
/// </summary>
/// <param name="siteNumber">The site number.</param>
void CMCSystem::SetSiteNumber( UINT siteNumber )
{
	m_siteNumber = siteNumber;
}

/// <summary>
/// Sets the server sock port.
/// </summary>
/// <param name="serverPort">The server port.</param>
void CMCSystem::SetServerSockPort( UINT serverPort )
{
	m_serverSockPort = serverPort;
}

/// <summary>
/// Sets the server IP address.
/// </summary>
/// <param name="ipAddress">The IP address.</param>
void CMCSystem::SetServerIPAddress( const CString& ipAddress )
{
	m_serverIPAddress = ipAddress;
}

/// <summary>
/// Gets the press.
/// </summary>
/// <returns></returns>
CMCPress* CMCSystem::GetPress()
{
	return m_pPress.get();
}


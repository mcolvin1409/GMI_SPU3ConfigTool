// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MCSPU.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#include "stdafx.h"
#include "MCSPU.h"

const CString emptyIPAddress = _T( "0.0.0.0" );

/// <summary>
/// Initializes a new instance of the <see cref="CMCSPU"/> class.
/// </summary>
CMCSPU::CMCSPU()
{
	InitParameters();
}

/// <summary>
/// Finalizes an instance of the <see cref="CMCSPU"/> class.
/// </summary>
CMCSPU::~CMCSPU()
{ }

/// <summary>
/// Initializes the parameters.
/// </summary>
void CMCSPU::InitParameters()
{
	m_spuName = _T( "" );
	m_COMPort = _T( "" );
	m_IPAddress = emptyIPAddress;
	m_serialInterface = true;
	m_bSPU3Device = false;
}

/// <summary>
/// Gets the name of the SPU.
/// </summary>
/// <returns></returns>
CString& CMCSPU::GetSPUName()
{
	return m_spuName;
}

/// <summary>
/// Gets the IP address.
/// </summary>
/// <returns></returns>
CString& CMCSPU::GetIPAddress()
{
	return m_IPAddress;
}

/// <summary>
/// Gets the COM port.
/// </summary>
/// <returns></returns>
CString& CMCSPU::GetCOMPort()
{
	return m_COMPort;
}

/// <summary>
/// Determines whether [is serial interface].
/// </summary>
/// <returns>
///   <c>true</c> if [is serial interface]; otherwise, <c>false</c>.
/// </returns>
bool CMCSPU::IsSerialInterface()
{
	return m_serialInterface;
}

/// <summary>
/// Sets the name of the SPU.
/// </summary>
/// <param name="spuName">Name of the spu.</param>
void CMCSPU::SetSPUName( const CString& spuName )
{
	m_spuName = spuName;
}

/// <summary>
/// Sets the IP address.
/// </summary>
/// <param name="ipAddress">The IP address.</param>
void CMCSPU::SetIPAddress( const CString& ipAddress )
{
	m_IPAddress = ipAddress;
	m_bSPU3Device = ( 0 != emptyIPAddress.CompareNoCase( m_IPAddress ) );
}

/// <summary>
/// Sets the COM port.
/// </summary>
/// <param name="comPort">The COM port.</param>
void CMCSPU::SetCOMPort( const CString& comPort )
{
	m_COMPort = comPort;
}

/// <summary>
/// Sets the serial interface.
/// </summary>
/// <param name="serialInterface">if set to <c>true</c> [serial interface].</param>
void CMCSPU::SetSerialInterface( bool serialInterface )
{
	m_serialInterface = serialInterface;
}

/// <summary>
/// Determines whether [is SPU3 device].
/// </summary>
/// <returns>
///   <c>true</c> if [is SPU3 device]; otherwise, <c>false</c>.
/// </returns>
bool CMCSPU::IsSPU3Device()
{
	return m_bSPU3Device;
}

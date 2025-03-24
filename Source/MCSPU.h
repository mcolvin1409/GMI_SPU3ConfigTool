
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MCSPU.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

class CMCSPU
{
public:
	CMCSPU();
	virtual ~CMCSPU();

public:
	CString& GetSPUName();
	CString& GetIPAddress();
	CString& GetCOMPort();
	bool IsSerialInterface();
	bool IsSPU3Device();

	void SetSPUName( const CString& spuName );
	void SetIPAddress( const CString& ipAddress );
	void SetCOMPort( const CString& comPort );
	void SetSerialInterface( bool serialInterface );

private:
	void InitParameters();
	
private:
	CString m_spuName;
	CString m_COMPort;
	CString m_IPAddress;
	bool m_serialInterface;
	bool m_bSPU3Device;
};


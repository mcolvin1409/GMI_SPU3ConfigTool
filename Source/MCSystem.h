
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MCSystem.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include <memory>
using namespace std;

class CMCPress;

class CMCSystem
{
public:
	CMCSystem();
	virtual ~CMCSystem();

public:
	enum ConfigurationLoadStatus { NOT_FOUND, INVALID_FORMAT, FILE_LOADED, FILE_CREATED };
	CString& GetSiteName();
	UINT GetSiteNumber();
	UINT GetServerSockPort();
	CString& GetServerIPAddress();

	void SetSiteName( const CString& siteName );
	void SetSiteNumber( UINT siteNumber );
	void SetServerSockPort( UINT serverPort );
	void SetServerIPAddress( const CString& ipAddress );

	CMCPress* GetPress();		

private:
	void InitParameters();
	
private:
	CString m_siteName;
	UINT m_siteNumber;
	UINT m_serverSockPort;
	CString m_serverIPAddress;
	std::unique_ptr< CMCPress > m_pPress;
};


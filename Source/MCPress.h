
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MCPress.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include "MCSPU.h"
#include "MCInker.h"
#include <vector>

using namespace std;

class CMCPress
{
public:
	CMCPress();
	virtual ~CMCPress();

private:
	void InitParameters();
	void GetInkersListBySPUName( const CString& spuName, std::vector< CMCInker >& inkersList );
	
public:
	CString& GetPressName();
	void SetPressName( const CString& pressName );
	void AddToSPUsList( const CMCSPU& spu );
	void AddToInkersList( const CMCInker& inker );
	void Clear();
	void GenerateSPUToInkersList();
	void GetSPUIPAddressList( std::vector< CString >& spuIPAddressList );
	void GetInkersListBySPUIPAddress( const CString& spuIPAddress, std::vector< CMCInker >& inkersList );
	CString GetSPUNameByIPAddress( const CString& spuIPAddress );
	int GetSPU3DeviceCount();

private:
	CString m_pressName;
	std::vector< CMCInker > m_inkerList;
	std::vector< CMCSPU > m_spuList;		
	std::map< CString, std::vector< CMCInker > > m_spuIPAddressToInkersMap;	
	std::map< CString, CString > m_spuIPAddressToNameMap;
};


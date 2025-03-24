
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MCInker.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include <map>
#include <vector>

using namespace std;

class CMCInker
{
public:
	CMCInker();
	virtual ~CMCInker();

private:
	void InitParameters();

public:
	void SetName( const CString& inkerName );
	CString& GetName();
	void AddToRailList( const CString& spuName, BYTE portNumber );
	std::map< CString, std::vector<BYTE> > GetRailsList();

private:
	CString m_inkerName;
	std::map< CString, std::vector< BYTE > > m_spuToPortMap;
};



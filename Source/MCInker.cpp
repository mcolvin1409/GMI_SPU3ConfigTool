// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MCInker.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#include "stdafx.h"
#include "MCInker.h"
#include "MCSPU.h"

/// <summary>
/// Initializes a new instance of the <see cref="CMCInker"/> class.
/// </summary>
CMCInker::CMCInker()
{
	InitParameters();
}

/// <summary>
/// Finalizes an instance of the <see cref="CMCInker"/> class.
/// </summary>
CMCInker::~CMCInker()
{ }

/// <summary>
/// Initializes the parameters.
/// </summary>
void CMCInker::InitParameters()
{
	m_inkerName = _T( "" );
	m_spuToPortMap.clear();
}

/// <summary>
/// Adds to rail list.
/// </summary>
/// <param name="spuName">Name of the spu.</param>
/// <param name="portNumber">The port number.</param>
void CMCInker::AddToRailList( const CString& spuName, BYTE portNumber )
{
	if( m_spuToPortMap.find( spuName ) == m_spuToPortMap.end() )
	{
		std::vector< BYTE > portList;
		portList.push_back( portNumber );

		m_spuToPortMap.insert( std::pair< CString, std::vector<BYTE>>( spuName, portList ) );
	}
	else
	{
		m_spuToPortMap[ spuName ].push_back( portNumber );
	}
}

/// <summary>
/// Gets the rails list.
/// </summary>
/// <returns></returns>
std::map< CString, std::vector<BYTE> > CMCInker::GetRailsList()
{
	return m_spuToPortMap;
}

/// <summary>
/// Sets the name.
/// </summary>
/// <param name="inkerName">Name of the inker.</param>
void CMCInker::SetName( const CString& inkerName )
{
	m_inkerName = inkerName;
}

/// <summary>
/// Gets the name.
/// </summary>
/// <returns></returns>
CString& CMCInker::GetName()
{
	return m_inkerName;
}

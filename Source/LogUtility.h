// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="LogUtility.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include <fstream>

class CLogUtility
{
public:
	CLogUtility();
	virtual ~CLogUtility();

public:
	void SetFileName( const CString& logFileName );
	void LogMessage( const CString& logMessage );

private:
	HANDLE	m_guard;		// Guard 	
	CString m_fileName;		// Name of the Log file name
	std::wofstream m_fileStream;	// file stream
};
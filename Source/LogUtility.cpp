// ***********************************************************************
// Author           : Mark C
// Created          : 09-May-2017
//
// ***********************************************************************
// <copyright file="LogUtility.cpp" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#include "stdafx.h"
#include "LogUtility.h"
#include <sstream>

/// <summary>
/// Initializes a new instance of the <see cref="CLogUtility"/> class.
/// </summary>
CLogUtility::CLogUtility()
{
	m_fileName = _T("");
	m_guard = CreateMutex( NULL, FALSE, NULL );
}

/// <summary>
/// Finalizes an instance of the <see cref="CLogUtility"/> class.
/// </summary>
CLogUtility::~CLogUtility()
{
	// close the mutex
	if( NULL != m_guard )
	{
		ReleaseMutex( m_guard );
		CloseHandle( m_guard );
	}

	// close the log file, if opened
	if( m_fileStream.is_open() )
	{
		m_fileStream.close();
	}
}

/// <summary>
/// Sets the name of the file.
/// </summary>
/// <param name="logFileName">Name of the log file.</param>
void CLogUtility::SetFileName( const CString& logFileName )
{
	m_fileName = logFileName;
	m_fileStream.open( logFileName, std::wios::out | std::wios::app );
	m_fileStream << std::endl;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="logMessage">The log message.</param>
void CLogUtility::LogMessage( const CString& logMessage )
{
	// If file is NOT opened yet (OR) file name is empty, cannot write to the file stream	
	if( !m_fileStream.is_open() ||
		m_fileName.IsEmpty() ||
		( NULL == m_guard ) )
	{
		return;
	}

	try
	{
		WaitForSingleObject( m_guard, INFINITE );

		// get the current system time
		SYSTEMTIME systemTime, *p_st;
		GetLocalTime( &systemTime );
		p_st = &systemTime;
		CString timeStamp = _T("");
		timeStamp.Format( _T("%4.4d:%2.2d:%2.2d-%2.2d:%2.2d:%2.2d:%3.3d "),
						  p_st->wYear, p_st->wMonth, p_st->wDay,
						  p_st->wHour, p_st->wMinute, p_st->wSecond, p_st->wMilliseconds );

		// log the time and message details
		m_fileStream << timeStamp.GetString() << logMessage.GetString() << std::endl;

		ReleaseMutex( m_guard );
	}
	catch( CException* ex )
	{
		ReleaseMutex( m_guard );
		ex->ReportError();
		ex->Delete();
	}
	catch( ... )
	{
		ReleaseMutex( m_guard );
	}
}


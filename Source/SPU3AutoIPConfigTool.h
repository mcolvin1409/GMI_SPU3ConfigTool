
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="SPU3AutoIPConfigTool.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

// SPU3AutoIPConfigTool.h : main header file for the SPU3AutoIPConfigTool application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CSPU3AutoIPConfigToolApp:
// See SPU3AutoIPConfigTool.cpp for the implementation of this class
//

class CSPU3AutoIPConfigToolApp : public CWinAppEx
{
	friend class CCommandProcessor;

public:
	CSPU3AutoIPConfigToolApp();

private:
	void SetAppBuildDate();
	BOOL ParseCmdOptions( LPCTSTR lpszParam );
	void SetExecutableFolderPath();
	void CreateLogsFolder();
	BOOL FirstInstance();
	BOOL RegisterWindowClass();

private:
	CString m_appExeFilePath;
	HANDLE m_hSingleInstanceMutex;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;
	CString m_appBuildDate;
	CString m_appVersion;	
	CString m_logFolderPath;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CSPU3AutoIPConfigToolApp theApp;

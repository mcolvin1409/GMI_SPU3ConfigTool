
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MCXMLArchive.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include "XML_PARSER.h"
#include "MigrationConstants.h"

class CMCSystem;
class CMCPress;
class CMCInker;

class CMCXMLArchive : public CXmlParser
{
public:
	CMCXMLArchive( CMCSystem* pSystem );
	virtual ~CMCXMLArchive();

public:
	BOOL LoadSystemConfiguration( const CString& strXMLPath );
	BOOL ReadSystemPressConfiguration();
	BOOL ReadConfig_PressUnit();
	BOOL ReadMercuryServerConfig();
	BOOL ReadConfig_UnitInker( CMCInker *pInker );
	BOOL ReadConfig_ServoBank( CMCInker *pInker );

private:
	CMCSystem *m_pSystem;
	CMCPress *m_pPress;
};


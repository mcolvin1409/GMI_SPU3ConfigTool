
#pragma once

#include "MigrationConstants.h"

class CAppException
{
protected:
	CAppException( const tstring &msg = _T( "" ) ) : m_message( msg )
	{ }

public:
	virtual ~CAppException()
	{ }

	const tstring &GetMessage()
	{
		return m_message;
	}

private:
	tstring m_message;
};


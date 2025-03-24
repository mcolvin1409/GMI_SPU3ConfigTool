
#pragma once

#include "AppException.h"

class InvalidAccessException : public CAppException
{
public:
	InvalidAccessException( const tstring &errorMessage ) : CAppException( errorMessage )
	{ }

	virtual ~InvalidAccessException()
	{ }
};

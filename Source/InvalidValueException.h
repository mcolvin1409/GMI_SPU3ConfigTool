
#pragma once

#include "AppException.h"

class InvalidValueException : public CAppException
{
public:
	InvalidValueException( const tstring &message ) : CAppException( message )
	{ }

	virtual ~InvalidValueException( void )
	{ }
};


#pragma once

#include "AppException.h"

class IndexOutOfBoundsException : public CAppException
{
public:
	IndexOutOfBoundsException( const tstring &errorMessage ) : CAppException( errorMessage )
	{ }

	virtual ~IndexOutOfBoundsException()
	{ }
};

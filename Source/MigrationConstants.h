
// ***********************************************************************
// Author           : Mark C
// Created          : 15-May-2018
//
// ***********************************************************************
// <copyright file="MigrationConstants.h" company="AVT">
//     Copyright (c) AVT. All rights reserved.
// </copyright>
// ***********************************************************************

#pragma once

#include <string>

using namespace std;

//////////////////////////////////////////////////////////////////////////////////
//
//Generic Type Definitions supporting UNICODE/ ANSI datatypes accordingly.
//
//////////////////////////////////////////////////////////////////////////////////

typedef basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > tstring; 

typedef basic_stringstream<TCHAR, char_traits<TCHAR>,	allocator<TCHAR> > tstringstream;

typedef basic_ifstream<TCHAR, char_traits<TCHAR> > tifstream;

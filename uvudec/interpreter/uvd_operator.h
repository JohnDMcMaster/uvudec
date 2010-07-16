/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#pragma once

#include "uvd_types.h"
#include <string>

// x + y
#define UVD_OPERATOR_ADD				'+'
// x - y
#define UVD_OPERATOR_SUBTRACT			'-'
// x * y
#define UVD_OPERATOR_MULTIPLY			'*'
// x % y
#define UVD_OPERATOR_MODULUS			'%'
// x / y
#define UVD_OPERATOR_DIVIDE				'/'
// x << y
#define UVD_OPERATOR_BITWISE_LEFT		'l'
// x >> y
#define UVD_OPERATOR_BITWISE_RIGHT		'r'
// x ** y
#define UVD_OPERATOR_POWER				'P'

//Operator action
//Create for doing basic scripted arithmetic
class UVDOperator
{
public:
	UVDOperator();
	uv_err_t init();
	static uv_err_t getOperator(char operatorIn, UVDOperator **out);
	std::string toString();
	static std::string toString(char o);
	
	//Should be UVDVarient?
	uv_err_t operate(UVDVarient *l, UVDVarient *r, UVDVarient *result);
	
public:
	char m_operator;
};


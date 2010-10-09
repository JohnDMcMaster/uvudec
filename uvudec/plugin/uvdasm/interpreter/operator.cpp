/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_types.h"
#include "uvdasm/interpreter/operator.h"
#include <string>

UVDOperator::UVDOperator()
{
}

uv_err_t UVDOperator::init()
{
	return UV_ERR_OK;
}

//operator may need to be string later for bitwise shift
//Otherwise use defines and use random chars
uv_err_t UVDOperator::getOperator(char operatorIn, UVDOperator **out)
{
	UVDOperator *operatorTemp = NULL;
	uv_assert_ret(out);

	printf_debug("Getting operator: %s (%c)\n", toString(operatorIn).c_str(), operatorIn);

	operatorTemp = new UVDOperator();
	uv_assert_ret(operatorTemp);
	uv_assert_err_ret(operatorTemp->init());

	operatorTemp->m_operator = operatorIn;

	*out = operatorTemp;
	return UV_ERR_OK;
}

//Don't care about unary or trinary operators
uv_err_t UVDOperator::operate(UVDVarient *l, UVDVarient *r, UVDVarient *result)
{
	uv_assert_ret(l);
	uv_assert_ret(r);
	uv_assert_ret(result);
	
	uint32_t lVal = 0;
	uint32_t rVal = 0;
	uint32_t resultVal = 0;
	
	uv_assert_err_ret(l->getUI32(lVal));
	uv_assert_err_ret(r->getUI32(rVal));

	switch( m_operator )
	{
	case UVD_OPERATOR_ADD:
		resultVal = lVal + rVal;
		break;
	case UVD_OPERATOR_SUBTRACT:
		resultVal = lVal - rVal;
		break;
	case UVD_OPERATOR_MULTIPLY:
		resultVal = lVal * rVal;
		break;
	case UVD_OPERATOR_MODULUS:
		resultVal = lVal % rVal;
		break;
	case UVD_OPERATOR_DIVIDE:
		resultVal = lVal / rVal;
		break;
	default:
		return UV_DEBUG(UV_ERR_GENERAL);
	};

	result->setUI32(resultVal);

	return UV_ERR_OK;
}

std::string UVDOperator::toString()
{
	return toString(m_operator);
}

std::string UVDOperator::toString(char o)
{
	//Eventually might need fixup for multi char operators
	//Haha funny program errors from "" + o
	return std::string("") + o;	
}

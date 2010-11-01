/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

int a(int i);
int a2(int i);
int aa3(int i);
int aaa4(int i);
int caller()
{
	a(1);
	a2(1);
	aa3(1);
	aaa4(1);
	return 32423;
}


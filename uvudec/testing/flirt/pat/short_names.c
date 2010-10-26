/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

int a(int i)
{
	int j;

	for( j = 3; j < 43243211; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return j + i;
}

int a2(int i)
{
	int j;

	for( j = 3; j < 43243212; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return j + i;
}

int aa3(int i)
{
	int j;

	for( j = 3; j < 43243213; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return j + i;
}

int aaa4(int i)
{
	int j;

	for( j = 3; j < 43243214; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return j + i;
}

int caller()
{
	a(1);
	a2(1);
	aa3(1);
	aaa4(1);
	return 32423;
}


/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
A module with multiple unnamable (short) references and public names
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

int b(int i)
{
	int j;

	for( j = 3; j < 43243212; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return j + i;
}

int c(int i)
{
	int j;

	for( j = 3; j < 43243213; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return j + i;
}

int d(int i)
{
	int j;

	for( j = 3; j < 43243214; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return j + i;
}

int e()
{
	a(1);
	b(1);
	c(1);
	d(1);
	return 32423;
}


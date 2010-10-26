/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
Function that calls itself
Name could potentially appear in public names and referenced names (but only should be in the public name)
*/
int recursive(int i)
{
	int j;
	
	for( j = 3; j < 32432432; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return recursive(j + i);
}


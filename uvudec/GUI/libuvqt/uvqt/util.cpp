/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

#include "uvqt/util.h"
#include <stdio.h>

void UVDQtPrintRect(QRect rect)
{
	printf("rect: (%d, %d), width: %d, height: %d\n", rect.x(), rect.y(), rect.width(), rect.height());
}


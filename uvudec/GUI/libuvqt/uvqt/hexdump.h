/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVQT_HEXDUMP_H
#define UVQT_HEXDUMP_H

#include <QtDesigner/QDesignerExportWidget>
#include <string>
#include <stdint.h>
#include "uvqt/dynamic_text.h"
#include "uvd/data/data.h"

class UVQtHexdumpData;
class QDESIGNER_WIDGET_EXPORT UVQtHexdump : public UVQtScrollableDynamicText
{
    Q_OBJECT

public:
	UVQtHexdump(QWidget *parent = NULL);

	//To cast it
	UVQtHexdumpData *getHexdumpData();
	//We might want to be a hex editor later, so try not to be too const friendly
	uv_err_t setData(UVDData *data);
};

#endif


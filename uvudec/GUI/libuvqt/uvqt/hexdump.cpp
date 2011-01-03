/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvqt/hexdump.h"
#include "uvqt/hexdump_data.h"
#include "uvqt/util.h"
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include "stdio.h"
#include <stdint.h>
#include <math.h>

/*
UVQtHexdump
*/

UVQtHexdump::UVQtHexdump(QWidget *parent) : UVQtScrollableDynamicText(parent)
{
}

UVQtHexdumpData *UVQtHexdump::getHexdumpData()
{
	return (UVQtHexdumpData *)m_viewportShadow->m_textData; 
}

uv_err_t UVQtHexdump::setData(UVDData *data)
{
	UVQtHexdumpData *hexdumpData = new UVQtHexdumpData();
	hexdumpData->m_data = data;

	//If we called setDynamicData() before m_data was valid,
	//it would blow up since we wouldn't be able to query the ranges
	setDynamicData(hexdumpData);

	//Now that the viewport is setup, we can set its font
	QFont font;
	font.setFamily(QString::fromUtf8("Courier"));
	m_viewportShadow->setFont(font);

	return UV_ERR_OK;
}

/*
QSize UVQtHexdump::sizeHint() const 
{
	//Best way to get width size hint really is to render something
	//Otherwise, maintenance nightmare
	//leading address + hex view + char view + spacer contribution + char view padding
	QSize ret = QSize((7 + m_bytesPerRow * 5 + m_bytesPerRow / m_bytesPerSubRow + 2) * fontMetrics().width('0'),
			fontMetrics().height() * m_numberRows);
	printf("UVQtHexdump::sizeHint() = (width=%d, height=%d)\n", ret.width(), ret.height());
	return ret;
}
*/



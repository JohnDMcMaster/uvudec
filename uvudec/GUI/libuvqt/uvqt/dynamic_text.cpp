/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include "stdio.h"
#include <stdint.h>
#include <math.h>
#include "uvqt/dynamic_text.h"
#include "uvqt/util.h"
#include "uvd/util/util.h"
#include "uvqt/dynamic_text_plugin_impl.h"

/*
UVQtDynamicText
*/

UVQtDynamicText::UVQtDynamicText(QWidget *parent) : QWidget(parent)
{
	m_textData = NULL;
}

UVQtDynamicText::UVQtDynamicText(UVQtDynamicTextData *textData, QWidget *parent) : QWidget(parent)
{
	//m_startOffset = 0;
	//m_startIndex = 0;
	//UV_DEBUG(m_textData->begin(0, 0, &iter));
	//m_numberLines = 10;
	//m_textData = NULL;
	UV_DEBUG(setDynamicData(textData));
}

QSize UVQtDynamicText::sizeHint() const 
{
	//Really, we should see if we can impose a QScrollArea for x size, we are mostly concerned with y size right now
	//QSize ret = QSize((7 + m_bytesPerRow * 5 + m_bytesPerRow / m_bytesPerSubRow + 2) * fontMetrics().width('0'),
	QSize ret = QSize(2000,
			fontMetrics().height() * getNumberLines());
	printf("UVQtDynamicText::sizeHint() = (width=%d, height=%d)\n", ret.width(), ret.height());
	return ret;
}

void UVQtDynamicText::doPaintEventTest(QPaintEvent *event)
{
	printf("doPaintEventTest()\n");
	QPainter painter(this);
	printf("painter made\n");

	UVDQtPrintRect(event->rect());
	printf("pos x: %d, y: %d\n", pos().x(), pos().y());
	printf("basic x: %d, y: %d\n", x(), y());
	printf("geomtry x: %d, y: %d\n", geometry().x(), geometry().y());

	for( int i = 0; i < 3; ++i )
	{
		painter.drawLine(0, i * 5, 100, i * 5);
	}
	for( int i = 0; i < 3; ++i )
	{
		painter.drawLine(i * 5, 0, i * 5, 100);
	}

	painter.drawText(0, fontMetrics().height(), "ABC123");
}

unsigned int UVQtDynamicText::getNumberLines() const
{
	return height() / fontMetrics().height();
	//return m_numberLines;
}

void UVQtDynamicText::doPaintEvent(QPaintEvent *event, QPainter &painter)
{
	printf("***UVQtDynamicText::doPaintEvent()\n");
	if( m_textData == NULL )
	{
		printf("WARNING: could not doPaintEvent b/c no m_textData\n");
		return;
	}
	if( m_start.m_impl == NULL )
	{
		printf("WARNING: could not doPaintEvent b/c no m_start.m_impl\n");
		return;
	}
	printf("paint with %s\n", m_start.m_impl->toString().c_str());
	
	//printf("pos x: %d, y: %d\n", pos().x(), pos().y());
	//printf("basic x: %d, y: %d\n", x(), y());
	//printf("geomtry x: %d, y: %d\n", geometry().x(), geometry().y());
	//It paints the entire window, but the title bar cuts us off
	//Is this just expected?
	int curX = 0;
	//int curX = event->rect().x();
	//int curX = event->rect().x() + geometry().x();
	//FIXME: why do I need this delta not to get cut off?
	//setting viewport margin shifts, but still results in cutoff
	//int curY = event->rect().y();
	//int curY = event->rect().y() + geometry().y();
	//Text painting is at baseline, so "skip" a line
	int curY = fontMetrics().height();
	//printf("curX/Y: %d/%d\n", curX, curY);
	
	//QPainter painter(this);

	//printf("begin start copy\n");
	//wtf why doesn't this use the overloaded oper?
	//seems to work other places 
	//	UVQtDynamicTextData::iterator iter = m_start;
	UVQtDynamicTextData::iterator iter;
	iter.operator=(m_start);
	//printf("copy done, old iter m_impl 0x%08X new 0x%08X\n", (int)m_start.m_impl, (int)iter.m_impl); 
//exit(1);
	for( unsigned int curLines = 0; curLines < getNumberLines(); ++curLines )
	{
		std::string curLine;

		if( iter == m_textData->end() )
		{
			break;
		}
		UV_DEBUG(iter.get(curLine));
		
		painter.drawText(curX, curY, QString(curLine.c_str()));
		curY += fontMetrics().height();
		/*
		eh pointer leak?
		don't want to copy it
		really do want an iter wrapper
		if( iter->equals(end()) )
		{
		}
		*/
		UV_DEBUG(iter.next());
	}
}

uv_err_t UVQtDynamicText::changePositionByLineDelta(int delta)
{
	return UV_DEBUG(m_start.changePositionByLineDelta(delta));
}

uv_err_t UVQtDynamicText::changePositionToLine(unsigned int offset, unsigned int index)
{
	return UV_DEBUG(m_start.changePositionToLine(offset, index));
}

unsigned int UVQtDynamicText::getMinOffset()
{
	unsigned int ret = 0;

	if( m_textData == NULL )
	{
		printf("WARNING: getMinOffset(): m_textData NULL\n");
		return 0;
	}
	UV_DEBUG(m_textData->getMinOffset(&ret));
	return ret;
}

unsigned int UVQtDynamicText::getMaxOffset()
{
	unsigned int ret = 0;
	if( m_textData == NULL )
	{
		printf("WARNING: getMaxOffset(): m_textData NULL\n");
		return 0;
	}
	UV_DEBUG(m_textData->getMaxOffset(&ret));
	return ret;
}


uv_err_t UVQtDynamicText::setDynamicData(UVQtDynamicTextData *data)
{

	//printf("start set, data: 0x%08X\n", (int)data);
	//fflush(stdout);
	uv_assert_ret(data);
	delete m_textData;
	m_textData = data;
	//Reset to top if we swap out the data completly
	UV_DEBUG(m_textData->begin(getMinOffset(), 0, &m_start));
	//printf("set data done\n");
	//UVQtDynamicTextDataPluginImpl::iterator_impl *iter_impl = dynamic_cast<UVQtDynamicTextDataPluginImpl::iterator_impl *>(m_start.m_impl);
	//printf("iter impl set data: 0x%08X\n", (int)iter_impl->m_dataImpl);
	return UV_ERR_OK;
}


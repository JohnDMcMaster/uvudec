/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
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
UVQtDynamicTextData::iterator_impl
*/

UVQtDynamicTextData::iterator_impl::iterator_impl()
{
}

UVQtDynamicTextData::iterator_impl::iterator_impl(unsigned int offset, unsigned int index)
{
	(void)offset;
	(void)index;
}

UVQtDynamicTextData::iterator_impl::~iterator_impl()
{
}

std::string UVQtDynamicTextData::iterator_impl::toString()
{
	return "";
}

/*
iterator *UVQtDynamicTextData::iterator_impl::copy()
{
	return 
}

uv_err_t UVQtDynamicTextData::iterator_impl::get(std::string &ret)
{
	(void)ret;
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVQtDynamicTextData::iterator_impl::next()
{
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVQtDynamicTextData::iterator_impl::changePositionByDelta(int delta)
{
	(void)delta;
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVQtDynamicTextData::iterator_impl::changePositionToAbsolute(unsigned int offset, unsigned int index)
{
	(void)offset;
	(void)index;
	return UV_DEBUG(UV_ERR_GENERAL);
}
*/

/*
UVQtDynamicTextData::iterator
*/

UVQtDynamicTextData::iterator::iterator()
{
	//printf("no impl construction on 0x%08X\n", (int)this);
	m_impl = NULL;
	//UVD_PRINT_STACK();
}

UVQtDynamicTextData::iterator::iterator(iterator_impl *impl)
{
	//printf("setting iterator_impl m_impl: 0x%08X on 0x%08X\n", (int)impl, (int)this);
	m_impl = impl;
}

UVQtDynamicTextData::iterator::~iterator()
{
	delete m_impl;
}

UVQtDynamicTextData::iterator &UVQtDynamicTextData::iterator::operator=(const UVQtDynamicTextData::iterator &source)
{
	//printf("oper called\n");
	//exit(1);

	//printf("assigning new m_impl, old: 0x%08X, template 0x%08X\n", (int)m_impl, (int)source.m_impl);
	delete m_impl;
	m_impl = source.m_impl->copy();
	//printf("new m_impl: 0x%08X\n", (int)m_impl);	
	if( m_impl == NULL )
	{
		printf("iterator_impl m_impl null, boom\n");
		fflush(stdout);
		//UVD_PRINT_STACK();
	}
	
	//	iterator_impl *retImpl = m_impl->copy();
	//printf("ret iter m_impl: 0x%08X\n", (int)retImpl);	
	//	return UVQtDynamicTextData::iterator(retImpl);
	return *this;
}

bool UVQtDynamicTextData::iterator::operator==(const iterator &other)
{
	return compare(&other) == 0;
}

bool UVQtDynamicTextData::iterator::operator!=(const iterator &other)
{
	return compare(&other) != 0;
}

int UVQtDynamicTextData::iterator::compare(const iterator *other)
{
	/*
	if( m_impl == NULL || other.m_impl == NULL )
	{
		return 0;
	}
	*/
	return m_impl->compare(other->m_impl);
}

uv_err_t UVQtDynamicTextData::iterator::get(std::string &ret)
{
	uv_assert_ret(m_impl);
	return UV_DEBUG(m_impl->get(ret));
}

uv_err_t UVQtDynamicTextData::iterator::next()
{
	uv_assert_ret(m_impl);
	return UV_DEBUG(m_impl->next());
}

uv_err_t UVQtDynamicTextData::iterator::changePositionByDelta(int delta)
{
	uv_assert_ret(m_impl);
	return UV_DEBUG(m_impl->changePositionByDelta(delta));
}

uv_err_t UVQtDynamicTextData::iterator::changePositionToAbsolute(unsigned int offset, unsigned int index)
{
	uv_assert_ret(m_impl);
	return UV_DEBUG(m_impl->changePositionToAbsolute(offset, index));
}

unsigned int UVQtDynamicTextData::iterator::offset()
{
	uv_assert_ret(m_impl);
	return UV_DEBUG(m_impl->offset());
}

/*
UVQtDynamicTextData
*/

UVQtDynamicTextData::UVQtDynamicTextData()
{
}

UVQtDynamicTextData::iterator UVQtDynamicTextData::begin(unsigned int offset, unsigned int index)
{
	UVQtDynamicTextData::iterator ret;
	UV_DEBUG(begin(offset, index, &ret));
	return ret;
}

UVQtDynamicTextData::iterator UVQtDynamicTextData::end()
{
	UVQtDynamicTextData::iterator ret;
	UV_DEBUG(end(&ret));
	return ret;
}

/*
UVQtDynamicText
*/

UVQtDynamicText::UVQtDynamicText(UVQtDynamicTextData *textData, QWidget *parent) : QWidget(parent)
{
	QFont font;
	font.setFamily(QString::fromUtf8("Courier [unknown]"));
	setFont(font);

	//m_startOffset = 0;
	//m_startIndex = 0;
	//UV_DEBUG(m_textData->begin(0, 0, &iter));
	//m_numberLines = 10;
	//m_textData = NULL;
	setData(textData);
}

QSize UVQtDynamicText::sizeHint() const 
{
	//Really, we should see if we can impose a QScrollArea for x size, we are mostly concerned with y size right now
	//QSize ret = QSize((7 + m_bytesPerRow * 5 + m_bytesPerRow / m_bytesPerSubRow + 2) * fontMetrics().width('0'),
	QSize ret = QSize(300,
			fontMetrics().height() * getNumberLines());
	printf("UVQtDynamicText::sizeHint() = (width=%d, height=%d)\n", ret.width(), ret.height());
	return ret;
}

/*
UVQtDynamicTextData::iterator & operator=(UVQtDynamicTextData::iterator &dest, const UVQtDynamicTextData::iterator &source)
{
printf("global call\n");
	return dest;
}
*/

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
	printf("paint with %s\n", m_start.m_impl->toString().c_str());
	
printf("pos x: %d, y: %d\n", pos().x(), pos().y());
printf("basic x: %d, y: %d\n", x(), y());
printf("geomtry x: %d, y: %d\n", geometry().x(), geometry().y());
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
	printf("curX/Y: %d/%d\n", curX, curY);
	
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

uv_err_t UVQtDynamicText::changePositionByDelta(int delta)
{
	return UV_DEBUG(m_start.changePositionByDelta(delta));
}

uv_err_t UVQtDynamicText::changePositionToAbsolute(unsigned int offset, unsigned int index)
{
	return UV_DEBUG(m_start.changePositionToAbsolute(offset, index));
}

unsigned int UVQtDynamicText::getMinOffset()
{
	unsigned int ret = 0;
	UV_DEBUG(m_textData->getMinOffset(&ret));
	return ret;
}

unsigned int UVQtDynamicText::getMaxOffset()
{
	unsigned int ret = 0;
	UV_DEBUG(m_textData->getMaxOffset(&ret));
	return ret;
}

/*
UVQtScrollableDynamicText
*/

UVQtScrollableDynamicText::UVQtScrollableDynamicText(QWidget *parent)
{
	m_viewportShadow = NULL;
	m_verticalScrollbarValueShadow = 0;
}
	
UVQtScrollableDynamicText::UVQtScrollableDynamicText(UVQtDynamicTextData *data, QWidget *parent) : QAbstractScrollArea(parent)
{
	setData(data);
}

/*
void UVQtDynamicText::paintEvent(QPaintEvent *event)
{
	printf("UVQtDynamicText::paintEvent()\n");
}

void UVQtDynamicText::resizeEvent(QResizeEvent *event)
{
	printf("UVQtDynamicText::resizeEvent()\n");
}
*/

void UVQtScrollableDynamicText::resizeEvent(QResizeEvent *event)
{
	/*
	Is there any reason to use the resize information in the event vs querying from the widget?
	*/
	printf("UVQtScrollableDynamicText::resizeEvent()\n");
	//Keep 2 common lines per step, or a minimum of 3 lines for it to remain useful
	int pageStep = uvd_max(3, (int)m_viewportShadow->getNumberLines() - 2);
	printf("page step: %d\n", pageStep);
	verticalScrollBar()->setPageStep(pageStep);
}

void UVQtScrollableDynamicText::paintEvent(QPaintEvent *event)
{
	/*
	The event seems to have to be on the viewport
	Creating it on the scrollable widget does not work
	Moreover, the viewport itself doesnt' seem to get paint events
	Why doesn't the widget get the paint events instead then?	
		Also, it seems the geometry is incorrect if we construct the painter in the viewport class
	
	"Note: The y-position is used as the baseline of the font."
	missed that in the documentation, was found out experimentally
	*/

	printf("UVQtScrollableDynamicText::paintEvent()\n");
	//QAbstractScrollArea::paintEvent(event);

	printf("constructing painter\n");
	//Incorrect
	//QPainter painter(this);
  	//Correct
   	QPainter painter(viewport());
	printf("painter made\n");

	UVDQtPrintRect(event->rect());
printf("pos x: %d, y: %d\n", pos().x(), pos().y());
printf("basic x: %d, y: %d\n", x(), y());
printf("geomtry x: %d, y: %d\n", geometry().x(), geometry().y());

	/*
	if( true )
	{
		for( int i = 0; i < 3; ++i )
		{
			painter.drawLine(0, i * 5, 100, i * 5);
		}
		for( int i = 0; i < 3; ++i )
		{
			painter.drawLine(i * 5, 0, i * 5, 100);
		}
	}
	*/

	//Will this clip for us?
	//current tests are only single widget, so hard to tell
	//Also paint even makes this harder to tell
	//QPainter painter(viewport());
	//painter.fillRect(event->rect(), Qt::lightGray);
	
	//Both of the following seem to be required to get this to actually show up
	//However, paintEvent() is going nuts
	//protected, not suppose to do this really
	//m_viewportShadow->doPaintEvent(event);
	//CPU usage spikes if this is called
	//Why does updating the viewport cause a repaint on the parent widget?
	//viewport()->update();
	
	m_viewportShadow->doPaintEvent(event, painter);
	//m_viewportShadow->doPaintEventTest(event);
	printf("paint done\n\n");
}

void UVQtScrollableDynamicText::scrollContentsBy(int dx, int dy)
{
	(void)dx;
	//FIXME: guidelines say not to use dx/dy, but rather directly query from scrollbar
	printf("\nUVQtScrollableDynamicText::scrollContentsBy(dx = %d, dy = %d)\n", dx, dy);
	printf("start scroll at %s\n", m_viewportShadow->m_start.m_impl->toString().c_str());
	//m_viewportShadow->m_startAddress -= dy;
	//UV_DEBUG(m_viewportShadow->changePositionByDelta(-dy));
	int currentAbsoluate = verticalScrollBar()->value();
	printf("currentAbsolute: %d\n", currentAbsoluate);
	//Externally generated event?
	if( m_verticalScrollbarValueShadow != currentAbsoluate )
	{
		UV_DEBUG(m_viewportShadow->changePositionToAbsolute(currentAbsoluate, 0));
		m_verticalScrollbarValueShadow = currentAbsoluate;
	}
	printf("finished scroll at %s\n", m_viewportShadow->m_start.m_impl->toString().c_str());
	//printf("start address: %d\n", m_viewportShadow->m_startAddress);
	//Why doesn't this do it?
	viewport()->update();
}

uv_err_t UVQtDynamicText::setData(UVQtDynamicTextData *data)
{

	//printf("start set, data: 0x%08X\n", (int)data);
	fflush(stdout);
	uv_assert_ret(data);
	m_textData = data;
	//Reset to top if we swap out the data completly
	UV_DEBUG(m_textData->begin(getMinOffset(), 0, &m_start));
	//printf("set data done\n");
	//UVQtDynamicTextDataPluginImpl::iterator_impl *iter_impl = dynamic_cast<UVQtDynamicTextDataPluginImpl::iterator_impl *>(m_start.m_impl);
	//printf("iter impl set data: 0x%08X\n", (int)iter_impl->m_dataImpl);
	return UV_ERR_OK;
}

uv_err_t UVQtScrollableDynamicText::setData(UVQtDynamicTextData *data)
{
	m_viewportShadow = new UVQtDynamicText(data, this);
	UVQtDynamicTextDataPluginImpl::iterator_impl *iter_impl = dynamic_cast<UVQtDynamicTextDataPluginImpl::iterator_impl *>(m_viewportShadow->m_start.m_impl);
	printf("iter impl: 0x%08X\n", (int)iter_impl->m_dataImpl);
	setViewport(m_viewportShadow);
	//m_viewportShadow->resize(320, 240);
	m_viewportShadow->resize(sizeHint());
	//Title bar is blocking top of widget?
	//not an issue once we got our own area
	//setViewportMargins(0, 20, 0, 0);

	//horizontalScrollBar()->setRange(0, 20);
	//horizontalScrollBar()->setPageStep(1);
	//doesn't work because data wasn't set yet
	verticalScrollBar()->setRange(m_viewportShadow->getMinOffset(), m_viewportShadow->getMaxOffset());
	verticalScrollBar()->setPageStep(3);

	m_verticalScrollbarValueShadow = m_viewportShadow->m_start.offset();
	verticalScrollBar()->setSliderPosition(m_viewportShadow->m_start.offset());

	//updateWidgetPosition();
	m_viewportShadow->show();

	return UV_DEBUG(m_viewportShadow->setData(data));
}

uv_err_t UVQtScrollableDynamicText::scrollUnits(int units)
{
	UV_DEBUG(m_viewportShadow->changePositionByDelta(units));
	//We need to update the slider to keep in sync
	//This emits dx/dy events though
	m_verticalScrollbarValueShadow = m_viewportShadow->m_start.offset();
	verticalScrollBar()->setSliderPosition(m_viewportShadow->m_start.offset());
	viewport()->update();
	return UV_ERR_OK;
}

void UVQtScrollableDynamicText::keyPressEvent(QKeyEvent *event)
{
	printf("key event: 0x%08X\n", event->key());
	switch( event->key() )
	{
	case UVQT_KEY_UP:
		printf("UVQT_KEY_UP\n");
		UV_DEBUG(scrollUnits(-1));
		break;
	case UVQT_KEY_DOWN:
		printf("UVQT_KEY_DOWN\n");
		UV_DEBUG(scrollUnits(1));
		break;
	case UVQT_KEY_PAGEUP:
		printf("UVQT_KEY_PAGEUP\n");
		UV_DEBUG(scrollUnits(-verticalScrollBar()->pageStep()));
		break;
	case UVQT_KEY_PAGEDOWN:
		printf("UVQT_KEY_PAGEDOWN\n");
		UV_DEBUG(scrollUnits(verticalScrollBar()->pageStep()));
		break;
	default:
		return;
	}
}


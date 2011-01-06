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
UVQtScrollableDynamicText
*/

UVQtScrollableDynamicText::UVQtScrollableDynamicText(QWidget *parent) : QAbstractScrollArea(parent)
{
	m_viewportShadow = NULL;
	m_verticalScrollbarValueShadow = 0;
	
	UV_DEBUG(initViewport());
}
	
UVQtScrollableDynamicText::UVQtScrollableDynamicText(UVQtDynamicTextData *data, QWidget *parent) : QAbstractScrollArea(parent)
{
	m_viewportShadow = NULL;
	m_verticalScrollbarValueShadow = 0;

	UV_DEBUG(initViewport());
	setDynamicData(data);
}

uv_err_t UVQtScrollableDynamicText::initViewport()
{
	m_viewportShadow = new UVQtDynamicText(this);
	//UV_ASSERT_VOID(m_viewportShadow->m_start.m_impl);
	//UVQtDynamicTextDataPluginImpl::iterator_impl *iter_impl = dynamic_cast<UVQtDynamicTextDataPluginImpl::iterator_impl *>(m_viewportShadow->m_start.m_impl);
	//UV_ASSERT_VOID(iter_impl);
	//printf("iter impl: 0x%08X\n", (int)iter_impl->m_dataImpl);
	setViewport(m_viewportShadow);
	//m_viewportShadow->resize(320, 240);
	m_viewportShadow->resize(sizeHint());
	//Title bar is blocking top of widget?
	//not an issue once we got our own area
	//setViewportMargins(0, 20, 0, 0);

	return UV_ERR_OK;
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

uv_err_t UVQtScrollableDynamicText::refreshVerticalPageStep()
{
	UV_ASSERT_VOID(m_viewportShadow);
	//Keep 2 common lines per step, or a minimum of 3 lines for it to remain useful
	int pageStep = uvd_max(3, (int)m_viewportShadow->getNumberLines() - 2);
	printf("page step: %d\n", pageStep);
	verticalScrollBar()->setPageStep(pageStep);
	return UV_ERR_OK;
}

void UVQtScrollableDynamicText::resizeEvent(QResizeEvent *event)
{
	/*
	Is there any reason to use the resize information in the event vs querying from the widget?
	*/
	printf("UVQtScrollableDynamicText::resizeEvent()\n");
	UV_DEBUG(refreshVerticalPageStep());
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
	//UV_DEBUG(m_viewportShadow->changePositionByLineDelta(-dy));
	int currentAbsoluate = verticalScrollBar()->value();
	printf("currentAbsolute: %d\n", currentAbsoluate);
	//Externally generated event?
	if( m_verticalScrollbarValueShadow != currentAbsoluate )
	{
		UV_DEBUG(m_viewportShadow->changePositionToLine(currentAbsoluate, 0));
		m_verticalScrollbarValueShadow = currentAbsoluate;
	}
	printf("finished scroll at %s\n", m_viewportShadow->m_start.m_impl->toString().c_str());
	//printf("start address: %d\n", m_viewportShadow->m_startAddress);
	//Why doesn't this do it?
	viewport()->update();
}

uv_err_t UVQtScrollableDynamicText::refreshDynamicData()
{
printf("refreshDynamicData()\n");
	//Probably best not to try to display until after propagated
	//horizontalScrollBar()->setRange(0, 20);
	//horizontalScrollBar()->setPageStep(1);
	//doesn't work because data wasn't set yet
	if( m_viewportShadow )
	{
		verticalScrollBar()->setRange(m_viewportShadow->getMinOffset(), m_viewportShadow->getMaxOffset());
		m_verticalScrollbarValueShadow = m_viewportShadow->m_start.offset();
		verticalScrollBar()->setSliderPosition(m_verticalScrollbarValueShadow);
printf("refreshDynamicData(): min: %d, max: %d, pos: %d\n", m_viewportShadow->getMinOffset(), m_viewportShadow->getMaxOffset(), m_verticalScrollbarValueShadow);
	}
	else
	{
printf("refreshDynamicData(): no shadow set\n");
		verticalScrollBar()->setRange(0, 0);
		m_verticalScrollbarValueShadow = 0;
		verticalScrollBar()->setSliderPosition(0);
	}
	
	//printf("%d, %d\n", m_viewportShadow->getMinOffset(), m_viewportShadow->getMaxOffset());
	uv_assert_err_ret(refreshVerticalPageStep());
	//updateWidgetPosition();

	return UV_ERR_OK;
}

uv_err_t UVQtScrollableDynamicText::setDynamicData(UVQtDynamicTextData *data)
{
	//Prepare child, then try to do higher level queries on it now that it should be setup
	uv_assert_err_ret(m_viewportShadow->setDynamicData(data));
	uv_assert_err_ret(refreshDynamicData());
	m_viewportShadow->show();

	return UV_ERR_OK;
}

uv_err_t UVQtScrollableDynamicText::scrollUnits(int units)
{
	UV_DEBUG(m_viewportShadow->changePositionByLineDelta(units));
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


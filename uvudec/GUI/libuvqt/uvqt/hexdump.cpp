#include "uvqt/hexdump.h"
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include "stdio.h"
#include <stdint.h>
#include <math.h>

//using std::cout;

void printRect(QRect rect)
{
	printf("rect: (%d, %d), width: %d, height: %d\n", rect.x(), rect.y(), rect.width(), rect.height());
}

/*
UVQtHexdump
*/

UVQtHexdump::UVQtHexdump(QWidget *parent) : QWidget(parent)
{
	QFont font;
	font.setFamily(QString::fromUtf8("Courier [unknown]"));
	setFont(font);
	m_startAddress = 0;
	//m_data = "***Rensselaer Center for Open Source Software***";
	m_data = "***Rensselaer Center for Open Source Software***It is not the critic who counts; not the man who points out how the strong man stumbles, or where the doer of deeds could have done them better. The credit belongs to the man who is actually in the arena, whose face is marred by dust and sweat and blood; who strives valiantly; who errs, who comes short again and again, because there is no effort without error and shortcoming; but who does actually strive to do the deeds; who knows great enthusiasms, the great devotions; who spends himself in a worthy cause; who at the best knows in the end the triumph of high achievement, and who at the worst, if he fails, at least fails while daring greatly, so that his place shall never be with those cold and timid souls who neither know victory nor defeat.";
	m_bytesPerRow = 16;
	m_bytesPerSubRow = 8;
	m_numberRows = 5;
}

QSize UVQtHexdump::sizeHint() const 
{
	printf("sizeHint()\n");
	//Best way to get width size hint really is to render something
	//Otherwise, maintenance nightmare
	//leading address + hex view + char view + spacer contribution + char view padding
	return QSize((7 + m_bytesPerRow * 5 + m_bytesPerRow / m_bytesPerSubRow + 2) * fontMetrics().width('0'),
			fontMetrics().height() * m_numberRows);
}

unsigned int UVQtHexdump::getNumberLines()
{
	return ceil(1.0 * m_data.size() / m_bytesPerRow);
}

void UVQtHexdump::doPaintEvent(QPaintEvent *event)
{
	printf("***UVQtHexdump::doPaintEvent()\n");

	const uint8_t *data = (const uint8_t *)m_data.c_str();
	size_t size = m_data.size();

	/*
	[mcmaster@gespenst icd2prog-0.3.0]$ hexdump -C /bin/ls |head
	00000000  7f 45 4c 46 01 01 01 00  00 00 00 00 00 00 00 00  |.ELF............|
	00000010  02 00 03 00 01 00 00 00  f0 99 04 08 34 00 00 00  |............4...|
	00017380  00 00 00 00 01 00 00 00  00 00 00 00			    |............    |
	*/

	//It paints the entire window, but the title bar cuts us off
	//Is this just expected?
	int curX = event->rect().x();
	//FIXME: why do I need this delta not to get cut off?
	//setting viewport margin shifts, but still results in cutoff
	int curY = event->rect().y() + 20;
	
	std::string fillerChar = " ";

	QPainter painter(this);

	size_t pos = m_bytesPerRow * m_startAddress;
	unsigned int curLines = 0;
	while( pos < size && curLines < m_numberRows )
	{
		std::string curLine;
		uint32_t row_start = pos;
		uint32_t i = 0;
		char buff[16];

		snprintf(buff, sizeof(buff), "%04X:  ", pos);
		curLine += buff;

		for( unsigned int curRow = 0; curRow < m_bytesPerRow; curRow += m_bytesPerSubRow )
		{
			pos = hexdumpHalfRow(data, size, pos, curLine);
		}
		
		curLine += "|";

		//Char view
		for( i = row_start; i < row_start + m_bytesPerRow && i < size; ++i )
		{
			char c = data[i];
			if( isprint(c) )
			{
				curLine += c;
			}
			else
			{
				curLine += '.';
			}

		} 
		for( ; i < row_start + m_bytesPerRow; ++i )
		{
			curLine += fillerChar;
		}

		curLine += "|";
		
		painter.drawText(curX, curY, QString(curLine.c_str()));
		curY += fontMetrics().height();
		++curLines;
	}
}

unsigned int UVQtHexdump::hexdumpHalfRow(const uint8_t *data, size_t size, uint32_t start, std::string &ret)
{
	uint32_t col = 0;
	char buff[4];

	for( ; col < m_bytesPerSubRow && start + col < size; ++col )
	{
		uint32_t index = start + col;
		uint8_t c = data[index];
		
		snprintf(buff, sizeof(buff), " %02X ", (unsigned int)c);
		ret += buff;
	}

	//pad remaining
	while( col < m_bytesPerSubRow )
	{
		ret += "   ";
		++col;
	}

	//End pad
	ret += " ";

	return start + m_bytesPerSubRow;
}

/*
UVQtScrollableHexdump
*/

UVQtScrollableHexdump::UVQtScrollableHexdump(QWidget *parent) : QAbstractScrollArea(parent)
{
	m_viewportShadow = new UVQtHexdump(this);
	setViewport(m_viewportShadow);
	m_viewportShadow->resize(320, 240);
	//Title bar is blocking top of widget?
	setViewportMargins(0, 20, 0, 0);
	m_viewportShadow->show();

	verticalScrollBar()->setPageStep(1);
	horizontalScrollBar()->setPageStep(1);
	verticalScrollBar()->setRange(0, m_viewportShadow->getNumberLines() - m_viewportShadow->m_numberRows);
	verticalScrollBar()->setPageStep(3);
	//horizontalScrollBar()->setRange(0, 20);
	//updateWidgetPosition();
}

void UVQtScrollableHexdump::paintEvent(QPaintEvent *event)
{
	printf("UVQtScrollableHexdump::paintEvent()\n");
    QAbstractScrollArea::paintEvent(event);

	printRect(event->rect());
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
	m_viewportShadow->doPaintEvent(event);
}

void UVQtScrollableHexdump::scrollContentsBy(int dx, int dy)
{
	//FIXME: guidelines say not to use dx/dy, but rather directly query from scrollbar
	printf("\nUVQtScrollableHexdump::scrollContentsBy(dx = %d, dy = %d)\n", dx, dy);
	m_viewportShadow->m_startAddress -= dy;
	printf("start address: %d\n", m_viewportShadow->m_startAddress);
	//Why doesn't this do it?
	viewport()->update();
}


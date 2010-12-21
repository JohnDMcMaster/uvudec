#ifndef SIMPLE_H
#define SIMPLE_H

#include <QWidget>
#include <QtDesigner/QDesignerExportWidget>
#include <QAbstractScrollArea>
#include <string>
#include <stdint.h>

class UVQtHexdump : public QWidget
{
    Q_OBJECT

public:
	UVQtHexdump(QWidget *parent = NULL);
	QSize sizeHint() const;
	
public:
	unsigned int getNumberLines();
	//I'm not clear why this doesn't get paint events, but Okteta seems to do the same thing (ColumnsView)
	//I put render code in UVQtHexdump, they have it in the scroll widget
	void doPaintEvent(QPaintEvent *event);

protected:
	unsigned int hexdumpHalfRow(const uint8_t *data, size_t size, uint32_t start, std::string &ret);
	void hexdump(const uint8_t *data, size_t size);

public:
	unsigned int m_startAddress;
	std::string m_data;
	unsigned int m_bytesPerRow;
	unsigned int m_bytesPerSubRow;
	unsigned int m_numberRows;
};

/*
When inheriting QAbstractScrollArea, you need to do the following:
    * Control the scroll bars by setting their range, value, page step, and tracking their movements.
    * Draw the contents of the area in the viewport according to the values of the scroll bars.
    * Handle events received by the viewport in viewportEvent() - notably resize events.
    * Use viewport->update() to update the contents of the viewport instead of update() as all painting operations take place on the viewport.
*/

class QDESIGNER_WIDGET_EXPORT UVQtScrollableHexdump : public QAbstractScrollArea
{
    Q_OBJECT

public:
	UVQtScrollableHexdump(QWidget *parent = NULL);

protected:
	void paintEvent(QPaintEvent *event);
	void scrollContentsBy(int dx, int dy);

public:
	UVQtHexdump *m_viewportShadow;
};

#endif


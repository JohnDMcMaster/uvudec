/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the GPL V3 or later, see COPYING for details
*/

/*
Functionality among the lines of what could be QAbstractPlainTextEdit

Like QPlainTextEdit, but rows are dynamically generated
Original use is to take care of disassembly listings, which have some unusual indexing behavior
Scroll bar is to be indexed by address, but incremental changes are by line
This takes care of difficulty of knowing exact index with intermediate data taken into account

If an index is no longer availible, should roll back to take the first availible data
Roughly modeling this off of MVC so I can port it to it as I become more comfterable
with the basic widgets
Ex:
	Original
		0:0 # Refered by X
		0:1 # Refereced by Y 
		0:2 MOV EAX, 3
		1:0 # Refered by X
	->	1:1 # Refereced by Y 
		1:2 SUB ESP, 8
	Say top of scroll is set to 1:1,
	but SUB ESP, 8 is no longer referenced by Y
	New top should be 1:0
*/

#ifndef UVQT_DYNAMIC_TEXT_H
#define UVQT_DYNAMIC_TEXT_H

#include <QWidget>
#include <QtDesigner/QDesignerExportWidget>
#include <QAbstractScrollArea>
#include <string>
#include <stdint.h>
#include "uvd/util/types.h"

/*
Maybe should implement an iterator?
Subclass this to implement render
UVDIterator doesn't have previous(), so try not to rely on any prevoius operation
Doesn't have begin(address, index), (only begin(addresS)), 
but can be implemented without too much trouble

Also, it might make sense to allow offset to be negative
I want 32 bit address ranges though, so I want easy 32 bit access over neg value possibility
*/
class UVQtDynamicTextData
{
public:
	/*
	Try to auto_ptr wrap this maybe?
	Tried void *m_private before, but poor results
		Got annoyed because of all of the shadow()->m_data, casts, etc
	auto_ptr<T> isn't really right either
	Really what was going to happen is was going to have to wrap ref count, but thats already done for us
	*/
	class iterator_impl
	{
	public:
		iterator_impl();
		iterator_impl(unsigned int offset, unsigned int index);
		virtual ~iterator_impl();
	
		virtual unsigned int offset() = 0;
		virtual int compare(const iterator_impl *other) = 0;
		virtual iterator_impl *copy() = 0;
		virtual uv_err_t get(std::string &ret) = 0;
		//If this would underflow or overflow, set to max possible value
		//What about making sure we have at least one page viewable?
		//we would not want to allow advancing to the very end
		virtual uv_err_t next() = 0;
		//Try to process in the scroll class somehow
		virtual uv_err_t changePositionByDelta(int delta) = 0;
		//Set the position absolutly
		virtual uv_err_t changePositionToAbsolute(unsigned int offset, unsigned int index) = 0;
		//For debugging
		virtual std::string toString();
	public:
	};
	//class cap differtiation is kinda lame, but w/e
	//typedef auto_ptr<iterator> Iterator;

	class iterator
	{
	public:
		iterator();
		iterator(iterator_impl *impl);
		~iterator();
	
		iterator &operator=(const iterator &source);
		bool operator ==(const iterator &other);
		bool operator !=(const iterator &other);
		int compare(const iterator *other);

		unsigned int offset();
		uv_err_t get(std::string &ret);
		uv_err_t next();
		//If this would underflow or overflow, set to max possible value
		//What about making sure we have at least one page viewable?
		//we would not want to allow advancing to the very end
		//Try to process in the scroll class somehow
		uv_err_t changePositionByDelta(int delta);
		//Set the position absolutly
		uv_err_t changePositionToAbsolute(unsigned int offset, unsigned int index);

	public:
		iterator_impl *m_impl;
	};
	
public:
	UVQtDynamicTextData();
	
	iterator begin(unsigned int offset, unsigned int index);
	virtual uv_err_t begin(unsigned int offset, unsigned int index, iterator *out) = 0;
	iterator end();
	virtual uv_err_t end(iterator *out) = 0;
	virtual uv_err_t getMinOffset(unsigned int *out) = 0;
	virtual uv_err_t getMaxOffset(unsigned int *out) = 0;
};

/*
A text edit widget
*/
class UVQtDynamicText : public QWidget
{
    Q_OBJECT

public:
	UVQtDynamicText(UVQtDynamicTextData *data, QWidget *parent = NULL);
	QSize sizeHint() const;

	//For scrollbar positions
	unsigned int getMinOffset();
	unsigned int getMaxOffset();
	
	unsigned int getNumberLines() const;
	void doPaintEvent(QPaintEvent *event, QPainter &painter);
	void doPaintEventTest(QPaintEvent *event);
	virtual uv_err_t changePositionByDelta(int delta);
	virtual uv_err_t changePositionToAbsolute(unsigned int offset, unsigned int index);

	uv_err_t setData(UVQtDynamicTextData *data);

protected:
	/*
	//A test, never gets called
	void paintEvent(QPaintEvent *event);
	//this either
	void resizeEvent(QResizeEvent *event);
	*/
	
public:
	//Primary position
	//unsigned int m_startOffset;
	//Secondary position
	//unsigned int m_startIndex;
	UVQtDynamicTextData::iterator m_start;
	
	//How many lines we should display
	//unsigned int m_numberLines;
	//For fetching actual data
	UVQtDynamicTextData *m_textData;
};

/*
When inheriting QAbstractScrollArea, you need to do the following:
    * Control the scroll bars by setting their range, value, page step, and tracking their movements.
    * Draw the contents of the area in the viewport according to the values of the scroll bars.
    * Handle events received by the viewport in viewportEvent() - notably resize events.
    * Use viewport->update() to update the contents of the viewport instead of update() as all painting operations take place on the viewport.
*/

class QDESIGNER_WIDGET_EXPORT UVQtScrollableDynamicText : public QAbstractScrollArea
{
    Q_OBJECT

public:
	//QtDesigner requires this form for .ui generation
	UVQtScrollableDynamicText(QWidget *parent = NULL);
	UVQtScrollableDynamicText(UVQtDynamicTextData *data, QWidget *parent = NULL);
	uv_err_t setData(UVQtDynamicTextData *data);
	uv_err_t scrollUnits(int units);

protected:
	void paintEvent(QPaintEvent *event);
	//Seems this gets called when the program starts up
	//Is that reliable?
	void resizeEvent(QResizeEvent *event);
	void scrollContentsBy(int dx, int dy);
	void keyPressEvent(QKeyEvent *event);

public:
	UVQtDynamicText *m_viewportShadow;
	/*
	This is needed to differentiate incremental adjustments versus those set by the slider
	*/
	int m_verticalScrollbarValueShadow;
};

#endif


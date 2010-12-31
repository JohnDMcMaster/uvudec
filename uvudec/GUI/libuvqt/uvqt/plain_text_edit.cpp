/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvqt/plain_text_edit.h"
#include <QTextBlock>
#include <stdio.h>

/*
looking at Qt doc, only has a few implemented virtual funcs:
	virtual void 	clear ()
	virtual QTextObject * 	createObject ( const QTextFormat & format )
	virtual QVariant 	loadResource ( int type, const QUrl & name )
so it doesn't seem like it can be customized
*/
class UVQtTextDocument: public QTextDocument
{
public:
	UVQtTextDocument();
	QString toHtml ( const QByteArray & encoding = QByteArray() ) const;
	QString toPlainText () const;
    QTextBlock findBlockByNumber(int blockNumber) const;
	
public:
	QTextBlock begin() const;
};

UVQtTextDocument::UVQtTextDocument()
{
	printf("UVQtTextDocument() => 0x%08X\n", (int)this);
}

QString UVQtTextDocument::toHtml ( const QByteArray & encoding ) const
{
	(void)encoding;
	printf("toHtml\n");
	return "";
}

QString UVQtTextDocument::toPlainText () const
{
	printf("toPlainText\n");
	return "";
}

QTextBlock UVQtTextDocument::begin() const
{
	printf("UVQtTextDocument::begin()\n");
	QTextBlock block;
	return block;
}

QTextBlock UVQtTextDocument::findBlockByNumber(int blockNumber) const
{
	printf("UVQtTextDocument::findBlockByNumber(%d)\n", blockNumber);
	QTextBlock block;
	return block;
}

/*
UVQtPlainTextEdit
*/

UVQtPlainTextEdit::UVQtPlainTextEdit()
{
	QTextDocument *document = NULL;
	
	//document = new QTextDocument();
	document = new UVQtTextDocument();
	
	setDocument(document);
}

/*
UVQtTextEdit
*/

UVQtTextEdit::UVQtTextEdit()
{
	QTextDocument *document = NULL;
	
	//document = new QTextDocument();
	document = new UVQtTextDocument();
	
	setDocument(document);
}


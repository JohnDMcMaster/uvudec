/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVQT_PLAIN_TEXT_EDIT_H
#define UVQT_PLAIN_TEXT_EDIT_H

#include <QTextEdit>
#include <QPlainTextEdit>

class UVQtPlainTextEdit : public QPlainTextEdit
{
public:
	UVQtPlainTextEdit();
};

class UVQtTextEdit : public QTextEdit
{
public:
	UVQtTextEdit();
};

#endif


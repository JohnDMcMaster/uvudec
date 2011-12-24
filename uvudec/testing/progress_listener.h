/*
UVNet Universal Decompiler(uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef PROGRESS_LISTENER_H
#define PROGRESS_LISTENER_H

#include <cppunit/TestListener.h>
#include <string>

class UVTestProgressListener : public CppUnit::TestListener
{
public:
	UVTestProgressListener();

	virtual ~UVTestProgressListener();

	void startTest( CppUnit::Test *test );
	void addFailure( const CppUnit::TestFailure &failure );
	void endTest( CppUnit::Test *test );

private:
	UVTestProgressListener( const UVTestProgressListener &copy );
	void operator =( const UVTestProgressListener &copy );

private:
	bool m_lastTestFailed;
	std::string m_testName;
};

#endif


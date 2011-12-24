/*
UVNet Universal Decompiler(uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "testing/progress_listener.h"
#include "uvd/util/curses.h"
#include "uvd/util/util.h"
#include <stdio.h>
#include <cppunit/Exception.h>
#include <cppunit/SourceLine.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>


UVTestProgressListener::UVTestProgressListener()
    : m_lastTestFailed( false )
{
}


UVTestProgressListener::~UVTestProgressListener()
{
}

#define UntCurse(_string, _color) \
	UVDCurse(_string, _color, false, true)

#define UntInfo(_format, ...) \
	printf("UNT %s: " _format "\n", UntCurse("INFO", UVD_CURSE_BLUE).c_str(), ## __VA_ARGS__)
#define UntError(_format, ...) \
	printf("UNT %s: " _format "\n", UntCurse("ERROR", UVD_CURSE_RED).c_str(), ## __VA_ARGS__)

void UVTestProgressListener::startTest( CppUnit::Test *test )
{
	printf("\n");
	UntInfo("Starting %s", test->getName().c_str());
	fflush(stdout);

	m_lastTestFailed = false;
	m_testName = test->getName();
}

void UVTestProgressListener::addFailure( const CppUnit::TestFailure &failure )
{
	CppUnit::Exception *e = failure.thrownException();
	CppUnit::SourceLine sl = e->sourceLine();
	std::string ss = "unknown location";
	if (sl.isValid()) {
		ss = UVDSprintf("%s:%d", sl.fileName().c_str(), sl.lineNumber());
	}
	UntError("Failed %s by %s @ %s",
			m_testName.c_str(), 
			failure.isError() ? "error" : "assertion", 
			ss.c_str());
	printf("%s", e->what());
	m_lastTestFailed  = true;
}


void UVTestProgressListener::endTest( CppUnit::Test *test )
{
	if ( !m_lastTestFailed ) {
		UntInfo("%s OK", m_testName.c_str());
	}
}


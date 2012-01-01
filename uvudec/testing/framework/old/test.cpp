#include <cppunit/Portability.h>
#include "testing/framework/test.h"
#include <cppunit/TestPath.h>
#include <stdexcept>
#include <stdio.h>

UVTest::UVTest( CppUnit::Test *test )
	: m_test(test)
{
}

void
UVTest::run( CppUnit::TestResult *result )
{
/*
Heres the fun part
The child will get the output in the TestResult object but it will need to be serialized to be put into a new TestResult object
The output TestResult object will then be throw away as the process exits

Really what we are interfering with is (at least for my purposes)
TestComposite::run( TestResult *result )
	doStartSuite( result );
	doRunChildTests( result );
	doEndSuite( result );
In order for this to work we have to replace the TestSuite object with this proxy object
*/
printf("Run wrapper\n");
	return m_test->run(result);
}

int 
UVTest::countTestCases () const
{
	return m_test->countTestCases();
}

int 
UVTest::getChildTestCount() const
{
	return m_test->getChildTestCount();
}

CppUnit::Test *
UVTest::getChildTestAt( int index ) const
{
	return m_test->getChildTestAt( index );
}

std::string 
UVTest::getName () const
{
	return m_test->getName();
}

CppUnit::Test *
UVTest::findTest( const std::string &testName ) const
{
	return m_test->findTest( testName );
}

bool 
UVTest::findTestPath( const std::string &testName,
                    CppUnit::TestPath &testPath ) const
{
	return m_test->findTestPath( testName, testPath );
}

bool 
UVTest::findTestPath( const Test *test,
                    CppUnit::TestPath &testPath ) const
{
	return m_test->findTestPath( test, testPath );
}

CppUnit::TestPath 
UVTest::resolveTestPath( const std::string &testPath ) const
{
	return m_test->resolveTestPath( testPath );
}


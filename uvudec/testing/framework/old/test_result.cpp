#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestListener.h>
#include <cppunit/tools/Algorithm.h>
#include <algorithm>
#include <stdio.h>

#include "testing/framework/test_result.h"


UVTestResult::UVTestResult( SynchronizationObject *syncObject )
    : TestResult( syncObject )
    , m_stop( false )
{ 
printf("making UVTestResult(), 0x%p\n", this);
m_parentPid = -1;
}


UVTestResult::~UVTestResult()
{
}


void 
UVTestResult::reset()
{
  ExclusiveZone zone( m_syncObject ); 
  m_stop = false;
}


void 
UVTestResult::addError( CppUnit::Test *test, 
                      CppUnit::Exception *e )
{ 
printf("adding error\n");
  CppUnit::TestFailure failure( test, e, true );
  addFailure( failure );
}


void 
UVTestResult::addFailure( CppUnit::Test *test, CppUnit::Exception *e )
{ 
printf("adding failure1 from test %p from expection %p\n", test, e);
  CppUnit::TestFailure failure( test, e, false );
  addFailure( failure );
}


void 
UVTestResult::addFailure( const CppUnit::TestFailure &failure )
{
printf("adding failure2\n");
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->addFailure( failure );
}


void 
UVTestResult::startTest( CppUnit::Test *test )
{ 
/*
Called once for each individual test
Therefore its our job to fork here?

(gdb) info stack
#0  UVTestResult::startTest (this=0x808d0b8, test=0x808a6e0) at ../testing/framework/test_result.cpp:84
#1  0x01122c87 in CppUnit::TestCase::run (this=0x808a6e0, result=0x808d0b8) at TestCase.cpp:55
	result->startTest(this);
#2  0x01123586 in CppUnit::TestComposite::doRunChildTests (this=0x808a5a8, controller=0x808d0b8) at TestComposite.cpp:64
#3  0x0112349b in CppUnit::TestComposite::run (this=0x808a5a8, result=0x808d0b8) at TestComposite.cpp:23
#4  0x0112cb3c in CppUnit::TestRunner::WrappingSuite::run (this=0x808d0a0, result=0x808d0b8) at TestRunner.cpp:47
#5  0x08054c14 in UVTestResult::runTest (this=0x808d0b8, test=0x808d0a0) at ../testing/framework/test_result.cpp:257
#6  0x0112c952 in CppUnit::TestRunner::run (this=0xbfffe508, controller=..., testPath="") at TestRunner.cpp:96
#7  0x08052198 in UVTextTestRunner::run (this=0xbfffe508, controller=..., testPath="") at ../testing/framework/text_test_runner.cpp:147
#8  0x08051fdb in UVTextTestRunner::run (this=0xbfffe508, testName="", doWait=false, doPrintResult=true, doPrintProgress=false)
    at ../testing/framework/text_test_runner.cpp:68
#9  0x08060e28 in main (argc=3, argv=0xbfffe644) at ../testing/main.cpp:252
*/
printf("start test\n");
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->startTest( test );
}

  
void 
UVTestResult::endTest( CppUnit::Test *test )
{ 
printf("end test\n");
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->endTest( test );
}


void 
UVTestResult::startSuite( CppUnit::Test *test )
{
printf("start suite\n");
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->startSuite( test );
}


void 
UVTestResult::endSuite( CppUnit::Test *test )
{
printf("end suite\n");
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->endSuite( test );
}


bool 
UVTestResult::shouldStop() const
{ 
printf("should stop\n");
  ExclusiveZone zone( m_syncObject );
  return m_stop; 
}


void 
UVTestResult::stop()
{ 
printf("stop\n");
  ExclusiveZone zone( m_syncObject );
  m_stop = true; 
}


void 
UVTestResult::addListener( CppUnit::TestListener *listener )
{
printf("add list\n");
  ExclusiveZone zone( m_syncObject ); 
  m_listeners.push_back( listener );
}


void 
UVTestResult::removeListener ( CppUnit::TestListener *listener )
{
printf("remove list\n");
  ExclusiveZone zone( m_syncObject ); 
  removeFromSequence( m_listeners, listener );
}


void 
UVTestResult::runTest( CppUnit::Test *test )
{

/*
The problem is that a test can be a suite or a single test
Lower level logic decomposes this into individual tests, but it doesn't get filtered back up here
	CppUnit::TestComposite::doRunChildTests() handles this
*/
printf("***runTest() start\n");
  startTestRun( test );
  test->run( this );
  endTestRun( test );
printf("**runTest() end\n");
}

#include <unistd.h>

void 
UVTestResult::startTestRun( CppUnit::Test *test )
{
/*
Only called once even for a collection of tests
*/
printf("***startTestRun() start\n");
m_parentPid = getpid();
printf("PID: %d\n", m_parentPid);
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->startTestRun( test, this );
printf("**startTestRun() end\n");
}


void 
UVTestResult::endTestRun( CppUnit::Test *test )
{
printf("***endTestRun() start\n");
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->endTestRun( test, this );
printf("**endTestRun() end\n");
}


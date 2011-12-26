#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestListener.h>
#include <cppunit/tools/Algorithm.h>
#include <algorithm>

#include "testing/framework/test_result.h"


UVTestResult::UVTestResult( SynchronizationObject *syncObject )
    : TestResult( syncObject )
    , m_stop( false )
{ 
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
  CppUnit::TestFailure failure( test, e, true );
  addFailure( failure );
}


void 
UVTestResult::addFailure( CppUnit::Test *test, CppUnit::Exception *e )
{ 
  CppUnit::TestFailure failure( test, e, false );
  addFailure( failure );
}


void 
UVTestResult::addFailure( const CppUnit::TestFailure &failure )
{
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->addFailure( failure );
}


void 
UVTestResult::startTest( CppUnit::Test *test )
{ 
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->startTest( test );
}

  
void 
UVTestResult::endTest( CppUnit::Test *test )
{ 
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->endTest( test );
}


void 
UVTestResult::startSuite( CppUnit::Test *test )
{
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->startSuite( test );
}


void 
UVTestResult::endSuite( CppUnit::Test *test )
{
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->endSuite( test );
}


bool 
UVTestResult::shouldStop() const
{ 
  ExclusiveZone zone( m_syncObject );
  return m_stop; 
}


void 
UVTestResult::stop()
{ 
  ExclusiveZone zone( m_syncObject );
  m_stop = true; 
}


void 
UVTestResult::addListener( CppUnit::TestListener *listener )
{
  ExclusiveZone zone( m_syncObject ); 
  m_listeners.push_back( listener );
}


void 
UVTestResult::removeListener ( CppUnit::TestListener *listener )
{
  ExclusiveZone zone( m_syncObject ); 
  removeFromSequence( m_listeners, listener );
}


void 
UVTestResult::runTest( CppUnit::Test *test )
{
  startTestRun( test );
  test->run( this );
  endTestRun( test );
}


void 
UVTestResult::startTestRun( CppUnit::Test *test )
{
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->startTestRun( test, this );
}


void 
UVTestResult::endTestRun( CppUnit::Test *test )
{
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->endTestRun( test, this );
}


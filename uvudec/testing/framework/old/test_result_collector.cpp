#include <cppunit/TestFailure.h>
#include "testing/framework/test_result_collector.h"
#include <stdio.h>


UVTestResultCollector::UVTestResultCollector( SynchronizationObject *syncObject )
    : TestSuccessListener( syncObject )
{
  reset();
}


UVTestResultCollector::~UVTestResultCollector()
{
  freeFailures();
}


void 
UVTestResultCollector::freeFailures()
{
  TestFailures::iterator itFailure = m_failures.begin();
  while ( itFailure != m_failures.end() )
    delete *itFailure++;
  m_failures.clear();
}


void 
UVTestResultCollector::reset()
{
  TestSuccessListener::reset();

  ExclusiveZone zone( m_syncObject ); 
  freeFailures();
  m_testErrors = 0;
  m_tests.clear();
}


void 
UVTestResultCollector::startTest( CppUnit::Test *test )
{
printf("Starting test %p\n", test);
  ExclusiveZone zone (m_syncObject); 
  m_tests.push_back( test );
}


void 
UVTestResultCollector::addFailure( const CppUnit::TestFailure &failure )
{
printf("test result collector: add failure\n");
  CppUnit::TestSuccessListener::addFailure( failure );

  ExclusiveZone zone( m_syncObject ); 
  if ( failure.isError() )
    ++m_testErrors;
  m_failures.push_back( failure.clone() );
}


/// Gets the number of run tests.
int 
UVTestResultCollector::runTests() const
{ 
printf("test result collector: run tests: %d\n", m_tests.size());
  ExclusiveZone zone( m_syncObject ); 
  return m_tests.size(); 
}


/// Gets the number of detected errors (uncaught exception).
int 
UVTestResultCollector::testErrors() const
{ 
printf("test result collector: test errors\n");
  ExclusiveZone zone( m_syncObject );
  return m_testErrors;
}


/// Gets the number of detected failures (failed assertion).
int 
UVTestResultCollector::testFailures() const
{ 
printf("test result collector: test failures\n");
  ExclusiveZone zone( m_syncObject ); 
  return m_failures.size() - m_testErrors;
}


/// Gets the total number of detected failures.
int 
UVTestResultCollector::testFailuresTotal() const
{
printf("test result collector: test failures total\n");
  ExclusiveZone zone( m_syncObject ); 
  return m_failures.size();
}


/// Returns a the list failures (random access collection).
const UVTestResultCollector::TestFailures & 
UVTestResultCollector::failures() const
{ 
printf("test result collector: failures()\n");
  ExclusiveZone zone( m_syncObject );
  return m_failures; 
}


const UVTestResultCollector::Tests &
UVTestResultCollector::tests() const
{
printf("test result collector: tests()\n");
  ExclusiveZone zone( m_syncObject );
  return m_tests;
}


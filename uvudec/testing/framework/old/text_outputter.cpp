#include <cppunit/Exception.h>
#include <cppunit/SourceLine.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResultCollector.h>
#include "testing/framework/text_outputter.h"
#include "testing/framework/test_result_collector.h"
#include <stdio.h>

UVTextOutputter::UVTextOutputter( TestResultCollector *result,
                              CppUnit::OStream &stream )
    : m_result( result )
    , m_stream( stream )
{
}


UVTextOutputter::~UVTextOutputter()
{
}


void 
UVTextOutputter::write() 
{
  printHeader();
  m_stream << "\n";
  printFailures();
  m_stream << "\n";
}


void 
UVTextOutputter::printFailures()
{
  TestResultCollector::TestFailures::const_iterator itFailure = m_result->failures().begin();
  int failureNumber = 1;
  while ( itFailure != m_result->failures().end() ) 
  {
    m_stream  <<  "\n";
    printFailure( *itFailure++, failureNumber++ );
  }
}


void 
UVTextOutputter::printFailure( CppUnit::TestFailure *failure,
                             int failureNumber )
{
  printFailureListMark( failureNumber );
  m_stream << ' ';
  printFailureTestName( failure );
  m_stream << ' ';
  printFailureType( failure );
  m_stream << ' ';
  printFailureLocation( failure->sourceLine() );
  m_stream << "\n";
  printFailureDetail( failure->thrownException() );
  m_stream << "\n";
}


void 
UVTextOutputter::printFailureListMark( int failureNumber )
{
  m_stream << failureNumber << ")";
}


void 
UVTextOutputter::printFailureTestName( CppUnit::TestFailure *failure )
{
  m_stream << "test: " << failure->failedTestName();
}


void 
UVTextOutputter::printFailureType( CppUnit::TestFailure *failure )
{
  m_stream << "("
           << (failure->isError() ? "E" : "F")
           << ")";
}


void 
UVTextOutputter::printFailureLocation( CppUnit::SourceLine sourceLine )
{
  if ( !sourceLine.isValid() )
    return;

  m_stream << "line: " << sourceLine.lineNumber()
           << ' ' << sourceLine.fileName();
}


void 
UVTextOutputter::printFailureDetail( CppUnit::Exception *thrownException )
{
  m_stream  <<  thrownException->message().shortDescription()  <<  "\n";
  m_stream  <<  thrownException->message().details();
}


void 
UVTextOutputter::printHeader()
{
  if ( m_result->wasSuccessful() )
    m_stream << "\nOK (" << m_result->runTests () << " tests)\n" ;
  else
  {
    m_stream << "\n";
    printFailureWarning();
    printStatistics();
  }
}


void 
UVTextOutputter::printFailureWarning()
{
  m_stream  << "!!!FAILURES!!!\n";
}


void 
UVTextOutputter::printStatistics()
{
printf("print stat\n");
  m_stream  << "Test Results:\n";

  m_stream  <<  "Run:  "  <<  m_result->runTests()
            <<  "   Failures: "  <<  m_result->testFailures()
            <<  "   Errors: "  <<  m_result->testErrors()
            <<  "\n";
}




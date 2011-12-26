#ifndef UVCPPUNIT_UI_TEXT_TEXTTESTRUNNER_H
#define UVCPPUNIT_UI_TEXT_TEXTTESTRUNNER_H


#include <cppunit/Portability.h>
#include <string>
#include <cppunit/TestRunner.h>
#include <cppunit/Outputter.h>
#include "testing/framework/test_result_collector.h"


/*!
 * \brief A text mode test runner.
 * \ingroup WritingTestResult
 * \ingroup ExecutingTest
 *
 * The test runner manage the life cycle of the added tests.
 *
 * The test runner can run only one of the added tests or all the tests. 
 *
 * TestRunner prints out a trace as the tests are executed followed by a
 * summary at the end. The trace and summary print are optional.
 *
 * Here is an example of use:
 *
 * \code
 * CppUnit::TextTestRunner runner;
 * runner.addTest( ExampleTestCase::suite() );
 * runner.run( "", true );    // Run all tests and wait
 * \endcode
 *
 * The trace is printed using a TextTestProgressListener. The summary is printed
 * using a TextOutputter. 
 *
 * You can specify an alternate Outputter at construction
 * or later with setOutputter(). 
 *
 * After construction, you can register additional TestListener to eventManager(),
 * for a custom progress trace, for example.
 *
 * \code
 * CppUnit::TextTestRunner runner;
 * runner.addTest( ExampleTestCase::suite() );
 * runner.setOutputter( CppUnit::CompilerOutputter::defaultOutputter( 
 *                          &runner.result(),
 *                          std::cerr ) );
 * MyCustomProgressTestListener progress;
 * runner.eventManager().addListener( &progress );
 * runner.run( "", true );    // Run all tests and wait
 * \endcode
 *
 * \see CompilerOutputter, XmlOutputter, TextOutputter.
 */
class UVTextTestRunner : public CPPUNIT_NS::TestRunner
{
public:
  UVTextTestRunner( CPPUNIT_NS::Outputter *outputter =NULL );

  virtual ~UVTextTestRunner();

  bool run( std::string testPath ="",
            bool doWait = false,
            bool doPrintResult = true,
            bool doPrintProgress = true );

  void setOutputter( CPPUNIT_NS::Outputter *outputter );

  UVTestResultCollector &result() const;

  CPPUNIT_NS::TestResult &eventManager() const;

public: // overridden from TestRunner (to avoid hidden virtual function warning)
  virtual void run( CPPUNIT_NS::TestResult &controller,
                    const std::string &testPath = "" );

protected:
  virtual void wait( bool doWait );
  virtual void printResult( bool doPrintResult );

  UVTestResultCollector *m_result;
  CppUnit::TestResult *m_eventManager;
  CppUnit::Outputter *m_outputter;
};


#endif  // CPPUNIT_UI_TEXT_TEXTTESTRUNNER_H


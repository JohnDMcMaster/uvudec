// ==> Implementation of cppunit/ui/text/TestRunner.h

#include <cppunit/config/SourcePrefix.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TextTestResult.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/TestResult.h>
#include <cppunit/portability/Stream.h>
#include <stdexcept>
#include "testing/framework/text_test_runner.h"



/*! Constructs a new text runner.
 * \param outputter used to print text result. Owned by the runner.
 */
UVTextTestRunner::UVTextTestRunner( CPPUNIT_NS::Outputter *outputter ) 
    : m_result( new CppUnit::TestResultCollector() )
    , m_eventManager( new CppUnit::TestResult() )
    , m_outputter( outputter )
{
  if ( !m_outputter )
    m_outputter = new CppUnit::TextOutputter( m_result, CPPUNIT_NS::stdCOut() );
  m_eventManager->addListener( m_result );
}


UVTextTestRunner::~UVTextTestRunner()
{
  delete m_eventManager;
  delete m_outputter;
  delete m_result;
}


/*! Runs the named test case.
 *
 * \param testName Name of the test case to run. If an empty is given, then
 *                 all added tests are run. The name can be the name of any
 *                 test in the hierarchy.
 * \param doWait if \c true then the user must press the RETURN key 
 *               before the run() method exit.
 * \param doPrintResult if \c true (default) then the test result are printed
 *                      on the standard output.
 * \param doPrintProgress if \c true (default) then TextTestProgressListener is
 *                        used to show the progress.
 * \return \c true is the test was successful, \c false if the test
 *         failed or was not found.
 */
bool
UVTextTestRunner::run( std::string testName,
                       bool doWait,
                       bool doPrintResult,
                       bool doPrintProgress )
{
  CppUnit::TextTestProgressListener progress;
  if ( doPrintProgress )
    m_eventManager->addListener( &progress );

  CppUnit::TestRunner *pThis = this;
  pThis->run( *m_eventManager, testName );

  if ( doPrintProgress )
    m_eventManager->removeListener( &progress );

  printResult( doPrintResult );
  wait( doWait );

  return m_result->wasSuccessful();
}


void 
UVTextTestRunner::wait( bool doWait )
{
#if !defined( CPPUNIT_NO_STREAM )
  if ( doWait ) 
  {
    CPPUNIT_NS::stdCOut() << "<RETURN> to continue\n";
    CPPUNIT_NS::stdCOut().flush();
    std::cin.get ();
  }
#endif
}


void 
UVTextTestRunner::printResult( bool doPrintResult )
{
  CPPUNIT_NS::stdCOut() << "\n";
  if ( doPrintResult )
    m_outputter->write();
}


/*! Returns the result of the test run.
 * Use this after calling run() to access the result of the test run.
 */
CPPUNIT_NS::TestResultCollector &
UVTextTestRunner::result() const
{
  return *m_result;
}


/*! Returns the event manager.
 * The instance of TestResult results returned is the one that is used to run the
 * test. Use this to register additional TestListener before running the tests.
 */
CPPUNIT_NS::TestResult &
UVTextTestRunner::eventManager() const
{
  return *m_eventManager;
}


/*! Specifies an alternate outputter.
 *
 * Notes that the outputter will be use after the test run only if \a printResult was
 * \c true.
 * \param outputter New outputter to use. The previous outputter is destroyed. 
 *                  The UVTextTestRunner assumes ownership of the outputter.
 * \see CompilerOutputter, XmlOutputter, TextOutputter.
 */
void 
UVTextTestRunner::setOutputter( CPPUNIT_NS::Outputter *outputter )
{
  delete m_outputter;
  m_outputter = outputter;
}


void 
UVTextTestRunner::run( CPPUNIT_NS::TestResult &controller,
                     const std::string &testPath )
{
  CppUnit::TestRunner::run( controller, testPath );
}



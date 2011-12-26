#ifndef UVCPPUNIT_UVTestResult_H
#define UVCPPUNIT_UVTestResult_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <cppunit/TestResult.h>
#include <cppunit/SynchronizedObject.h>
#include <cppunit/portability/CppUnitDeque.h>
#include <string>

//CPPUNIT_NS_BEGIN


#if CPPUNIT_NEED_DLL_DECL
//  template class CPPUNIT_API std::deque<TestListener *>;
#endif

/*! \brief Manages TestListener.
 * \ingroup TrackingTestExecution
 *
 * A single instance of this class is used when running the test. It is usually
 * created by the test runner (TestRunner).
 *
 * This class shouldn't have to be inherited from. Use a TestListener
 * or one of its subclasses to be informed of the ongoing tests.
 * Use a Outputter to receive a test summary once it has finished
 *
 * UVTestResult supplies a template method 'setSynchronizationObject()'
 * so that subclasses can provide mutual exclusion in the face of multiple
 * threads.  This can be useful when tests execute in one thread and
 * they fill a subclass of UVTestResult which effects change in another 
 * thread.  To have mutual exclusion, override setSynchronizationObject()
 * and make sure that you create an instance of ExclusiveZone at the 
 * beginning of each method.
 *
 * \see Test, TestListener, UVTestResultCollector, Outputter.
 */
class UVTestResult : public CppUnit::TestResult
{
public:
  /// Construct a UVTestResult
  UVTestResult( SynchronizationObject *syncObject = 0 );

  /// Destroys a test result
  virtual ~UVTestResult();

  virtual void addListener( CppUnit::TestListener *listener );

  virtual void removeListener( CppUnit::TestListener *listener );

  /// Resets the stop flag.
  virtual void reset();
  
  /// Stop testing
  virtual void stop();

  /// Returns whether testing should be stopped
  virtual bool shouldStop() const;

  /// Informs TestListener that a test will be started.
  virtual void startTest( CppUnit::Test *test );

  /*! \brief Adds an error to the list of errors. 
   *  The passed in exception
   *  caused the error
   */
  virtual void addError( CppUnit::Test *test, CppUnit::Exception *e );

  /*! \brief Adds a failure to the list of failures. The passed in exception
   * caused the failure.
   */
  virtual void addFailure( CppUnit::Test *test, CppUnit::Exception *e );

  /// Informs TestListener that a test was completed.
  virtual void endTest( CppUnit::Test *test );

  /// Informs TestListener that a test suite will be started.
  virtual void startSuite( CppUnit::Test *test );

  /// Informs TestListener that a test suite was completed.
  virtual void endSuite( CppUnit::Test *test );

  /*! \brief Run the specified test.
   * 
   * Calls startTestRun(), test->run(this), and finally endTestRun().
   */
  virtual void runTest( CppUnit::Test *test );

protected:
  /*! \brief Called to add a failure to the list of failures.
   */
  void addFailure( const CppUnit::TestFailure &failure );

  virtual void startTestRun( CppUnit::Test *test );
  virtual void endTestRun( CppUnit::Test *test );
  
protected:
  typedef CppUnitDeque<CppUnit::TestListener *> TestListeners;
  TestListeners m_listeners;
  bool m_stop;

private: 
  UVTestResult( const UVTestResult &other );
  UVTestResult &operator =( const UVTestResult &other );
};


//CPPUNIT_NS_END


#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif

#endif // CPPUNIT_UVTestResult_H



#ifndef UVCPPUNIT_UVTestResultCollector_H
#define UVCPPUNIT_UVTestResultCollector_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 4660 )  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <cppunit/TestSuccessListener.h>
#include <cppunit/portability/CppUnitDeque.h>


#if CPPUNIT_NEED_DLL_DECL
//  template class CPPUNIT_API std::deque<TestFailure *>;
//  template class CPPUNIT_API std::deque<Test *>;
#endif


/*! \brief Collects test result.
 * \ingroup WritingTestResult
 * \ingroup BrowsingCollectedTestResult
 * 
 * A UVTestResultCollector is a TestListener which collects the results of executing 
 * a test case. It is an instance of the Collecting Parameter pattern.
 *
 * The test framework distinguishes between failures and errors.
 * A failure is anticipated and checked for with assertions. Errors are
 * unanticipated problems signified by exceptions that are not generated
 * by the framework.
 * \see TestListener, TestFailure.
 */
class UVTestResultCollector : public CppUnit::TestSuccessListener
{
public:
  typedef CppUnitDeque<CppUnit::TestFailure *> TestFailures;
  typedef CppUnitDeque<CppUnit::Test *> Tests;


  /*! Constructs a UVTestResultCollector object.
   */
  UVTestResultCollector( SynchronizationObject *syncObject = 0 );

  /// Destructor.
  virtual ~UVTestResultCollector();

  void startTest( CppUnit::Test *test );
  void addFailure( const CppUnit::TestFailure &failure );

  virtual void reset();

  virtual int runTests() const;
  virtual int testErrors() const;
  virtual int testFailures() const;
  virtual int testFailuresTotal() const;

  virtual const TestFailures& failures() const;
  virtual const Tests &tests() const;

protected:
  void freeFailures();

  Tests m_tests;
  TestFailures m_failures;
  int m_testErrors;

private:
  /// Prevents the use of the copy constructor.
  UVTestResultCollector( const UVTestResultCollector &copy );

  /// Prevents the use of the copy operator.
  void operator =( const UVTestResultCollector &copy );
};



#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif


#endif  // CPPUNIT_UVTestResultCollector_H

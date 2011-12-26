#ifndef UVCPPUNIT_UVTextOutputter_H
#define UVCPPUNIT_UVTextOutputter_H

#include <cppunit/Portability.h>
#include <cppunit/Outputter.h>
#include <cppunit/portability/Stream.h>

class UVTestResultCollector;

/*! \brief Prints a TestResultCollector to a text stream.
 * \ingroup WritingTestResult
 */
class UVTextOutputter : public CppUnit::Outputter
{
public:
  UVTextOutputter( UVTestResultCollector *result,
                 CppUnit::OStream &stream );

  /// Destructor.
  virtual ~UVTextOutputter();

  void write();
  virtual void printFailures();
  virtual void printHeader();

  virtual void printFailure( CppUnit::TestFailure *failure,
                             int failureNumber );
  virtual void printFailureListMark( int failureNumber );
  virtual void printFailureTestName( CppUnit::TestFailure *failure );
  virtual void printFailureType( CppUnit::TestFailure *failure );
  virtual void printFailureLocation( CppUnit::SourceLine sourceLine );
  virtual void printFailureDetail( CppUnit::Exception *thrownException );
  virtual void printFailureWarning();
  virtual void printStatistics();

protected:
  UVTestResultCollector *m_result;
  CppUnit::OStream &m_stream;

private:
  /// Prevents the use of the copy constructor.
  UVTextOutputter( const UVTextOutputter &copy );

  /// Prevents the use of the copy operator.
  void operator =( const UVTextOutputter &copy );
};



#endif  // CPPUNIT_UVTextOutputter_H

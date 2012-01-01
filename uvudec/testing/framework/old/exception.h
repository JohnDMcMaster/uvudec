
#ifndef UVCPPUNIT_EXCEPTION_H
#define UVCPPUNIT_EXCEPTION_H

#include <cppunit/Exception.h>
#include <cppunit/Portability.h>
#include <cppunit/Message.h>
#include <cppunit/SourceLine.h>
#include <exception>



/*! \brief Exceptions thrown by failed assertions.
 * \ingroup BrowsingCollectedTestResult
 *
 * Exception is an exception that serves
 * descriptive strings through its what() method
 */
class UVException : public CppUnit::Exception
{
public:
  /*! \brief Constructs the exception with the specified message and source location.
   * \param message Message associated to the exception.
   * \param sourceLine Source location related to the exception.
   */
  UVException( const CppUnit::Message &message = Message(), 
             const CppUnit::SourceLine &sourceLine = CppUnit::SourceLine() );

#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
  /*!
   * \deprecated Use other constructor instead.
   */
  UVException( std::string  message, 
	     long lineNumber, 
	     std::string fileName );
#endif

  /*! \brief Constructs a copy of an exception.
   * \param other Exception to copy.
   */
  UVException( const UVException &other );

  /// Destructs the exception
  virtual ~UVException() throw();

  /// Performs an assignment
  UVException &operator =( const UVException &other );

  /// Returns descriptive message
  const char *what() const throw();

  /// Location where the error occured
  CppUnit::SourceLine sourceLine() const;

  /// Message related to the exception.
  CppUnit::Message message() const;

  /// Set the message.
  void setMessage( const Message &message );

#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
  /// The line on which the error occurred
  long lineNumber() const;

  /// The file in which the error occurred
  std::string fileName() const;

  static const std::string UNKNOWNFILENAME;
  static const long UNKNOWNLINENUMBER;
#endif

  /// Clones the exception.
  virtual Exception *clone() const;
};


#endif // CPPUNIT_EXCEPTION_H


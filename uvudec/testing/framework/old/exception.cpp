#include <cppunit/UVException.h>



#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
/*!
 * \deprecated Use SourceLine::isValid() instead.
 */
const std::string UVException::UNKNOWNFILENAME = "<unknown>";

/*!
 * \deprecated Use SourceLine::isValid() instead.
 */
const long UVException::UNKNOWNLINENUMBER = -1;
#endif


UVException::UVException( const UVException &other )
   : CppUnit::Exception( other )
{ 
} 


UVException::UVException( const Message &message, 
                      const SourceLine &sourceLine )
    : CppUnit::Exception( message, sourceLine )
{
}


#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
UVException::UVException( std::string message, 
                      long lineNumber, 
                      std::string fileName )
    : : CppUnit::Exception( message, fileName, lineNumber )
{
}
#endif


UVException::~UVException() throw()
{
}


UVException & 
UVException::operator =( const UVException& other )
{ 
// Don't call superclass operator =(). VC++ STL implementation
// has a bug. It calls the destructor and copy constructor of 
// std::exception() which reset the virtual table to std::exception.
//  SuperClass::operator =(other);

  if ( &other != this )
  {
  	//Copy parent data
  	//Ignore return code, we will take care of returning this as the correct object types
	CppUnit::Exception::operator=(other);
  }

  return *this; 
}


const char*
UVException::what() const throw()
{
  UVException *mutableThis = CPPUNIT_CONST_CAST( UVException *, this );
  mutableThis->m_whatMessage = m_message.shortDescription() + "\n" + 
                               m_message.details();
  return m_whatMessage.c_str();
}


SourceLine 
UVException::sourceLine() const
{
  return m_sourceLine;
}


Message 
UVException::message() const
{
  return m_message;
}


void 
UVException::setMessage( const Message &message )
{
  m_message = message;
}


#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
long 
UVException::lineNumber() const
{ 
  return m_sourceLine.isValid() ? m_sourceLine.lineNumber() : 
                                  UNKNOWNLINENUMBER; 
}


std::string 
UVException::fileName() const
{ 
  return m_sourceLine.isValid() ? m_sourceLine.fileName() : 
                                  UNKNOWNFILENAME;
}
#endif


UVException *
UVException::clone() const
{
  return new UVException( *this );
}



#ifndef SERIALIZED_TEST_RESULT_H
#define SERIALIZED_TEST_RESULT_H

#include <cppunit/TestListener.h>
#include <cppunit/TestResult.h>
#include <stdint.h>

using namespace CppUnit;

//CPPUNIT_NS_BEGIN

/*
Really what this is is a listener rolled into a TestResult
It should be broken out
*/

//TODO: replace this with something truely safe
//Never dereference this pointer
typedef const Test *SerializedTestID;
typedef struct {
	SerializedTestID test;
	uint8_t testsRan;
	uint8_t testsFailed;
	uint8_t isError;
} SerializedTestResult_t;

class SerializedTestListener : public TestListener
{
public:
	SerializedTestListener();
	virtual ~SerializedTestListener();

  	virtual void startSuite( Test * /*suite*/ );
  	virtual void endSuite( Test * /*suite*/ );
  	
  	virtual void startTest( Test * /*test*/ );
  	
	virtual void addFailure( const TestFailure & /*failure*/ );

	virtual unsigned int testsFailed();

	virtual size_t serialize(void *buff, size_t bufferSize);

	//Take the results in this object (failures etc) and make corresponding calls to given object
	//Thest test parameter is a double check until I figure something better
	virtual void augmentResult(TestResult *result, Test *test);

	static SerializedTestListener *deserialize(const void *buff, size_t bufferSize);

private:
	//TODO: consider replacing with vector so this can accept suites if I so ever desire
	SerializedTestResult_t m_result;
};


#endif


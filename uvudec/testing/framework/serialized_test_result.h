/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

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

#define MAX_DETAILS		3
//CppUnit::Message
typedef struct {
	char shortMessage[256];
	uint8_t details;
	char detail[256][MAX_DETAILS];
} SerializedMessage_t;

//CppUnit::SourceLine
typedef struct {
	char fileName[256];
	int lineNumber;
} SerializedSourceLine_t;

typedef struct {
	SerializedMessage_t message;
	SerializedSourceLine_t sourceLine;
} SerializedException_t;

//TODO: replace this with something truely safe
//Never dereference this pointer
typedef const Test *SerializedTestID;
typedef struct {
	SerializedTestID test;
	uint8_t testsRan;
	uint8_t testsFailed;
	uint8_t isError;
	
	SerializedException_t exception;
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
	static void serializeMessage(const Message &message, SerializedMessage_t *out);
	static void serializeSourceLine(const SourceLine &line, SerializedSourceLine_t *out);

	static Message deserializeMessage(const SerializedMessage_t *message);
	static SourceLine deserializeSourceLine(const SerializedSourceLine_t *sourceLine);
	
private:
	/*
	Serialization
	*/
	void serializeException(const Exception &exception);
	
	/*
	De-serialization
	*/
	Message message() const;
	SourceLine sourceLine() const;


	//TODO: consider replacing with vector so this can accept suites if I so ever desire
	SerializedTestResult_t m_result;
};


#endif


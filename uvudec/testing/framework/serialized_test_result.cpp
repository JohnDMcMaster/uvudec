/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include <cppunit/Exception.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestListener.h>
#include <cppunit/tools/Algorithm.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testing/framework/serialized_test_result.h"

static std::string SafeStr(const char *in, size_t inSize);

std::string SafeStr(const char *in, size_t inSize) {
	//Calling the copyish constructor chops off everything after the null
	return std::string(std::string(in, inSize).c_str());
}

#if 0
typedef struct {
	/*
	The test is created before the fork and so the pointer should be safe to pass
	It should be verified against all registered tests to make sure its valid
	*/
	CppUnit::Test *test;
	/*
	Exception should be a simple object
	Clone the object to make sure though that its not the more complex derrived type
	which we won't know how to serialize
	
	Consider decompising this into more basic C type objects to make it harder to taint across IPC
	Don't think std::string is safe to pass across
	*/
	//CppUnit::Exception e;
} UVTestResultEventData;

typedef enum {
	UV_TEST_RESULT_EVENT_INVALID,
	UV_TEST_RESULT_EVENT_ERROR,
	UV_TEST_RESULT_EVENT_FAILURE,
} UVTestResultEventType;

typedef struct {
	UVTestResultEventType t;
	union {
		//UV_TEST_RESULT_EVENT_ERROR
		UVTestResultEventData error;
		//UV_TEST_RESULT_EVENT_FAILURE
		UVTestResultEventData failure;
	} events;
} UVTestResultEvent;
#endif



SerializedTestListener::SerializedTestListener() {
	memset(&m_result, 0, sizeof(m_result));
}

SerializedTestListener::~SerializedTestListener() {
}

void SerializedTestListener::startSuite( Test * /*suite*/ ) {
	//For now only works on single tests
	throw std::exception();
}

void SerializedTestListener::endSuite( Test * /*suite*/ ) {
	//For now only works on single tests
	throw std::exception();
}
  	
void SerializedTestListener::startTest( Test * test ) {
	memset(&m_result, 0, sizeof(m_result));
	
	m_result.test = test;
	m_result.testsRan += 1;
}

void SerializedTestListener::addFailure( const TestFailure & failure ) {
	Exception *e = NULL;
	
	++m_result.testsFailed;
	m_result.isError = failure.isError();
	e = failure.thrownException();
	if (e) {
		serializeException(*e);
	}
}

//Returns the actual number of bytes written upon success
//or < 0 upon error
size_t SerializedTestListener::serialize(void *buff, size_t bufferSize) {
	size_t s = sizeof(SerializedTestResult_t);
	
	if (!buff) {
		throw std::exception();
	}
	if (bufferSize < s) {
		throw std::exception();
	}
	
	memcpy(buff, &m_result, s);
	return s;
}

Message SerializedTestListener::message() const {
	//return Message();
	return deserializeMessage(&m_result.exception.message);
}

SourceLine SerializedTestListener::sourceLine() const {
	//return SourceLine();
	return deserializeSourceLine(&m_result.exception.sourceLine);
}

//Take the results in this object (failures etc) and make corresponding calls to given object
void SerializedTestListener::augmentResult(TestResult *result, Test *test) {
	SerializedTestResult_t *r = &m_result;
	
	//TODO: refine this for more fortified or proper behavior
	if (!test) {
		test = (Test *)r->test;
	}
	if (!test) {
		throw std::exception();
	}
	if (r->test != test) {
		throw std::exception();
	}
	if (r->testsRan != 1) {
		throw std::exception();
	}
	if (r->testsFailed > 1) {
		throw std::exception();
	}
	
	for (unsigned int i = 0; i < r->testsRan; ++i) {
  		result->startTest( test );
		if (r->testsFailed) {
			//XXX: do any of these have object lifetime expectations I'm not meeting?
			//Create a dumb exception for now and enhance later
			/*
			NOTE:
			TestFailure::~TestFailure()
			  delete m_thrownException; 
			{ 
			}
			*/
			Exception *e = new Exception(message(), sourceLine());
			//TestFailure failure(test, &e, r->isError);
			//result->addFailure( failure );
			result->addFailure( test, e );
		}
  		result->endTest( test );
	}
}

void SerializedTestListener::serializeMessage(const Message &message, SerializedMessage_t *out) {
	//Be extra careful passing data
	memset(out, 0, sizeof(*out));
	
	strncpy(out->shortMessage, message.shortDescription().c_str(), sizeof(out->shortMessage));
	out->details = message.detailCount();
	if (message.detailCount() > MAX_DETAILS) {
		throw std::exception();
	}
	for (int i = 0; i < message.detailCount(); ++i) {
		strncpy(out->detail[i], message.detailAt(i).c_str(), sizeof(out->detail[i]));
	}
}

Message SerializedTestListener::deserializeMessage(const SerializedMessage_t *message) {
	Message ret;
	
	std::string shortMessage;
	shortMessage = SafeStr(message->shortMessage, sizeof(message->shortMessage));
	//printf("shortMessage: \"%s\", %d details\n", shortMessage.c_str(), message->details);
	ret.setShortDescription(shortMessage);
	if (message->details > MAX_DETAILS) {
		throw std::exception();
	}
	for (unsigned int i = 0; i < message->details; ++i) {
		std::string detail = SafeStr(message->detail[i], sizeof(message->detail[i]));
		//printf("detail[%d] = %s\n", i, detail.c_str());
		ret.addDetail(detail);
	}
	return ret;
}

void SerializedTestListener::serializeSourceLine(const SourceLine &line, SerializedSourceLine_t *out) {
	//printf("Serializing source line %s:%d\n", line.fileName().c_str(), line.lineNumber());
	//Be extra careful passing data
	memset(out, 0, sizeof(*out));
	
	//Note that isValid() is determined by file name being empty
	//We could pass it around for extra safety
	strncpy(out->fileName, line.fileName().c_str(), sizeof(out->fileName));
	out->lineNumber = line.lineNumber();
}

SourceLine SerializedTestListener::deserializeSourceLine(const SerializedSourceLine_t *sourceLine) {
	std::string fileName = SafeStr(sourceLine->fileName, sizeof(sourceLine->fileName));
	//printf("Deserialized to \"%s\":%d\n", fileName.c_str(), sourceLine->lineNumber);
	return SourceLine(fileName, sourceLine->lineNumber);	
}

void SerializedTestListener::serializeException(const Exception &exception) {
	serializeMessage(exception.message(), &m_result.exception.message);
	serializeSourceLine(exception.sourceLine(), &m_result.exception.sourceLine);
}

SerializedTestListener *SerializedTestListener::deserialize(const void *buff, size_t bufferSize) {
	SerializedTestListener *ret = NULL;
	const SerializedTestResult_t *res = NULL;
	
	if (!buff) {
		return NULL;
	}
	if (bufferSize != sizeof(SerializedTestResult_t)) {
		return NULL;
	}
	res = (const SerializedTestResult_t *)buff;
	
	ret = new SerializedTestListener();
	ret->m_result = *res;
	
	return ret;
}

unsigned int SerializedTestListener::testsFailed() {
	return m_result.testsFailed;
}


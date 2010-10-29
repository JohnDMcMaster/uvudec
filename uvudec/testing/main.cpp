/*
UVNet Universal Decompiler(uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details

Unit test
*/

//Based off of
//http://www.evocomp.de/tutorials/tutorium_cppunit/howto_tutorial_cppunit_en.html
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TestSuite.h>
#include <cppunit/ui/text/TestRunner.h>
#include <stdint.h>
#include <stdio.h>


int main(int argc, char* argv[])
{
	printf("main: begin\n");
	uint32_t wasSuccessful = false;

	if( true )
	{
		/// Get the top level suite from the registry
		CPPUNIT_NS::Test *suite = NULL;
		CPPUNIT_NS::TextUi::TestRunner uiTestRunner;
		CPPUNIT_NS::BriefTestProgressListener progress;

		//getRegistry(): return all registered tests
		//.makeTest(): make a test object out off all of the registered tests
		//Aparantly of type TestSuite, but function returns of subclass type Test
		suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
		
		/// Adds the test to the list of test to run
		uiTestRunner.addTest(suite);

		// Change the default outputter to a compiler error format outputter 
		// uncomment the following line if you need a compiler outputter.
		uiTestRunner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&uiTestRunner.result(),
				std::cout));
		uiTestRunner.eventManager().addListener(&progress);

		// Change the default outputter to a xml error format outputter 
		// uncomment the following line if you need a xml outputter.
		//runner.setOutputter( new CppUnit::XmlOutputter( &uiTestRunner.result(),
		//                                                    std::cerr ) );

		/// Run the tests.
		wasSuccessful = uiTestRunner.run();
	}
	else
	{
		//insert test-suite at test-runner by registry
		CPPUNIT_NS::TestRunner testRunner;
		//informs test-listener about testresults
		CPPUNIT_NS::TestResult testresult;
		//register listener for collecting the test-results
		CPPUNIT_NS::TestResultCollector collectedresults;

		testresult.addListener(&collectedresults);

		//register listener for per-test progress output
		CPPUNIT_NS::BriefTestProgressListener progress;
		testresult.addListener(&progress);

		testRunner.addTest(CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());
		testRunner.run(testresult);

		//output results in compiler-format
		CPPUNIT_NS::CompilerOutputter compileroutputter(&collectedresults, std::cout);
		compileroutputter.write();

		wasSuccessful = collectedresults.wasSuccessful();
	}

	printf("main: done, wasSuccessful: %d\n", wasSuccessful);

	// return 0 if tests were successful
	return wasSuccessful ? 0 : 1;
}


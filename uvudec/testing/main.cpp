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
#include "testing/main.h"
#include "uvd/util/benchmark.h"

std::vector<std::string> g_extraArgs;

int main(int argc, char **argv)
{
	std::vector<std::string> testsToRun;
	//Report time for ALL tests
	UVDBenchmark benchmark;
	
	printf("main: begin\n");
	benchmark.start();
	uint32_t wasSuccessful = false;

	for( int i = 1; i < argc; ++i )
	{
		const char *arg = argv[i];
		
		g_extraArgs.push_back(arg);
	}

	if( true )
	{
		CPPUNIT_NS::TextUi::TestRunner textUiTestRunner;
		CPPUNIT_NS::BriefTestProgressListener progress;

		//Run all if none specified
		if( testsToRun.empty() )
		{
			CPPUNIT_NS::Test *test = NULL;
			
			/// Get the top level suite from the registry
			//getRegistry(): return all registered tests
			//.makeTest(): make a test object out off all of the registered tests
			//Aparantly of type TestSuite, but function returns of subclass type Test
			test = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

			/// Adds the test to the list of test to run
			textUiTestRunner.addTest(test);
		}
		/*
		else
		{
			for( std::vector<std::string>::iterator iter = testsToRun.begin();
					iter != testsToRun.end(); ++iter )
			{
				const std::string &testName = *iter;	
				CPPUNIT_NS::Test *test = NULL;
				
				//textUiTestRunner.addTest(new CppUnit::TestCaller<MathTest>(
				//	"testAdd", &MathTest::testAdd));
				//ick how to get the symbol name?
				//they must be registered somewhere since makeTest() knows them...but TestFactoryRegistry doesn't seem to expose much
				textUiTestRunner.addTest(new CppUnit::TestCaller<UVDUnitTest>(testName, testAdd));

				//textUiTestRunner.addTest(test);
			}
		}
		*/

		// Change the default outputter to a compiler error format outputter 
		// uncomment the following line if you need a compiler outputter.
		textUiTestRunner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&textUiTestRunner.result(),
				std::cout));
		textUiTestRunner.eventManager().addListener(&progress);

		// Change the default outputter to a xml error format outputter 
		// uncomment the following line if you need a xml outputter.
		//runner.setOutputter( new CppUnit::XmlOutputter( &textUiTestRunner.result(),
		//                                                    std::cerr ) );

		/// Run the tests.
		wasSuccessful = textUiTestRunner.run();
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

	benchmark.stop();
	printf("main: done after %s, wasSuccessful: %d\n", benchmark.toString().c_str(), wasSuccessful);	

	// return 0 if tests were successful
	return wasSuccessful ? 0 : 1;
}


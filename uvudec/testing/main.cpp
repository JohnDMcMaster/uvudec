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
#include "testing/uvdobjgb.h"
#include "uvd/config/arg.h"
#include "uvd/config/arg_property.h"
#include "uvd/config/arg_util.h"
#include "uvd/util/benchmark.h"

std::vector<std::string> g_extraArgs;
UVDArgRegistry g_argRegistry;
UVDArgConfigs *g_configArgs = NULL;
std::vector<CPPUNIT_NS::Test *> g_tests;
std::map<std::string, CPPUNIT_NS::Test *> g_fixtureNameMap;
uvd_bool_t g_propagateArgs = false;
#define UVD_PROP_TESTING_FIXTURE			"testing.fixture"
#define UVD_PROP_TESTING_ARGS				"testing.args"

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments);

static uv_err_t initArgs()
{
	g_fixtureNameMap["uvdobjgb"] = UVDObjgbUnitTest::suite();
	
	uv_assert_err_ret(g_argRegistry.newArgConfgs(&g_configArgs));
	uv_assert_err_ret(g_configArgs->registerArgument(UVD_PROP_ACTION_HELP, 'h', "help", "print this message and exit", "", 0, argParser, false));
	uv_assert_err_ret(g_configArgs->registerArgument(UVD_PROP_ACTION_VERSION, 0, "version", "print version and exit", "", 0, argParser, false));
	uv_assert_err_ret(g_configArgs->registerArgument(UVD_PROP_TESTING_FIXTURE, 0, "fixture", "add a fixture to be tested", "", 1, argParser, false));
	uv_assert_err_ret(g_configArgs->registerArgument(UVD_PROP_TESTING_ARGS, 0, "args", "all future arguments passed to library initialization", "", 0, argParser, false));

	return UV_ERR_OK;
}

static void printfHelp()
{
	const char *program_name = "uvtest";

	printf_help("%s version %s\n", program_name, UVUDEC_VER_STRING);	
	UVDPrintHelp();

	printf_help("Fixtures:\n");	
	for( std::map<std::string, CPPUNIT_NS::Test *>::iterator iter = g_fixtureNameMap.begin();
			iter != g_fixtureNameMap.end(); ++iter )
	{
		printf_help("\t%s\n", (*iter).first.c_str());
	}
}

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments)
{
	//If present
	std::string firstArg;
	uint32_t firstArgNum = 0;
	bool firstArgBool = true;
	
	uv_assert_ret(argConfig);

	if( !argumentArguments.empty() )
	{
		firstArg = argumentArguments[0];
		firstArgNum = strtol(firstArg.c_str(), NULL, 0);
		firstArgBool = UVDArgToBool(firstArg);
	}

	//eat everything after --args?
	if( g_propagateArgs )
	{
		//Load'er up
		g_extraArgs.insert(g_extraArgs.begin(), argumentArguments.begin(), argumentArguments.end());
	}
	else if( argConfig->isNakedHandler() )
	{
		UVDPrintfError("no undecorated arg support, maybe you need --args\n");
		printfHelp();
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	else if( argConfig->m_propertyForm == UVD_PROP_ACTION_HELP )
	{
		printfHelp();
		return UV_ERR_DONE;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_ACTION_VERSION )
	{
		UVDPrintVersion();
		return UV_ERR_DONE;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_TESTING_FIXTURE )
	{
		uv_assert_ret(argumentArguments.size() == 1);
		if( g_fixtureNameMap.find(firstArg) == g_fixtureNameMap.end() )
		{
			printf_error("unrecognized fixture: %s\n", firstArg.c_str());
			printfHelp();
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		g_tests.push_back(g_fixtureNameMap[firstArg]);
	}
	else
	{
		printf_error("Property not recognized in callback: %s\n", argConfig->m_propertyForm.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}

int main(int argc, char **argv)
{
	uv_err_t rcTemp = UV_ERR_GENERAL;
	//Report time for ALL tests
	UVDBenchmark benchmark;
	uint32_t wasSuccessful = false;
	
	initArgs();
	rcTemp = g_argRegistry.processMain(argc, argv);
	if( UV_FAILED(rcTemp) )
	{
		printf_error("faled to parse args\n");
		goto error;
	}
	else if( rcTemp == UV_ERR_DONE )
	{
		wasSuccessful = true;
		goto error;
	}
	
	printf("main: beginning tests\n");
	benchmark.start();

	if( true )
	{
		CPPUNIT_NS::TextUi::TestRunner textUiTestRunner;
		CPPUNIT_NS::BriefTestProgressListener progress;

		//Run all if none specified
		if( g_tests.empty() )
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
		else
		{
			for( std::vector<CPPUNIT_NS::Test *>::iterator iter = g_tests.begin();
					iter != g_tests.end(); ++iter )
			{
				CPPUNIT_NS::Test *test = *iter;
				//CppUnit::TestResult result;
				
				textUiTestRunner.addTest(test);
								
				/*	
				CppUnit::TestCaller<UVDObjgbUnitTest> test(testName, 
								                             &ComplexNumberTest::testEquality );
				CppUnit::TestResult result;
				test.run( &result );
				*/
				
				/*
				CPPUNIT_NS::Test *test = NULL;
				
				//textUiTestRunner.addTest(new CppUnit::TestCaller<MathTest>(
				//	"testAdd", &MathTest::testAdd));
				//ick how to get the symbol name?
				//they must be registered somewhere since makeTest() knows them...but TestFactoryRegistry doesn't seem to expose much
				textUiTestRunner.addTest(new CppUnit::TestCaller<UVDUnitTest>(testName, testAdd));

				//textUiTestRunner.addTest(test);
				*/
			}
		}

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

error:
	// return 0 if tests were successful
	return wasSuccessful ? 0 : 1;
}


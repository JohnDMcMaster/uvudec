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
#include <cppunit/TextOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TestSuite.h>
#include <cppunit/ui/text/TestRunner.h>
#include <stdint.h>
#include <stdio.h>
#include "testing/assembly.h"
#include "testing/block.h"
#include "testing/flirtutil.h"
#include "testing/libuvudec.h"
#include "testing/main.h"
#include "testing/obj2pat.h"
#include "testing/progress_listener.h"
#include "testing/uvdobjgb.h"
#include "testing/uvudec.h"
#include "uvd/config/arg.h"
#include "uvd/config/arg_property.h"
#include "uvd/config/arg_util.h"
#include "uvd/util/benchmark.h"
#include "testing/framework/text_test_runner.h"
#include "testing/framework/text_outputter.h"

std::vector<std::string> g_extraArgs;
UVDArgRegistry g_argRegistry;
UVDArgConfigs *g_configArgs = NULL;
std::vector<CPPUNIT_NS::Test *> g_tests;
std::map<std::string, CPPUNIT_NS::Test *> g_fixtureNameMap;
#define UVD_PROP_TESTING_FIXTURE			"testing.fixture"

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments, void *user);

static uv_err_t initArgs()
{
	g_fixtureNameMap["assembly"] = UVDAssemblyUnitTest::suite();
	//g_fixtureNameMap["bin2obj" = UVDBin2objTestFixture::suite();
	g_fixtureNameMap["block"] = BlockFixture::suite();
	g_fixtureNameMap["flirtutil"] = UVDFlirtutilFixture::suite();
	g_fixtureNameMap["libuvudec"] = UVDLibuvudecUnitTest::suite();
	g_fixtureNameMap["obj2pat"] = UVDObj2patUnitTest::suite();
	g_fixtureNameMap["uvdobjgb"] = UVDGBUnitTest::suite();
	g_fixtureNameMap["uvudec"] = UVDUvudecUnitTest::suite();
	
	uv_assert_err_ret(g_argRegistry.newArgConfgs(&g_configArgs));
	uv_assert_err_ret(g_configArgs->registerArgument(UVD_PROP_ACTION_HELP, 'h', "help", "print this message and exit", "", 0, argParser, false, NULL));
	uv_assert_err_ret(g_configArgs->registerArgument(UVD_PROP_ACTION_VERSION, 0, "version", "print version and exit", "", 0, argParser, false, NULL));
	uv_assert_err_ret(g_configArgs->registerArgument(UVD_PROP_TESTING_FIXTURE, 0, "fixture", "add a fixture to be tested", "", 1, argParser, false, NULL));

	return UV_ERR_OK;
}

static uv_err_t printfHelp()
{
	const char *program_name = "uvtest";

	printf_help("%s version %s\n", program_name, UVUDEC_VER_STRING);	
	UVDPrintVersion();
	uv_assert_err_ret(g_argRegistry.printUsage());
	printf_help("Fixtures:\n");	
	for( std::map<std::string, CPPUNIT_NS::Test *>::iterator iter = g_fixtureNameMap.begin();
			iter != g_fixtureNameMap.end(); ++iter )
	{
		printf_help("\t%s\n", (*iter).first.c_str());
	}
	
	return UV_ERR_OK;
}

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments, void *user)
{
	//If present
	std::string firstArg;
	uint32_t firstArgNum = 0;
	bool firstArgBool = true;
	
	printf("main arg parser\n");
	
	uv_assert_ret(argConfig);

	if( !argumentArguments.empty() )
	{
		firstArg = argumentArguments[0];
		firstArgNum = strtol(firstArg.c_str(), NULL, 0);
		firstArgBool = UVDArgToBool(firstArg);
	}

	if( argConfig->isNakedHandler() )
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
		printf("Fixture\n");
		uv_assert_ret(argumentArguments.size() == 1);
		if( g_fixtureNameMap.find(firstArg) == g_fixtureNameMap.end() )
		{
			printf("bad fixture\n");
			printf_error("unrecognized fixture: %s\n", firstArg.c_str());
			//printfHelp();
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		g_tests.push_back(g_fixtureNameMap[firstArg]);
	}
	else
	{
		printf_error("main: property not recognized in callback: %s\n", argConfig->m_propertyForm.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	printf("returning OK\n");
	return UV_ERR_OK;
}








int main(int argc, char **argv)
{
	int parsed_argc = 0;
	
	uv_err_t rcTemp = UV_ERR_GENERAL;
	//Report time for ALL tests
	UVDBenchmark benchmark;
	uint32_t wasSuccessful = false;
	
	//Everything after --args and remove it
	for( parsed_argc = 1; parsed_argc < argc; ++parsed_argc )
	{
		if( std::string(argv[parsed_argc]) == "--args" )
		{
			for( int i = parsed_argc + 1; i < argc; ++i )
			{
				g_extraArgs.push_back(argv[i]);
			}
			break;
		}
	}
	
	UV_DEBUG(initArgs());
	rcTemp = g_argRegistry.processMain(parsed_argc, argv);
	if( UV_FAILED(rcTemp) )
	{
		printf_error("faled to parse args (rc = %d)\n", rcTemp);
		printfHelp();
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
		//CPPUNIT_NS::TextUi::TestRunner textUiTestRunner;
		UVTextTestRunner textUiTestRunner;
		//CPPUNIT_NS::BriefTestProgressListener progress;
		UVTestProgressListener progress;

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
		//textUiTestRunner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&textUiTestRunner.result(), std::cout));
		textUiTestRunner.setOutputter(new UVTextOutputter( &textUiTestRunner.result(), std::cout ));
		
		
		textUiTestRunner.eventManager().addListener(&progress);

		// Change the default outputter to a xml error format outputter 
		// uncomment the following line if you need a xml outputter.
		//runner.setOutputter( new CppUnit::XmlOutputter( &textUiTestRunner.result(),
		//                                                    std::cerr ) );

		/// Run the tests.
		wasSuccessful = textUiTestRunner.run("", false, true, false);
	}
#if 0
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
#endif

	benchmark.stop();
	printf("main: done after %s, wasSuccessful: %d\n", benchmark.toString().c_str(), wasSuccessful);	

error:
	// return 0 if tests were successful
	return wasSuccessful ? 0 : 1;
}


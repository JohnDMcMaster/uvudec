/*
UVNet Universal Decompiler (uvudec)
Copyright 2011 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
Everything has a purpose. 
The Emperor ordains it so. 
You may corrupt the souls of men. 
But I am steel! 
I am doom! 
I march for Macragge, and I know no fear!
*/

#include "block.h"
#include "uvd/util/debug.h"

CPPUNIT_TEST_SUITE_REGISTRATION(BlockFixture);

static bool g_bool = true;
//#define DO() CPPUNIT_ASSERT(g_bool); g_bool = false; return
//#define DO() CPPUNIT_ASSERT(g_bool); return
#define DO() (void)g_bool;

/*
If we register a notifier it should tell us after something is added or removed
Removing the notifier should no longer notify us
*/
static bool g_notified;
static UVDBasicBlock *g_notified_block;
static uvd_block_event_t g_notified_event;
static uv_err_t g_notifier_rc = UV_ERR_OK;
uv_err_t notifyTestNotifier(UVDBasicBlock *block, uvd_block_event_t event, void *user) {
	g_notified = true;
	g_notified_block = block;
	g_notified_event = event;
	printf("Notified, returning %d\n", g_notifier_rc);
	return g_notifier_rc;
}

void clearNotifications(void) {
	g_notifier_rc = UV_ERR_OK;
	g_notified = false;
	g_notified_block = NULL;
	g_notified_event = UVD_BLOCK_EVENT_INVALID;
}


void BlockFixture::setUp(void) {
	/*
	(gdb) info stack
	#0  BlockFixture::setUp (this=0x808a740) at ../testing/block.cpp:48
	#1  0x0806790a in CppUnit::TestCaller<BlockFixture>::setUp (this=0x808a7e8) at /usr/include/cppunit/TestCaller.h:177
	#2  0x01122fd9 in CppUnit::TestCaseMethodFunctor::operator() (this=0xbfffe32c) at TestCase.cpp:32
	#3  0x01115b79 in CppUnit::DefaultProtector::protect (this=0x808cb28, functor=..., context=...) at DefaultProtector.cpp:15
	#4  0x0111f3e4 in CppUnit::ProtectorChain::ProtectFunctor::operator() (this=0x808dc18) at ProtectorChain.cpp:20
	#5  0x0111eeaa in CppUnit::ProtectorChain::protect (this=0x808acd0, functor=..., context=...) at ProtectorChain.cpp:77
	#6  0x0112a6b8 in CppUnit::TestResult::protect (this=0x808d0b8, functor=..., test=0x808a7e8, shortDescription="setUp() failed") at TestResult.cpp:178
	#7  0x01122ce6 in CppUnit::TestCase::run (this=0x808a7e8, result=0x808d0b8) at TestCase.cpp:87
	#8  0x01123586 in CppUnit::TestComposite::doRunChildTests (this=0x808a5a8, controller=0x808d0b8) at TestComposite.cpp:64
	#9  0x0112349b in CppUnit::TestComposite::run (this=0x808a5a8, result=0x808d0b8) at TestComposite.cpp:23
	#10 0x0112cb3c in CppUnit::TestRunner::WrappingSuite::run (this=0x808d0a0, result=0x808d0b8) at TestRunner.cpp:47
	#11 0x08054c14 in UVTestResult::runTest (this=0x808d0b8, test=0x808d0a0) at ../testing/framework/test_result.cpp:257
	#12 0x0112c952 in CppUnit::TestRunner::run (this=0xbfffe508, controller=..., testPath="") at TestRunner.cpp:96
	#13 0x08052198 in UVTextTestRunner::run (this=0xbfffe508, controller=..., testPath="") at ../testing/framework/text_test_runner.cpp:147
	#14 0x08051fdb in UVTextTestRunner::run (this=0xbfffe508, testName="", doWait=false, doPrintResult=true, doPrintProgress=false)
		at ../testing/framework/text_test_runner.cpp:68
	#15 0x08060e28 in main (argc=3, argv=0xbfffe644) at ../testing/main.cpp:252
	*/
	printf("\nBlockFixture::setUp()\n");
	//UVD_PRINT_STACK();
	printf("\n");

	CPPUNIT_ASSERT(configInit() == UV_ERR_OK);
	
	m_block = UVDBasicBlock(UVDAddressRange(0, 16, &m_space));
	clearNotifications();
}

void BlockFixture::tearDown(void) {
	printf("\nBlockFixture::tearDown()\n");
	//UVD_PRINT_STACK();
	printf("\n");
}

void BlockFixture::addRemoveTest(void) {
	DO();


	UVDBlockGroup bg;
	uv_err_t rc_tmp = -1;
	printf("\n");
	
	UVCPPUNIT_ASSERT(bg.init(&m_space));
	
	rc_tmp = bg.add(&m_block);
	printf("\nrc: %d\n", rc_tmp);
	UVCPPUNIT_ASSERT(rc_tmp);
	CPPUNIT_ASSERT(bg.add(&m_block) == UV_ERR_DUPLICATE);

	UVCPPUNIT_ASSERT(bg.remove(&m_block));
	CPPUNIT_ASSERT(bg.remove(&m_block) == UV_ERR_NOTFOUND);
}

void BlockFixture::findTest(void) {
	DO();
	
	UVDBlockGroup bg;
	UVDBasicBlock b1, b2, b3;
	
	UVCPPUNIT_ASSERT(bg.init(&m_space));
	
	//A block by itself
	m_block = UVDBasicBlock(UVDAddressRange(0, 15, &m_space));
	b1 = UVDBasicBlock(UVDAddressRange(16, 31, &m_space));
	b2 = UVDBasicBlock(UVDAddressRange(23, 31, &m_space));
	b3 = UVDBasicBlock(UVDAddressRange(20, 32, &m_space));

	CPPUNIT_ASSERT(UV_FAILED(bg.add(&m_block)));
	//Should work for any of the addresses
	for (unsigned int i = 0; i <= 16; ++i) {
		UVDBasicBlockSet bs;
		
		UVCPPUNIT_ASSERT(bg.getAtAddress(i, &bs));
		CPPUNIT_ASSERT(bs.size() == 1);
		CPPUNIT_ASSERT((*bs.begin()) == &m_block);
	}
	
	CPPUNIT_ASSERT(UV_FAILED(bg.add(&b1)));
	CPPUNIT_ASSERT(UV_FAILED(bg.add(&b2)));
	CPPUNIT_ASSERT(UV_FAILED(bg.add(&b3)));
	

	{
		UVDBasicBlockSet bs;
		UVCPPUNIT_ASSERT(bg.getAtAddress(16, &bs));
		CPPUNIT_ASSERT(bs.size() == 1);
	}
	
	{
		UVDBasicBlockSet bs;
		UVCPPUNIT_ASSERT(bg.getAtAddress(23, &bs));
		CPPUNIT_ASSERT(bs.size() == 3);
	}
	
	{
		UVDBasicBlockSet bs;
		UVCPPUNIT_ASSERT(bg.getAtAddress(31, &bs));
		CPPUNIT_ASSERT(bs.size() == 3);
	}
	
	{
		UVDBasicBlockSet bs;
		UVCPPUNIT_ASSERT(bg.getAtAddress(32, &bs));
		CPPUNIT_ASSERT(bs.size() == 1);
	}

	{
		UVDBasicBlockSet bs;
		UVCPPUNIT_ASSERT(bg.getAtAddress(33, &bs));
		CPPUNIT_ASSERT(bs.size() == 0);
	}
	
		
	{
		UVDBasicBlockSet bs;
		UVCPPUNIT_ASSERT(bg.getAtAddresses( UVDAddressRange(0, 33), &bs ));
		CPPUNIT_ASSERT(bs.size() == 4);
	}
	/*
	no order garauntee
	(*(bs.begin() + 0)) == &m_block;
	(*(bs.begin() + 1)) == &b1;
	(*(bs.begin() + 2)) == &b2;
	(*(bs.begin() + 3)) == &b3;
	*/

	{
		UVDBasicBlockSet bs;
		UVCPPUNIT_ASSERT(bg.getAtAddresses( UVDAddressRange(0, 32), &bs ));
		CPPUNIT_ASSERT(bs.size() == 3);
	}

	{
		UVDBasicBlockSet bs;
		UVCPPUNIT_ASSERT(bg.getAtAddresses( UVDAddressRange(16, 32), &bs ));
		CPPUNIT_ASSERT(bs.size() == 3);
	}
}

void BlockFixture::notifyTest(void) {
	printf("before assertion\n");
	CPPUNIT_ASSERT(false);
	DO();
	printf("past assertion\n");
	
	
	UVDBlockGroup bg;
	//outer space get it haha
	
	UVCPPUNIT_ASSERT(bg.init(&m_space));
	
	UVCPPUNIT_ASSERT(bg.addNotifier(notifyTestNotifier, this));
	
	clearNotifications();
	UVCPPUNIT_ASSERT(bg.add(&m_block));
	CPPUNIT_ASSERT(g_notified);	
	CPPUNIT_ASSERT(g_notified_block == &m_block);	
	CPPUNIT_ASSERT(g_notified_event == UVD_BLOCK_EVENT_NEW);	
	
	clearNotifications();
	UVCPPUNIT_ASSERT(bg.remove(&m_block));
	CPPUNIT_ASSERT(g_notified);
	CPPUNIT_ASSERT(g_notified_block == &m_block);
	CPPUNIT_ASSERT(g_notified_event == UVD_BLOCK_EVENT_DELETE);
	
	clearNotifications();
	UVCPPUNIT_ASSERT(bg.removeNotifier(notifyTestNotifier, this));
	UVCPPUNIT_ASSERT(bg.add(&m_block));
	UVCPPUNIT_ASSERT(bg.remove(&m_block));
	CPPUNIT_ASSERT(!g_notified);	


	//Returning error in the notifier should propagate up
	clearNotifications();
	g_notifier_rc = UV_ERR_GENERAL;
	CPPUNIT_ASSERT(UV_FAILED(bg.add(&m_block)));
	//And the block shouldn't be added
	CPPUNIT_ASSERT(bg.remove(&m_block) == UV_ERR_NOTFOUND);
}



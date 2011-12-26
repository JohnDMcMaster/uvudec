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


static bool g_bool = true;
#define DO() CPPUNIT_ASSERT(g_bool); g_bool = false; return
//#define DO() CPPUNIT_ASSERT(g_bool); return

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
	CPPUNIT_ASSERT(configInit() == UV_ERR_OK);
	
	m_block = UVDBasicBlock(UVDAddressRange(0, 16, &m_space));
	clearNotifications();
}

void BlockFixture::tearDown(void) {
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
	DO();
	
	
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



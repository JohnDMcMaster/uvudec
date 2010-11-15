/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
The ROM for testing is as of yet undecided
*/

/*
Consider templating some of this
*/

#include "testing/uvdobjgb.h"
#include "uvd/core/runtime.h"
#include "uvd/core/uvd.h"
#include <string.h>

CPPUNIT_TEST_SUITE_REGISTRATION(UVDGBUnitTest);

//FIXME: we should be able to do something better with templating
#define UVCPPUNIT_ASSERT_EQUAL_INT(_targetIn, _valueIn) \
	do \
	{ \
		typeof(_valueIn) _target = _targetIn; \
		typeof(_valueIn) _value = _valueIn; \
		try \
		{ \
			CPPUNIT_ASSERT_EQUAL((int)_target, (int)_value); \
		} \
		catch(...) \
		{ \
			printf(UVDSprintf("target: 0x%%.%dX (%%d), value: 0x%%.%dX (%%d)\n", sizeof(_target) * 2, sizeof(_value) * 2).c_str(), _target, _target, _value, _value); \
			throw; \
		} \
	} while( 0 ) \

void UVDGBUnitTest::setUp(void)
{
	UVDTestingObjectFixture::setUp();
	m_pluginName = "uvdgb";
	//All tests on this for now
	m_uvdInpuFileName = getUnitTestDir() + "/image/game_boy/Arkaid_Release_02/arkaid.gb";
}

UVDObjgbPlugin *UVDGBUnitTest::getPlugin()
{
	CPPUNIT_ASSERT(m_plugin);
	return (UVDObjgbPlugin *)m_plugin;
}

UVDGBObject *UVDGBUnitTest::getObject()
{
	CPPUNIT_ASSERT(m_uvd);
	CPPUNIT_ASSERT(m_uvd->m_runtime);
	CPPUNIT_ASSERT(m_uvd->m_runtime->m_object);
	return (UVDGBObject *)m_uvd->m_runtime->m_object;
}

void UVDGBUnitTest::loadsCorrectObjectTest(void)
{
	//m_objectTypeID = typeid(UVDGBObject *);
	//CPPUNIT_ASSERT(typeid())
	init();
	CPPUNIT_ASSERT(typeid(UVDGBObject) == typeid(*m_object));
	getObject()->debugPrint();
	deinit();
}

void UVDGBUnitTest::isNintendoLogoTest(void)
{
}

void UVDGBUnitTest::getTitleTest(void)
{
	std::string title;
	std::string target = "ARKAID";

	init();
	UVCPPUNIT_ASSERT(getObject()->getTitle(title));
	try
	{
		CPPUNIT_ASSERT_EQUAL(target, title);
	}
	catch(...)
	{
		printf("target(%d): <%s>, got(%d): <%s>\n", target.size(), target.c_str(), title.size(), title.c_str());
		UVD_HEXDUMP_STR(target);
		UVD_HEXDUMP_STR(title);
		throw;
	}
	deinit();
}

void UVDGBUnitTest::getManufacturerCodeTest(void)
{
	uint32_t code = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getManufacturerCode(&code));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0, code);
}

void UVDGBUnitTest::getCGBFlagsTest(void)
{
	uint8_t flags = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getCGBFlags(&flags));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0xC0, flags);
}

void UVDGBUnitTest::getNewLicenseeCodeTest(void)
{
	uint16_t code = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getNewLicenseeCode(&code));
	deinit();
	CPPUNIT_ASSERT(0 == code);
}

void UVDGBUnitTest::getSGBFlagsTest(void)
{
	uint8_t flags = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getSGBFlags(&flags));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0, flags);
}

void UVDGBUnitTest::isSGBEnabledTest(void)
{
	uvd_bool_t data = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->isSGBEnabled(&data));
	deinit();
	CPPUNIT_ASSERT(!data);
}

void UVDGBUnitTest::getCartridgeTypeTest(void)
{
	uint8_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getCartridgeType(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0x01, result);
}

void UVDGBUnitTest::getROMSizeTest(void)
{
	uint32_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getROMSize(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0x00020000, result);
}

void UVDGBUnitTest::getROMSizeRawTest(void)
{
	uint8_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getROMSizeRaw(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0x02, result);
}

void UVDGBUnitTest::getSaveRAMSizeTest(void)
{
	uint32_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getSaveRAMSize(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0x00, result);
}

void UVDGBUnitTest::getSaveRAMSizeRawTest(void)
{
	uint8_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getSaveRAMSizeRaw(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0x00, result);
}

void UVDGBUnitTest::getDestinationCodeTest(void)
{
	uint8_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getDestinationCode(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0x01, result);
}

void UVDGBUnitTest::getOldLicenseeCodeTest(void)
{
	uint8_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getOldLicenseeCode(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0x00, result);
}

void UVDGBUnitTest::getMaskROMVersioNumberTest(void)
{
	uint8_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getMaskROMVersioNumber(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0x01, result);
}

void UVDGBUnitTest::getHeaderChecksumTest(void)
{
	uint8_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getHeaderChecksum(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0x76, result);
}

void UVDGBUnitTest::computeHeaderChecksumTest(void)
{
	uint8_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->computeHeaderChecksum(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0x76, result);
}

void UVDGBUnitTest::isHeaderChecksumValidTest(void)
{
	uvd_bool_t data = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->isHeaderChecksumValid(&data));
	deinit();
	CPPUNIT_ASSERT(data);
}

void UVDGBUnitTest::getGlobalChecksumTest(void)
{
	uint16_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->getGlobalChecksum(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0xCEF2, result);
}

void UVDGBUnitTest::computeGlobalChecksumTest(void)
{
	uint16_t result = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->computeGlobalChecksum(&result));
	deinit();
	UVCPPUNIT_ASSERT_EQUAL_INT(0xCEF2, result);
}

void UVDGBUnitTest::isGlobalChecksumValidTest(void)
{
	uvd_bool_t data = 0;

	init();
	UVCPPUNIT_ASSERT(getObject()->isGlobalChecksumValid(&data));
	deinit();
	CPPUNIT_ASSERT(data);
}


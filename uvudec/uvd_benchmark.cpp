#include "uvd_benchmark.h"
#include "uvd_util.h"
#include <string>

/*
UVDBenchmark BNAME;
BNAME.start();

BNAME.stop();
printf_debug_level(UVD_DEBUG_PASSES, "BSTR time: %s\n", BNAME.toString().c_str());
*/

UVDBenchmark::UVDBenchmark()
{
	m_start = 0;
	m_end = 0;
	m_debugLevel = UVD_DEBUG_VERBOSE;
	start();
}

UVDBenchmark::UVDBenchmark(int debugLevel)
{
	m_start = 0;
	m_end = 0;
	m_debugLevel = debugLevel;
	start();
}

void UVDBenchmark::start()
{
	m_start = getTimingMicroseconds();
}

uint64_t UVDBenchmark::getStart()
{
	return m_start;
}

void UVDBenchmark::stop()
{
	m_end = getTimingMicroseconds();
}

uint64_t UVDBenchmark::getStop()
{
	return m_end;
}

void UVDBenchmark::print()
{
	printf_debug_level(m_debugLevel, "time: %s\n", toString().c_str());
}

std::string UVDBenchmark::toString()
{
	uint64_t delta = getDelta();

	int microseconds = delta % 1000000;
	int seconds = delta / 1000000;
	int minutes = seconds / 60;
	seconds = seconds % 60;
	int hours = minutes / 60;
	minutes = minutes % 60;

	char buffer[64];
	snprintf(buffer, 64, "%.2d:%.2d:%.2d.%.6d", hours, minutes, seconds, microseconds);
	return std::string(buffer);
}

uint64_t UVDBenchmark::getDelta()
{
	if( m_start > m_end )
	{
		return 0;
	}
	return (uint64_t)(m_end - m_start);
}


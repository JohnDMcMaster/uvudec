/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#pragma once

#include <stdint.h>
#include <string>

/*
Benchmark with microsecond accuracy
*/
class UVDBenchmark
{
public:
	UVDBenchmark();
	UVDBenchmark(int debugLevel);
	
	//Start benchmark
	void start();
	uint64_t getStart();
	//End benchmark
	void stop();
	uint64_t getStop();
	
	std::string toString();
	//Human readable print
	void print();
	//Get delta in us
	uint64_t getDelta();
	
protected:
	//To control printing
	int m_debugLevel;
	//Benchmark start time in us
	uint64_t m_start;
	//Benchmark end time in us
	uint64_t m_end;
};

cmake_minimum_required(VERSION 3.20.0)

set(cppunit_sources
	collectoroutput.cpp
	compileroutput.cpp
	htmloutput.cpp
	missing.cpp
	source.cpp
	suite.cpp
	textoutput.cpp
	time.cpp
	utils.cpp
)

set(cppunit_headers
	cpptest-assert.h
	cpptest-collectoroutput.h
	cpptest-compileroutput.h
	cpptest-htmloutput.h
	cpptest-output.h
	cpptest-source.h
	cpptest-suite.h
	cpptest-textoutput.h
	cpptest-time.h
	cpptest.h
	missing.h
	utils.h
	winconfig.h
)

add_library(cppunit ${cppunit_sources} ${cppunit_headers})


if(UNIX)
	add_definitions(-fPIC)
endif()

set(LIBRARY_OUTPUT_PATH ../../bin)

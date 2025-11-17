CMAKE_MINIMUM_REQUIRED(VERSION 3.17.0)

SET(cppunit_sources
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

SET(cppunit_headers
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

ADD_LIBRARY(cppunit ${cppunit_sources} ${cppunit_headers})


IF(UNIX)
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(LIBRARY_OUTPUT_PATH ../../bin)

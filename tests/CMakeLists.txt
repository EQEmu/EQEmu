CMAKE_MINIMUM_REQUIRED(VERSION 3.17.0)

ADD_SUBDIRECTORY(cppunit)

SET(tests_sources
	main.cpp
)

SET(tests_headers
	atobool_test.h
	data_verification_test.h
	fixed_memory_test.h
	fixed_memory_variable_test.h
	hextoi_32_64_test.h
	ipc_mutex_test.h
	memory_mapped_file_test.h
	string_util_test.h
	skills_util_test.h
	task_state_test.h
)

ADD_EXECUTABLE(tests ${tests_sources} ${tests_headers})

TARGET_LINK_LIBRARIES(tests common cppunit ${SERVER_LIBS})

INSTALL(TARGETS tests RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

IF(MSVC)
	SET_TARGET_PROPERTIES(tests PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(tests "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(tests "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	TARGET_LINK_LIBRARIES(tests "${CMAKE_DL_LIBS}")
	TARGET_LINK_LIBRARIES(tests "z")
	TARGET_LINK_LIBRARIES(tests "m")
	IF(NOT DARWIN)
		TARGET_LINK_LIBRARIES(tests "rt")
	ENDIF(NOT DARWIN)
	TARGET_LINK_LIBRARIES(tests "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

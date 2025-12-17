cmake_minimum_required(VERSION 3.20.0)

add_subdirectory(cppunit)

set(tests_sources
	main.cpp
)

set(tests_headers
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

add_executable(tests ${tests_sources} ${tests_headers})

target_link_libraries(tests common cppunit)
target_include_directories(tests PRIVATE ..)

install(TARGETS tests RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

if(MSVC)
	set_target_properties(tests PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	target_link_libraries(tests "Ws2_32.lib")
endif()

if(MINGW)
	target_link_libraries(tests "WS2_32")
endif()

if(UNIX)
	target_link_libraries(tests "${CMAKE_DL_LIBS}")
	target_link_libraries(tests "z")
	target_link_libraries(tests "m")
	if(NOT DARWIN)
		target_link_libraries(tests "rt")
	endif()
	target_link_libraries(tests "pthread")
	add_definitions(-fPIC)
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set_property(TARGET tests PROPERTY FOLDER executables/tests)

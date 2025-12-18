cmake_minimum_required(VERSION 3.20)

set(ucs_sources
	chatchannel.cpp
	clientlist.cpp
	database.cpp
	ucs.cpp
	ucsconfig.cpp
	worldserver.cpp
)

set(ucs_headers
	chatchannel.h
	clientlist.h
	database.h
	ucsconfig.h
	worldserver.h
)

add_executable(ucs ${ucs_sources} ${ucs_headers})

install(TARGETS ucs RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_definitions(-DUCS)

target_link_libraries(ucs common)
target_include_directories(ucs PRIVATE ..)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set_property(TARGET ucs PROPERTY FOLDER executables/servers)

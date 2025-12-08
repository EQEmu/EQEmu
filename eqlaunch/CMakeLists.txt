cmake_minimum_required(VERSION 3.20)

set(eqlaunch_sources
	eqlaunch.cpp
	worldserver.cpp
	zone_launch.cpp
)

set(eqlaunch_headers
	worldserver.h
	zone_launch.h
)

add_executable(eqlaunch ${eqlaunch_sources} ${eqlaunch_headers})

install(TARGETS eqlaunch RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

target_link_libraries(eqlaunch common)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

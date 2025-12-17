cmake_minimum_required(VERSION 3.17.0)

set(shared_memory_sources
	items.cpp
	main.cpp
	spells.cpp
)

set(shared_memory_headers
	items.h
	spells.h
)

add_executable(shared_memory ${shared_memory_sources} ${shared_memory_headers})

install(TARGETS shared_memory RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

target_link_libraries(shared_memory common)
target_include_directories(shared_memory PRIVATE ..)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set_property(TARGET shared_memory PROPERTY FOLDER executables/servers)

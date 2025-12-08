cmake_minimum_required(VERSION 3.20.0)

set(import_sources
	main.cpp
)

set(import_headers
)

add_executable(import_client_files ${import_sources} ${import_headers})

install(TARGETS import_client_files RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

target_link_libraries(import_client_files common)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

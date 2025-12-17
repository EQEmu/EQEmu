cmake_minimum_required(VERSION 3.20.0)

set(export_sources
	main.cpp
)

set(export_headers
)

add_executable(export_client_files ${export_sources} ${export_headers})

install(TARGETS export_client_files RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

target_link_libraries(export_client_files common)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set_property(TARGET export_client_files PROPERTY FOLDER executables/tools)

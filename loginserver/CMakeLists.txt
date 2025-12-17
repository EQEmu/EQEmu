cmake_minimum_required(VERSION 3.20)

set(eqlogin_sources
	account_management.cpp
	client.cpp
	client_manager.cpp
	encryption.cpp
	loginserver_command_handler.cpp
	loginserver_webserver.cpp
	main.cpp
	world_server_manager.cpp
	world_server.cpp
)

set(eqlogin_headers
	account_management.h
	client.h
	client_manager.h
	encryption.h
	loginserver_command_handler.h
	loginserver_webserver.h
	login_server.h
	login_types.h
	options.h
	world_server_manager.h
	world_server.h
)

add_compile_definitions(EQEMU_USE_OPENSSL)

add_executable(loginserver ${eqlogin_sources} ${eqlogin_headers})

install(TARGETS loginserver RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

target_link_libraries(loginserver common)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set_property(TARGET loginserver PROPERTY FOLDER executables/servers)

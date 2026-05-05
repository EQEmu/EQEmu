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
target_include_directories(loginserver PRIVATE ..)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set_property(TARGET loginserver PROPERTY FOLDER executables/servers)

# vcpkg doesn't copy legacy.dll automatically because it is loaded at runtime, not via the import table.
if(WIN32 AND DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
    add_custom_command(TARGET loginserver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<IF:$<CONFIG:Debug>,${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin/legacy.dll,${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/legacy.dll>"
            "$<TARGET_FILE_DIR:loginserver>/legacy.dll"
        VERBATIM
    )
endif()

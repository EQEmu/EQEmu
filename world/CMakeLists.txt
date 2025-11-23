cmake_minimum_required(VERSION 3.20)

set(world_sources
    adventure.cpp
    adventure_manager.cpp
    client.cpp
    cliententry.cpp
    clientlist.cpp
    console.cpp
    ../common/data_bucket.cpp
    dynamic_zone.cpp
    dynamic_zone_manager.cpp
    eql_config.cpp
    eqemu_api_world_data_service.cpp
    launcher_link.cpp
    launcher_list.cpp
    lfplist.cpp
    login_server.cpp
    login_server_list.cpp
    main.cpp
    queryserv.cpp
    shared_task_manager.cpp
    shared_task_world_messaging.cpp
    ucs.cpp
    web_interface.cpp
    web_interface_eqw.cpp
    wguild_mgr.cpp
    world_event_scheduler.cpp
    world_config.cpp
    world_console_connection.cpp
    world_server_cli.cpp
    worlddb.cpp
    world_boot.cpp
    zonelist.cpp
    zoneserver.cpp
    )

set(world_headers
    adventure.h
    adventure_manager.h
    adventure_template.h
    client.h
    cliententry.h
    clientlist.h
    console.h
    ../common/data_bucket.h
    dynamic_zone.h
    dynamic_zone_manager.h
    eql_config.h
    eqemu_api_world_data_service.h
    launcher_link.h
    launcher_list.h
    lfplist.h
    login_server.h
    login_server_list.h
    queryserv.h
    shared_task_manager.h
    shared_task_world_messaging.h
    sof_char_create_data.h
    ucs.h
    web_interface.h
    web_interface_eqw.h
    wguild_mgr.h
    world_config.h
    world_console_connection.h
    world_tcp_connection.h
    world_server_cli.h
    worlddb.h
    world_boot.h
    world_event_scheduler.h
    zonelist.h
    zoneserver.h
    )

add_executable(world ${world_sources} ${world_headers})

install(TARGETS world RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

if(WIN32 AND EQEMU_BUILD_PCH)
    TARGET_PRECOMPILE_HEADERS(world PRIVATE ../common/pch/std-pch.h)
endif()

add_definitions(-DWORLD)

target_link_libraries(world ${SERVER_LIBS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

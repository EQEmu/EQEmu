cmake_minimum_required(VERSION 3.20)

set(world_sources
    adventure.cpp
    adventure_manager.cpp
    cli/cli_bots_disable.cpp
    cli/cli_bots_enable.cpp
    cli/cli_copy_character.cpp
    cli/cli_database_concurrency.cpp
    cli/cli_database_dump.cpp
    cli/cli_database_get_schema.cpp
    cli/cli_database_set_account_status.cpp
    cli/cli_database_updates.cpp
    cli/cli_database_version.cpp
    cli/cli_etl_get_settings.cpp
    cli/cli_mercs_disable.cpp
    cli/cli_mercs_enable.cpp
    cli/cli_test.cpp
    cli/cli_test_colors.cpp
    cli/cli_test_expansion.cpp
    cli/cli_test_repository.cpp
    cli/cli_test_repository_2.cpp
    cli/cli_test_string_benchmark.cpp
    cli/cli_version.cpp
    client.cpp
    cliententry.cpp
    clientlist.cpp
    console.cpp
    dynamic_zone.cpp
    dynamic_zone_manager.cpp
    eqemu_api_world_data_service.cpp
    eql_config.cpp
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
    world_boot.cpp
    world_config.cpp
    world_console_connection.cpp
    world_event_scheduler.cpp
    world_server_cli.cpp
    worlddb.cpp
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
    dynamic_zone.h
    dynamic_zone_manager.h
    eqemu_api_world_data_service.h
    eql_config.h
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
    world_boot.h
    world_config.h
    world_console_connection.h
    world_event_scheduler.h
    world_server_cli.h
    world_tcp_connection.h
    worlddb.h
    zonelist.h
    zoneserver.h
)

source_group("cli" REGULAR_EXPRESSION "cli/.*")

add_executable(world ${world_sources} ${world_headers})

target_compile_definitions(world PRIVATE WORLD)

target_link_libraries(world common)

if(WIN32 AND EQEMU_BUILD_PCH)
    target_precompile_headers(world PRIVATE ../common/pch/std-pch.h)
endif()

install(TARGETS world RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
cmake_minimum_required(VERSION 3.20)

add_library(eqemu_world_selector_core STATIC
    selector_state.cpp
    selector_state.h
    ../common/json/jsoncpp.cpp
    ../common/multi_world_selector.cpp
    ../common/multi_world_selector.h
)
target_include_directories(eqemu_world_selector_core PUBLIC ..)

add_executable(eqemu_world_selector
    main.cpp
    selector_config_path.h
    world_selector_server.cpp
    world_selector_server.h
)
target_include_directories(eqemu_world_selector PRIVATE ..)
target_link_libraries(eqemu_world_selector
    eqemu_world_selector_core
    $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
)

if(WIN32)
    target_link_libraries(eqemu_world_selector Ws2_32.lib Iphlpapi.lib Userenv.lib)
endif()

if(EQEMU_BUILD_TESTS)
    add_executable(eqemu_world_selector_tests
        tests.cpp
        ../common/multi_world_selector_notifier.cpp
        ../common/multi_world_selector_notifier.h
        selector_config_path.h
        world_selector_server.cpp
        world_selector_server.h
    )
    target_include_directories(eqemu_world_selector_tests PRIVATE ..)
    target_link_libraries(eqemu_world_selector_tests
        eqemu_world_selector_core
        $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
    )
    if(WIN32)
        target_link_libraries(eqemu_world_selector_tests Ws2_32.lib Iphlpapi.lib Userenv.lib)
    endif()
    set_property(TARGET eqemu_world_selector_tests PROPERTY FOLDER executables/tests)
endif()

install(TARGETS eqemu_world_selector RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set_property(TARGET eqemu_world_selector PROPERTY FOLDER executables/servers)

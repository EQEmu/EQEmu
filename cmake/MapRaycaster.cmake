function(eqemu_link_map_raycaster target)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux"
            OR NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$"
            OR CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "Rust map backend requires native Linux x86-64")
    endif()

    find_package(Threads REQUIRED)

    if(TARGET map_raycaster)
        get_target_property(map_headers map_raycaster EQEMU_MAP_HEADERS)
        target_link_libraries(${target} PRIVATE map_raycaster Threads::Threads)
        target_include_directories(${target} PRIVATE "${map_headers}")
        add_dependencies(${target} map_raycaster_build)
        return()
    endif()

    # Cargo alone compiles generated CXX code; native callers include its header.
    # Use the guest's Cargo selection and inherit its pinned RUSTC/RUSTDOC,
    # CARGO_HOME and offline dependency environment. Never execute Cargo at configure.
    if(DEFINED ENV{CARGO} AND NOT "$ENV{CARGO}" STREQUAL "")
        set(map_cargo "$ENV{CARGO}")
    else()
        find_program(map_cargo NAMES cargo REQUIRED)
    endif()

    if(DEFINED ENV{CARGO_TARGET_DIR} AND NOT "$ENV{CARGO_TARGET_DIR}" STREQUAL "")
        set(map_target_dir "$ENV{CARGO_TARGET_DIR}")
    else()
        # A sibling of the source tree also works for -B build and nested presets.
        # Keep independent CMake build trees from sharing Cargo's output/clean state.
        file(REAL_PATH "${CMAKE_BINARY_DIR}" map_binary_dir)
        string(SHA256 map_build_id "${map_binary_dir}")
        get_filename_component(map_source_parent "${CMAKE_SOURCE_DIR}" DIRECTORY)
        set(map_target_dir "${map_source_parent}/.eqemu-rust-target/${map_build_id}")
    endif()
    file(REAL_PATH "${map_target_dir}" map_target_dir BASE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    file(REAL_PATH "${CMAKE_SOURCE_DIR}" map_source_dir)
    string(FIND "${map_target_dir}/" "${map_source_dir}/" map_target_in_source)
    if(map_target_in_source EQUAL 0)
        message(FATAL_ERROR "map-raycaster requires a Cargo target directory outside the source tree")
    endif()

    set(map_sanitizer_env)
    if(EQEMU_SANITIZERS)
        set(map_sanitizer_env "EQEMU_MAP_SANITIZERS=1")
    endif()
    set(map_archive "${map_target_dir}/x86_64-unknown-linux-gnu/release/libmap_raycaster.a")
    # Always let Cargo check its own inputs; its incremental build also handles
    # changes to build.rs, generated CXX code and the locked dependency graph.
    add_custom_target(map_raycaster_build
        COMMAND "${CMAKE_COMMAND}" -E env
            "CARGO_TARGET_DIR=${map_target_dir}"
            "CARGO_NET_OFFLINE=true"
            ${map_sanitizer_env}
            "${map_cargo}" build
            --manifest-path "${CMAKE_SOURCE_DIR}/rust/raycaster/Cargo.toml"
            --package map-raycaster --release --frozen --offline
            --target x86_64-unknown-linux-gnu
        BYPRODUCTS "${map_archive}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Building the Rust map raycaster (offline, frozen)"
        VERBATIM
    )
    add_library(map_raycaster STATIC IMPORTED GLOBAL)
    set_target_properties(map_raycaster PROPERTIES IMPORTED_LOCATION "${map_archive}")
    add_dependencies(map_raycaster map_raycaster_build)

    # Native libraries required by the pinned Linux Rust staticlib.
    set_property(TARGET map_raycaster PROPERTY INTERFACE_LINK_LIBRARIES
        "${CMAKE_DL_LIBS};m;rt;util")
    # CXX exposes headers beside Cargo's build profiles; include both native
    # and explicit-target layouts. Generated code remains compiled only by Cargo.
    set(map_headers "${map_target_dir}/cxxbridge;${map_target_dir}/x86_64-unknown-linux-gnu/cxxbridge")
    set_target_properties(map_raycaster PROPERTIES EQEMU_MAP_HEADERS "${map_headers}")
    target_include_directories(${target} PRIVATE "${map_headers}")
    add_dependencies(${target} map_raycaster_build)
    target_link_libraries(${target} PRIVATE map_raycaster Threads::Threads)
endfunction()

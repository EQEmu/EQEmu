cmake_minimum_required(VERSION 3.20.0)

set(lb_sources
    src/class.cpp
	src/class_info.cpp
	src/class_registry.cpp
	src/class_rep.cpp
	src/create_class.cpp
	src/error.cpp
	src/exception_handler.cpp
	src/function.cpp
	src/inheritance.cpp
	src/link_compatibility.cpp
	src/object_rep.cpp
	src/open.cpp
	src/pcall.cpp
	src/scope.cpp
	src/stack_content_by_name.cpp
	src/weak_ref.cpp
	src/wrapper_base.cpp
)

set(lb_headers

)

add_library(luabind ${lb_sources} ${lb_headers})
target_link_libraries(luabind PRIVATE PkgConfig::luajit Boost::dynamic_bitset Boost::tuple Boost::foreach)

if(UNIX)
    set_source_files_properties(${lb_sources} PROPERTY COMPILE_FLAGS -Wno-deprecated-declarations)
endif()

if(MSVC)
	set_source_files_properties(${lb_sources} PROPERTY COMPILE_FLAGS " /W0 " )
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

cmake_minimum_required(VERSION 3.20.0)

project(perlbind LANGUAGES CXX)

set(PERLBIND_HEADERS
  include/perlbind/array.h
  include/perlbind/forward.h
  include/perlbind/function.h
  include/perlbind/hash.h
  include/perlbind/interpreter.h
  include/perlbind/iterator.h
  include/perlbind/package.h
  include/perlbind/perlbind.h
  include/perlbind/scalar.h
  include/perlbind/stack.h
  include/perlbind/stack_push.h
  include/perlbind/stack_read.h
  include/perlbind/subcaller.h
  include/perlbind/traits.h
  include/perlbind/typemap.h
  include/perlbind/types.h
  include/perlbind/util.h
  include/perlbind/version.h
)

set(PERLBIND_SOURCES
  src/function.cpp
  src/hash.cpp
  src/interpreter.cpp
  src/package.cpp
)

if(MSVC)
  set(PERLBIND_SOURCES ${PERLBIND_SOURCES} src/perlbind.natvis)
endif()

add_library(perlbind ${PERLBIND_SOURCES} ${PERLBIND_HEADERS})

target_include_directories(perlbind PUBLIC
  ${PERL_INCLUDE_PATH}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

option(PERLBIND_BUILD_TESTS "Build tests" OFF)
option(PERLBIND_ENABLE_ASAN "Build with address sanitizer" OFF)

if(PERLBIND_ENABLE_ASAN)
  target_compile_options(perlbind PRIVATE -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(perlbind PRIVATE -fsanitize=address -fno-omit-frame-pointer)
endif()

if(PERLBIND_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT tests)

  target_include_directories(tests PRIVATE
    ${PERL_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

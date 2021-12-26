cmake_minimum_required(VERSION 3.12)

project(FractalServer VERSION 0.1
             DESCRIPTION "Adobe Native Extension which allows the creation of an HTTPS server."
             HOMEPAGE_URL "https://github.com/al-sabr/FractalServer")

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OPENSSL_USE_STATIC_LIBS TRUE CACHE STRING "OpenSSL use mode")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(link_threads_if_necessary)
include(link_atomic_if_necessary)

find_package(fmt CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(restinio CONFIG REQUIRED)
find_package(unofficial-http-parser CONFIG REQUIRED)
find_package(flash-runtime-extensions CONFIG REQUIRED)

set(HEADER_FILES
  include/dllMain.h
  include/includes.h
  include/cross-platform.h
  include/FractalServer.h
)

set(SRC_FILES
  src/dllMain.cpp
  src/FractalServer.cpp
)

if(WIN32)
  set(HEADER_FILES ${HEADER_FILES}
      include/win32.h
  )
  #set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_library(${PROJECT_NAME} SHARED ${SRC_FILES} ${HEADER_FILES})

set(WIN32_DEPS "")

if(WIN32)
  set(WIN32_DEPS ${WIN32_DEPS} wsock32 ws2_32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE -D_WIN32_WINNT=0x0601 -DWIN_EXPORT)
  target_compile_options(${PROJECT_NAME} PRIVATE /EHsc /MTd /W2 /c)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE
  fmt::fmt-header-only
  restinio::restinio
  OpenSSL::SSL OpenSSL::Crypto
  unofficial::http_parser::http_parser
  flash-runtime-extensions::flash-runtime-extensions
  ${WIN32_DEPS})

link_atomic_if_necessary(${PROJECT_NAME})
link_threads_if_necessary(${PROJECT_NAME})

# Let's set compiler-specific flags
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    # G++
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    # MSVC
    target_compile_options(${PROJECT_NAME} PRIVATE /EHsc /MTd /W2 /c)
    # Set the DLLEXPORT variable to export symbols
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN_EXPORT)
endif()
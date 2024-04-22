## Copyright 2018 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

set(API_SOURCES
  api.cpp
)

add_library(OpenImageDenoiseAMD ${OIDN_LIB_TYPE} ${API_SOURCES} ${OIDN_RESOURCE_FILE})
set_property(TARGET OpenImageDenoiseAMD PROPERTY VERSION ${PROJECT_VERSION})
set_property(TARGET OpenImageDenoiseAMD PROPERTY SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(OpenImageDenoiseAMD
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(OpenImageDenoiseAMD PRIVATE OpenImageDenoiseAMD_core)

if(NOT OIDN_STATIC_LIB)
  oidn_strip_symbols(OpenImageDenoiseAMD)
endif()

install(TARGETS OpenImageDenoiseAMD
  EXPORT
    OpenImageDenoiseAMD_Exports
  ARCHIVE
    DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT lib
)
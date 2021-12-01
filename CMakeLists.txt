include(CMakeDependentOption)
cmake_minimum_required(VERSION 3.10)
project(SpiceQL VERSION 0.0.1 DESCRIPTION "Syntax Sugar for cspice")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 20)

# Variables required by multiple build options
set(SPICEQL_BUILD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/include/")

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "$ENV{CONDA_PREFIX}")
endif()
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")

#################
# Library Build #
#################

option (SPICEQL_BUILD_LIB "Build the SpiceQL Library" ON)

if(SPICEQL_BUILD_LIB)

  set(JSON_BuildTests OFF CACHE INTERNAL "")

  add_subdirectory("submodules/gularkfilesystem")
  add_subdirectory("submodules/json")

  find_package(CSpice REQUIRED)
  find_package(fmt REQUIRED)

  set(SPICEQL_INSTALL_INCLUDE_DIR "include/SpiceQL")
  set(SPICEQL_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/utils.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/io.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/query.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/spice_types.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/config.cpp)

  set(SPICEQL_HEADER_FILES ${SPICEQL_BUILD_INCLUDE_DIR}/sugar_spice.h
                              ${SPICEQL_BUILD_INCLUDE_DIR}/utils.h
                              ${SPICEQL_BUILD_INCLUDE_DIR}/io.h
                              ${SPICEQL_BUILD_INCLUDE_DIR}/spice_types.h
                              ${SPICEQL_BUILD_INCLUDE_DIR}/query.h
                            ${SPICEQL_BUILD_INCLUDE_DIR}/config.h)

  set(SPICEQL_CONFIG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/clem1.json
                              ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/galileo.json
                              ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/lro.json
                              ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/mess.json
                              ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/juno.json)

  add_library(SpiceQL SHARED ${SPICEQL_SRC_FILES})

  ADD_DEFINITIONS(-D_SOURCE_PREFIX="${CMAKE_CURRENT_SOURCE_DIR}")


  set_target_properties(SpiceQL PROPERTIES
                                  VERSION ${PROJECT_VERSION}
                                  SOVERSION 0)

  target_include_directories(SpiceQL
                            PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/include>
                            $<INSTALL_INTERFACE:include>
                            PRIVATE)

  target_link_libraries(SpiceQL
                        PUBLIC
                        ghc_filesystem
                        fmt::fmt-header-only
                        nlohmann_json::nlohmann_json
                        PRIVATE
                        CSpice::cspice
                        )

  install(TARGETS SpiceQL LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(DIRECTORY ${SPICEQL_INCLUDE_DIR} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  # Generate the package config
  configure_file(cmake/config.cmake.in
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
                @ONLY)

  # Install the package config
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

  # Install the headers
  install(FILES ${SPICEQL_HEADER_FILES} DESTINATION ${SPICEQL_INSTALL_INCLUDE_DIR})

  # Install the json db files
  install(FILES ${SPICEQL_CONFIG_FILES} DESTINATION "etc/SpiceQL/db")

  # Install the library
  install(TARGETS SpiceQL nlohmann_json
          EXPORT spiceQLTargets
          LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
          INCLUDES DESTINATION ${SPICEQL_INSTALL_INCLUDE_DIR})

  # Install the target
  install(EXPORT spiceQLTargets
          NAMESPACE spice::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

else()
  message(STATUS "Skipping Library")
endif()

###############
# Tests Build #
###############

cmake_dependent_option (SPICEQL_BUILD_TESTS "Build the SpiceQL Tests" ON SPICEQL_BUILD_LIB OFF)

if(SPICEQL_BUILD_TESTS)
  include(GoogleTest)
  include(cmake/gtest.cmake)
  include(CTest)

  find_package (Threads)
  enable_testing()
  add_subdirectory(SpiceQL/tests)
else()
  message(STATUS "Skipping Tests")
endif()

##################
# Bindings Build #
##################

cmake_dependent_option (SPICEQL_BUILD_BINDINGS "Build the SpiceQL Bindings in Other Languages" ON SPICEQL_BUILD_LIB OFF)

if(SPICEQL_BUILD_BINDINGS)
  add_subdirectory(bindings)
else()
  message(STATUS "Skipping Bindings")
endif()

##############
# Docs Build #
##############

option (SPICEQL_BUILD_DOCS "Build the SpiceQL Docs" ON)

if(SPICEQL_BUILD_DOCS)
  add_subdirectory ("docs")
else()
  message(STATUS "Skipping Docs")
endif()

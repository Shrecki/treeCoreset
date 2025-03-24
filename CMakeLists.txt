cmake_minimum_required(VERSION 3.20)
project(treeCoresetProj)

# Add option to detect when building through scikit-build
option(SKBUILD "Build with scikit-build" OFF)
option(BUNDLE_DEPENDENCIES "Bundle dependencies in wheel" ON)

include(cmake-scripts/code-coverage.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensures we use all optimizations.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -msse4.1 -march=native")

# Required to link python lib
SET(GCC_COVERAGE_COMPILE_FLAGS "-Wl, --no-undefined")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

# Find Python
#set(Python_ROOT_DIR "$ENV{PYTHON_ROOT}" CACHE PATH "Python root directory")
#set(Python_EXECUTABLE "$ENV{PYBIN}" CACHE FILEPATH "Python executable")
#set(Python_INCLUDE_DIR "$ENV{PYTHON_INCLUDE}" CACHE PATH "Python include directory")
#set(Python_LIBRARY "$ENV{PYTHON_LIB}" CACHE FILEPATH "Python library")

set(CMAKE_FIND_DEBUG_MODE ON)
find_package(Python 3.10 COMPONENTS Interpreter Development.Module REQUIRED)


# Static linking for wheel builds
if(BUNDLE_DEPENDENCIES AND UNIX AND NOT APPLE)
    # Force static linking for bundled wheels
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    set(BUILD_SHARED_LIBS OFF)
    # Set RPATH to $ORIGIN for the installed library
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

# Only include tests when not building the wheel
if(NOT SKBUILD)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()


# Find Boost - prefer static libs for wheels
if(BUNDLE_DEPENDENCIES)
    set(Boost_USE_STATIC_LIBS ON)
endif()
find_package(Boost 1.79 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
message("boost lib: ${Boost_LIBRARIES}, inc:${Boost_INCLUDE_DIR}")

# Find ZeroMQ - prefer static libs for wheels
# Find ZeroMQ - prefer static libs for wheels
if(BUNDLE_DEPENDENCIES)
    set(ZeroMQ_USE_STATIC_LIBS ON)
endif()

# Add hints for ZeroMQ location
set(ZeroMQ_ROOT_DIR "${ZeroMQ_ROOT}" CACHE PATH "ZeroMQ root directory")

# Try to find ZeroMQ with a config file first
find_package(ZeroMQ CONFIG QUIET)

# If not found with config, try the manual approach
if(NOT ZeroMQ_FOUND)
    find_path(ZeroMQ_INCLUDE_DIR
            NAMES zmq.hpp
            PATHS ${ZeroMQ_ROOT_DIR}/include ${PC_ZeroMQ_INCLUDE_DIRS}
            PATH_SUFFIXES include
    )
    find_library(ZeroMQ_LIBRARY
            NAMES zmq libzmq
            PATHS ${ZeroMQ_ROOT_DIR}/lib ${PC_ZeroMQ_LIBRARY_DIRS}
            PATH_SUFFIXES lib
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(ZeroMQ DEFAULT_MSG ZeroMQ_LIBRARY ZeroMQ_INCLUDE_DIR)

    if(ZeroMQ_FOUND AND NOT TARGET ZeroMQ::ZeroMQ)
        add_library(ZeroMQ::ZeroMQ UNKNOWN IMPORTED)
        set_target_properties(ZeroMQ::ZeroMQ PROPERTIES
                IMPORTED_LOCATION "${ZeroMQ_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ZeroMQ_INCLUDE_DIR}"
        )
    endif()
endif()

# Print diagnostic info
message(STATUS "ZeroMQ include dir: ${ZeroMQ_INCLUDE_DIR}")
message(STATUS "ZeroMQ library: ${ZeroMQ_LIBRARY}")
if(NOT ZeroMQ_FOUND)
    message(FATAL_ERROR "ZeroMQ not found")
endif()

set(SOURCES
        src/clustering/Node.h src/clustering/Node.cpp
        src/clustering/Point.h src/clustering/Point.cpp
        src/clustering/RandomGenerator.h src/clustering/RandomGenerator.cpp
        src/clustering/coreset_algorithms.h src/clustering/coreset_algorithms.cpp
        src/clustering/ClusteredPoints.h src/clustering/ClusteredPoints.cpp
        src/clustering/kmeansplusplus.h src/clustering/kmeansplusplus.cpp
        src/network/Requests.h
        src/Errors.h
        src/network/ClientMessaging.h src/network/ClientMessaging.cpp
        src/network/ServerMessaging.h src/network/ServerMessaging.cpp
        src/network/MessagingUtils.h src/network/MessagingUtils.cpp)

# Only build the main executable when not building a wheel
if(NOT SKBUILD)
    add_executable(treeCoreset main.cpp ${SOURCES})
    target_include_directories(treeCoreset PRIVATE eigen)
    target_include_directories(treeCoreset PUBLIC ${ZeroMQ_INCLUDE_DIR})
    target_link_libraries(treeCoreset PUBLIC ${ZeroMQ_LIBRARY} ${Boost_LIBRARIES})

    # Add unit tests
    add_executable(unit_test ${SOURCES}
            test/clustering/point_test.cpp test/main_test.cpp test/clustering/node_test.cpp
            test/mocks/mock_RandomGenerator.cpp test/mocks/mock_RandomGenerator.h
            test/clustering/math_framework.cpp test/clustering/math_framework.h test/clustering/coreset_algorithms_test.cpp test/clustering/clustered_points_test.cpp
            test/clustering/kmeansplusplus_test.cpp test/clustering/utils.cpp test/utils.h
            test/network/client_messaging_test.cpp
            test/network/server_messaging_test.cpp)
    target_link_libraries(unit_test PRIVATE gtest_main)
    target_link_libraries(unit_test PRIVATE gmock_main)
    target_link_libraries(unit_test PRIVATE ${ZeroMQ_LIBRARY} ${Boost_LIBRARIES})
    target_include_directories(unit_test PRIVATE src)
    target_include_directories(unit_test PRIVATE eigen)
    target_code_coverage(unit_test target_code_coverage_PUBLIC)
    include(GoogleTest)
    gtest_discover_tests(unit_test)
endif()

# Without this, any build libraries automatically have names "lib{x}.so"
set(CMAKE_SHARED_MODULE_PREFIX "")

# Python module
set(SOURCE_CLIENT src/network/ClientMessaging.cpp src/network/ClientMessaging.h
                 src/network/MessagingUtils.h src/network/MessagingUtils.cpp
                 src/network/Requests.h src/Errors.h)

# Build the client_coreset module for Python
add_library(client_coreset MODULE src/clustering/Node.h src/clustering/Node.cpp
        src/clustering/Point.h src/clustering/Point.cpp
        src/clustering/RandomGenerator.h src/clustering/RandomGenerator.cpp
        src/clustering/coreset_algorithms.h src/clustering/coreset_algorithms.cpp
        src/clustering/ClusteredPoints.h src/clustering/ClusteredPoints.cpp
        src/clustering/kmeansplusplus.h src/clustering/kmeansplusplus.cpp
        src/Errors.h)
target_compile_definitions(client_coreset PUBLIC PYTHON_BIND=1)
target_link_libraries(client_coreset ${Boost_LIBRARIES} ${Python_LIBRARIES} ${ZeroMQ_LIBRARY})
set(PYBIND11_PYTHON_VERSION 3.10 CACHE STRING "")
target_include_directories(client_coreset PRIVATE ${Python_INCLUDE_DIRS} eigen pybind11 ${ZeroMQ_INCLUDE_DIR})

set_target_properties(client_coreset PROPERTIES
        PREFIX ""  # No 'lib' prefix
        OUTPUT_NAME "client_coreset"
        SUFFIX ".so")

# Installation for skbuild
if(SKBUILD)
    install(TARGETS client_coreset DESTINATION treeCoreset)
endif()

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

# Find Python
find_package(Python 3.10 COMPONENTS Interpreter Development REQUIRED)

# Find Boost - prefer static libs for wheels
if(BUNDLE_DEPENDENCIES)
    set(Boost_USE_STATIC_LIBS ON)
endif()
find_package(Boost 1.79 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
message("boost lib: ${Boost_LIBRARIES}, inc:${Boost_INCLUDE_DIR}")

# Find ZeroMQ - prefer static libs for wheels
if(BUNDLE_DEPENDENCIES)
    set(ZeroMQ_USE_STATIC_LIBS ON)
endif()
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
)
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
)

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
target_include_directories(client_coreset PRIVATE ${Python_INCLUDE_DIRS} eigen pybind11)

# Installation for skbuild
if(SKBUILD)
    install(TARGETS client_coreset DESTINATION treeCoreset)
endif()

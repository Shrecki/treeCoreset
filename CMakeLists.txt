cmake_minimum_required(VERSION 3.20)
project(treeCoresetProj)

include(cmake-scripts/code-coverage.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensures we use all optimizations.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -msse4.1 -march=native")

# Required to link python lib
SET(GCC_COVERAGE_COMPILE_FLAGS "-Wl, --no-undefined")
#SET(GCC_COVERAGE_LINK_FLAGS    "-lboost_numpy38 -lboost_python38")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")


include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(Boost COMPONENTS program_options system filesystem REQUIRED PATHS)
find_package(PythonLibs 3.10 REQUIRED)
find_package(Python 3.10 COMPONENTS Interpreter Development REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
message("boost lib: ${Boost_LIBRARIES}, inc:${Boost_INCLUDE_DIR}")

link_libraries('boost_filesystem')
link_libraries('boost_system')


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

find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )


# For main program
add_executable(treeCoreset main.cpp ${SOURCES})
target_include_directories(treeCoreset PRIVATE eigen)
target_include_directories(treeCoreset PUBLIC ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(treeCoreset PUBLIC ${ZeroMQ_LIBRARY} ${Boost_LIBRARIES})
#target_compile_features(treeCoreset, PUBLIC, cxx_std_17)



# For unit tests
add_executable(unit_test ${SOURCES}
        test/clustering/point_test.cpp test/main_test.cpp test/clustering/node_test.cpp
        test/mocks/mock_RandomGenerator.cpp test/mocks/mock_RandomGenerator.h
        test/clustering/math_framework.cpp test/clustering/math_framework.h test/clustering/coreset_algorithms_test.cpp test/clustering/clustered_points_test.cpp
        test/clustering/kmeansplusplus_test.cpp test/clustering/utils.cpp test/utils.h
        test/network/client_messaging_test.cpp
        test/network/server_messaging_test.cpp)
target_link_libraries(unit_test PRIVATE gtest_main)
target_link_libraries(unit_test PRIVATE gmock_main)
target_link_libraries(unit_test PRIVATE ${ZeroMQ_LIBRARY})
target_link_libraries(unit_test PRIVATE ${Boost_LIBRARIES})

target_include_directories(unit_test PRIVATE src)
target_include_directories(unit_test PRIVATE eigen)
target_code_coverage(unit_test target_code_coverage_PUBLIC)
#target_compile_features(unit_test, PUBLIC, cxx_std_17)

include(GoogleTest)
gtest_discover_tests(unit_test)



# Without this, any build libraries automatically have names "lib{x}.so"
set(CMAKE_SHARED_MODULE_PREFIX "")

#add_compile_definitions(PYTHON_BIND=1)


# Add a shared module - modules are intended to be imported at runtime.
# - This is where you add the source files
set(SOURCE_CLIENT src/network/ClientMessaging.cpp src/network/ClientMessaging.h
                  src/network/MessagingUtils.h src/network/MessagingUtils.cpp
                  src/network/Requests.h src/Errors.h)
add_library(client_coreset MODULE src/clustering/Node.h src/clustering/Node.cpp
        src/clustering/Point.h src/clustering/Point.cpp
        src/clustering/RandomGenerator.h src/clustering/RandomGenerator.cpp
        src/clustering/coreset_algorithms.h src/clustering/coreset_algorithms.cpp
        src/clustering/ClusteredPoints.h src/clustering/ClusteredPoints.cpp
        src/clustering/kmeansplusplus.h src/clustering/kmeansplusplus.cpp
        src/Errors.h)
target_compile_definitions(client_coreset PUBLIC PYTHON_BIND=1)
target_link_libraries(client_coreset ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} ${ZeroMQ_LIBRARY})
set(PYBIND11_PYTHON_VERSION 3.10 CACHE STRING "")
target_include_directories(client_coreset PRIVATE ${PYTHON_INCLUDE_DIRS} eigen pybind11)
cmake_minimum_required(VERSION 3.20)
project(treeCoresetProj)

set(CMAKE_CXX_STANDARD 17)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -msse4.1 -march=native")

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)



set( boost_INSTALL "/usr/local/share/boost_1_77_0/build-boost")
set( boost_INCLUDE_DIR ${boost_INSTALL}/include )
set( boost_LIB_DIR ${boost_INSTALL}/lib )
set( Boost_NO_SYSTEM_PATHS on CACHE BOOL "Do not search system for Boost" )

find_package(Boost REQUIRED PATHS ${boost_INSTALL})
include_directories(${boost_INCLUDE_DIR})
message("boost lib: ${boost_LIB_DIR}, inc:${boost_INCLUDE_DIR}")


set(SOURCES
        src/Node.cpp src/Node.h
        src/Point.cpp src/Point.h
        src/RandomGenerator.cpp src/RandomGenerator.h
        src/coreset_algorithms.h src/coreset_algorithms.cpp
        src/ClusteredPoints.cpp src/ClusteredPoints.h
        src/kmeansplusplus.cpp src/kmeansplusplus.h
        src/Requests.h src/NetworkProtocol.cpp src/NetworkProtocol.h)


set( boost_INSTALL "/usr/local/share/boost_1_77_0/build-boost")

find_package(PkgConfig)
pkg_check_modules(PC_ZeroMQ QUIET zmq)
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )
# For main program
add_executable(treeCoreset main.cpp ${SOURCES} src/NetworkProtocol.cpp src/NetworkProtocol.h)
target_include_directories(treeCoreset PRIVATE eigen)
target_include_directories(treeCoreset PUBLIC ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(treeCoreset PUBLIC ${ZeroMQ_LIBRARY})


# For unit tests
add_executable(unit_test ${SOURCES}
        test/point_test.cpp test/main_test.cpp test/node_test.cpp
        test/mocks/mock_RandomGenerator.cpp test/mocks/mock_RandomGenerator.h
        test/math_framework.cpp test/math_framework.h test/coreset_algorithms_test.cpp test/clustered_points_test.cpp
        test/kmeansplusplus_test.cpp test/utils.cpp test/utils.h )
target_link_libraries(unit_test gtest_main)
target_link_libraries(unit_test gmock_main)
target_link_libraries(unit_test ${ZeroMQ_LIBRARY})
target_include_directories(unit_test PRIVATE src)
target_include_directories(unit_test PRIVATE eigen)

include(GoogleTest)
gtest_discover_tests(unit_test)


cmake_minimum_required(VERSION 3.20)
project(untitled)

set(CMAKE_CXX_STANDARD 14)

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


set(SOURCES src/Node.cpp src/Node.h src/Point.cpp src/Point.h)

# For main program
add_executable(untitled main.cpp ${SOURCES})
target_include_directories(untitled PRIVATE eigen)

# For unit tests
add_executable(unit_test ${SOURCES} ${Boost_LIBRARIES}
        test/point_test.cpp test/main_test.cpp test/node_test.cpp
        test/mock_RandomGenerator.cpp test/mock_RandomGenerator.h src/RandomGenerator.cpp src/RandomGenerator.h
        test/math_framework.cpp test/math_framework.h)
target_link_libraries(unit_test gtest_main)
target_link_libraries(unit_test gmock_main)
target_include_directories(unit_test PRIVATE src)
target_include_directories(unit_test PRIVATE eigen)

include(GoogleTest)
gtest_discover_tests(unit_test)
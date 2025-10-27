cmake_minimum_required(VERSION 3.16)
project(InspectionQuerySystem)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set PKG_CONFIG_PATH for Homebrew-installed libraries
set(ENV{PKG_CONFIG_PATH} "/opt/homebrew/lib/pkgconfig:/opt/homebrew/Cellar/libpq/18.0/lib/pkgconfig")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(GFLAGS REQUIRED gflags)

# Find nlohmann/json
find_package(nlohmann_json REQUIRED)

# Include directories and library directories
include_directories(${PQXX_INCLUDE_DIRS} ${GFLAGS_INCLUDE_DIRS})
link_directories(${PQXX_LIBRARY_DIRS} ${GFLAGS_LIBRARY_DIRS})
include_directories(src)

# Create library for shared components
add_library(query_lib
    src/database/DatabaseManager.cpp
    src/geometry/Rectangle.cpp
    src/geometry/Point.cpp
    src/query/QueryEngine.cpp
    src/query/JsonParser.cpp
    src/query/QueryResult.cpp
)

target_link_libraries(query_lib ${PQXX_LIBRARIES} ${GFLAGS_LIBRARIES} nlohmann_json::nlohmann_json)
target_compile_options(query_lib PRIVATE ${PQXX_CFLAGS_OTHER} ${GFLAGS_CFLAGS_OTHER})

# Task 2: Query Processor executable
add_executable(query_processor
    src/apps/query_processor.cpp
)

target_link_libraries(query_processor query_lib ${PQXX_LIBRARIES} ${GFLAGS_LIBRARIES} nlohmann_json::nlohmann_json)
target_compile_options(query_processor PRIVATE ${PQXX_CFLAGS_OTHER} ${GFLAGS_CFLAGS_OTHER})

# Set compiler flags for debugging and warnings
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find GTest for testing
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add test executable
add_executable(test_query_engine
    test_query_engine.cpp
)

target_link_libraries(test_query_engine 
    query_lib 
    ${PQXX_LIBRARIES} 
    ${GFLAGS_LIBRARIES} 
    nlohmann_json::nlohmann_json
    ${GTEST_LIBRARIES}
    pthread
)

target_compile_options(test_query_engine PRIVATE ${PQXX_CFLAGS_OTHER} ${GFLAGS_CFLAGS_OTHER})

# Add random test executable
add_executable(test_random_queries
    test_random_queries.cpp
)

target_link_libraries(test_random_queries 
    query_lib 
    ${PQXX_LIBRARIES} 
    ${GFLAGS_LIBRARIES} 
    nlohmann_json::nlohmann_json
    ${GTEST_LIBRARIES}
    pthread
)

target_compile_options(test_random_queries PRIVATE ${PQXX_CFLAGS_OTHER} ${GFLAGS_CFLAGS_OTHER})

# Add custom target for running GTest
add_custom_target(gtest
    COMMAND ./test_query_engine --gtest_print_time=1
    DEPENDS test_query_engine
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running GTest test suite"
)

# Add custom target for running random tests (alias: gtest2)
add_custom_target(random_test
    COMMAND ./test_random_queries --gtest_print_time=1
    DEPENDS test_random_queries
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running random query test suite (data bounds auto-detected)"
)

# Add alias gtest2 for random tests
add_custom_target(gtest2
    COMMAND ./test_random_queries --gtest_print_time=1
    DEPENDS test_random_queries
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running random query test suite (gtest2 alias)"
)
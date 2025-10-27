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
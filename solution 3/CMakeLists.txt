cmake_minimum_required(VERSION 3.16)
project(ExtendedQueryProcessor)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set PKG_CONFIG_PATH for Homebrew-installed libraries
set(ENV{PKG_CONFIG_PATH} "/opt/homebrew/lib/pkgconfig:/opt/homebrew/Cellar/libpq/18.0/lib/pkgconfig")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# Find nlohmann/json
find_package(nlohmann_json REQUIRED)

# Include directories and library directories
include_directories(${PQXX_INCLUDE_DIRS})
link_directories(${PQXX_LIBRARY_DIRS})

# Directly include solution 2 source files (reuse without duplication)
set(SOLUTION2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../solution 2/src")

file(GLOB_RECURSE SOLUTION2_SOURCES
    "${SOLUTION2_DIR}/database/*.cpp"
    "${SOLUTION2_DIR}/geometry/*.cpp" 
    "${SOLUTION2_DIR}/query/*.cpp"
)

# Collect solution 3 extension source files
file(GLOB_RECURSE SOLUTION3_SOURCES
    "src/operators/*.cpp"
    "src/query/*.cpp"
)

# Create extended query processor executable
add_executable(extended_query_processor
    ${SOLUTION2_SOURCES}      # ← Reuse solution 2 code directly
    ${SOLUTION3_SOURCES}      # ← Add solution 3 extensions
    src/apps/extended_query_processor.cpp
)

# Include directories (both solution 2 and solution 3)
target_include_directories(extended_query_processor PRIVATE
    src                       # Solution 3 headers
    "${SOLUTION2_DIR}"        # Solution 2 headers  
    ${PQXX_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(extended_query_processor 
    ${PQXX_LIBRARIES} 
    nlohmann_json::nlohmann_json
)

# Compiler flags
target_compile_options(extended_query_processor PRIVATE ${PQXX_CFLAGS_OTHER})

# Set compiler flags for debugging and warnings
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
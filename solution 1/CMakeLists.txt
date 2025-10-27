cmake_minimum_required(VERSION 3.16)
project(InspectionRegionSystem)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set PKG_CONFIG_PATH for Homebrew-installed libraries
set(ENV{PKG_CONFIG_PATH} "/opt/homebrew/lib/pkgconfig:/opt/homebrew/Cellar/libpq/18.0/lib/pkgconfig")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# Find gflags using pkg-config since it's installed via Homebrew
pkg_check_modules(GFLAGS REQUIRED gflags)

# Include directories and library directories
include_directories(${PQXX_INCLUDE_DIRS} ${GFLAGS_INCLUDE_DIRS})
link_directories(${PQXX_LIBRARY_DIRS} ${GFLAGS_LIBRARY_DIRS})
include_directories(src)

# Create library for shared components
add_library(inspection_lib
    src/database/DatabaseManager.cpp
    src/data/DataLoader.cpp
    src/data/Point.cpp
)

target_link_libraries(inspection_lib ${PQXX_LIBRARIES} ${GFLAGS_LIBRARIES})
target_compile_options(inspection_lib PRIVATE ${PQXX_CFLAGS_OTHER} ${GFLAGS_CFLAGS_OTHER})

# Task 1: Data Loader executable
add_executable(data_loader
    src/apps/data_loader.cpp
)

target_link_libraries(data_loader inspection_lib ${PQXX_LIBRARIES} ${GFLAGS_LIBRARIES})
target_compile_options(data_loader PRIVATE ${PQXX_CFLAGS_OTHER} ${GFLAGS_CFLAGS_OTHER})

# Set compiler flags for debugging and warnings
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
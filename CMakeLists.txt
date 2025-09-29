cmake_minimum_required(VERSION 3.16)
project(BioMesh VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Include directories
include_directories(include)

# Find packages
find_package(Threads REQUIRED)

# Source files
set(BIOMESH_SOURCES
    src/biomesh/pdb_parser/pdb_parser.cpp
    src/biomesh/atom/atom.cpp
    src/biomesh/bounding_box/bounding_box.cpp
    src/biomesh/voxel_grid/voxel_grid.cpp
    src/biomesh/mesh_generator/mesh_generator.cpp
    src/biomesh/exporter/gid_exporter.cpp
    src/biomesh/exporter/vtk_exporter.cpp
)

# Create BioMesh library
add_library(biomesh ${BIOMESH_SOURCES})
target_include_directories(biomesh PUBLIC include)
target_link_libraries(biomesh Threads::Threads)

# Main executable (example)
add_executable(biomesh_app examples/main.cpp)
target_link_libraries(biomesh_app biomesh)

# Optional: GoogleTest (only if available)
find_package(GTest QUIET)
if(GTest_FOUND)
    enable_testing()

    set(TEST_SOURCES
        tests/test_pdb_parser.cpp
        tests/test_atom.cpp
        tests/test_bounding_box.cpp
        tests/test_voxel_grid.cpp
        tests/test_mesh_generator.cpp
        tests/test_exporter.cpp
    )

    add_executable(biomesh_tests ${TEST_SOURCES})
    target_link_libraries(biomesh_tests biomesh GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(biomesh_tests)
    
    message(STATUS "GoogleTest found - tests enabled")
else()
    message(STATUS "GoogleTest not found - tests disabled. Install libgtest-dev to enable tests.")
endif()

# Install rules
install(TARGETS biomesh biomesh_app
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/biomesh
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)
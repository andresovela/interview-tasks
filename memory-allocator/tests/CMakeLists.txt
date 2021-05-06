enable_testing()
include(CTest)

# Declares all the source files in the ${SOURCE_FILES} variable
include(${CMAKE_SOURCE_DIR}/cmake/source_files.cmake)

# Declares all the include paths in the ${INCLUDE_PATHS} variable
include(${CMAKE_SOURCE_DIR}/cmake/include_directories.cmake)

add_executable(tests test_allocator_runner.c test_allocator.c ${SOURCE_FILES})

target_include_directories(tests PUBLIC ${INCLUDE_PATHS})
target_link_libraries(tests unity)

add_test(tests test_allocator_runner.c test_allocator.c)
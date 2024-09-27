cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
  SOURCE_DIR     ${CMAKE_SOURCE_DIR}/third_party
)
FetchContent_MakeAvailable(googletest)



file(GLOB_RECURSE TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*/*test.cc")
file(GLOB_RECURSE SRC_SOURCES "${PROJECT_SOURCE_DIR}/src/*/*.cc")

# set(CODING_SRC ${PROJECT_SOURCE_DIR}/src/coding/coding.cc)
# set(FILES_SRC ${PROJECT_SOURCE_DIR}/src/db/files.cc)
# set(ARENA_SRC ${PROJECT_SOURCE_DIR}/src/tools/arena.cc)
# set(KEY_SRC ${PROJECT_SOURCE_DIR}/src/db/key.cc)
# set(MEMTABLE_SRC ${PROJECT_SOURCE_DIR}/src/db/memtable.cc)
# set(MEMTABLE_SRC ${PROJECT_SOURCE_DIR}/src/db/memtable.cc)
# set(MEMTABLE_SRC ${PROJECT_SOURCE_DIR}/src/db/memtable.cc)

foreach (test_source ${TEST_SOURCES})
    # Create a human readable name.
    get_filename_component(test_filename ${test_source} NAME)
    # 将test_filename的.cpp后缀去除 去除后的名字放在test_name中
    string(REPLACE ".cc" "" test_name ${test_filename})

    # Add the test target separately and as part of "make check-tests".
    add_executable(${test_name} ${SRC_SOURCES} ${test_source})

    target_link_libraries(${test_name} gtest gmock_main)

    # Set test target properties and dependencies.
    set_target_properties(${test_name}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
            COMMAND ${test_name}
            )
endforeach ()

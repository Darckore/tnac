cmake_minimum_required(VERSION 3.23)
find_package(Threads REQUIRED)

include(ExternalProject)

set(GTEST_DL "googletest-download")
configure_file(CMakeLists.txt.in "${CMAKE_BINARY_DIR}/${GTEST_DL}/CMakeLists.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
               WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${GTEST_DL}")
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${GTEST_DL}")

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory("${OPT_GTEST_DIR}"
                 "${CMAKE_BINARY_DIR}/googletest-build")
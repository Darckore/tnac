cmake_minimum_required(VERSION 3.23)
find_package(Threads REQUIRED)

include(ExternalProject)

set(UTILS_DL "utils-download")
configure_file(CMakeLists.txt.in "${CMAKE_BINARY_DIR}/${UTILS_DL}/CMakeLists.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
               WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${UTILS_DL}")
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${UTILS_DL}")

add_subdirectory("${OPT_DUTILS_DIR}"
                 "${CMAKE_BINARY_DIR}/utilities-build")
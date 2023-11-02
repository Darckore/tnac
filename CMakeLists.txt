cmake_minimum_required(VERSION 3.26)

set(OPT_PCH_NAME "pch.h")
set(OPT_STD "cxx_std_23")
set(OPT_UTILS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_cmake")
set(OPT_DATA_DIR "data")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

if (NOT DEFINED BUILT_FROM_ROOT)
  set(BUILT_FROM_ROOT OFF)
  if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(BUILT_FROM_ROOT ON)
  endif()
endif()

set(OPT_GTEST_DIR "${CMAKE_SOURCE_DIR}/../_third_party/gtest" CACHE PATH "Gtest repo will be cloned here")
set(OPT_DUTILS_DIR "${CMAKE_SOURCE_DIR}/../_deps/utils" CACHE PATH "Utils repo will be cloned here")
option(OPT_TESTS "Whether or not to build tests" ${BUILT_FROM_ROOT})
option(OPT_APP "Whether or not to build the application" ${BUILT_FROM_ROOT})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(THIRD_PARTY_DIR third_party)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER cmake)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

set(APP_TARGET tnac)
set(LIBCORE_TARGET tnac_lib)
set(COMMON_TARGET tnac_common)
set(FRONTEND_TARGET tnac_front)
set(EVAL_TARGET tnac_eval)
set(RUNTIME_TARGET tnac_runtime)
set(TESTS_TARGET tnac_tests)

project(tnac_project CXX)

set(TARGET_NAME ${COMMON_TARGET})
add_subdirectory("${TARGET_NAME}")

set(TARGET_NAME ${FRONTEND_TARGET})
add_subdirectory("${TARGET_NAME}")
target_link_libraries(${TARGET_NAME} ${COMMON_TARGET})

set(TARGET_NAME ${EVAL_TARGET})
add_subdirectory("${TARGET_NAME}")
target_link_libraries(${TARGET_NAME} ${COMMON_TARGET})

set(TARGET_NAME ${LIBCORE_TARGET})
add_subdirectory("${TARGET_NAME}")
target_link_libraries(${TARGET_NAME} ${COMMON_TARGET} ${FRONTEND_TARGET} ${EVAL_TARGET})

set(TARGET_NAME ${RUNTIME_TARGET})
add_subdirectory("${TARGET_NAME}")
target_link_libraries(${TARGET_NAME} ${COMMON_TARGET} ${LIBCORE_TARGET} ${FRONTEND_TARGET} ${EVAL_TARGET})

if(OPT_APP)
  set(TARGET_NAME ${APP_TARGET})
  add_subdirectory("${TARGET_NAME}")
  target_link_libraries(${TARGET_NAME} ${RUNTIME_TARGET} ${COMMON_TARGET} ${LIBCORE_TARGET} ${FRONTEND_TARGET} ${EVAL_TARGET})
endif()

if(OPT_TESTS AND NOT TARGET gtest)
  add_subdirectory("${OPT_UTILS_DIR}/${THIRD_PARTY_DIR}/gtest" gtest)
endif()

if(OPT_TESTS AND TARGET gtest)
  enable_testing()
  set(TARGET_NAME ${TESTS_TARGET})
  set(OPT_DATA_DIR "tests")
  add_subdirectory("${TARGET_NAME}")
  target_link_libraries(${TARGET_NAME} ${RUNTIME_TARGET} ${COMMON_TARGET} ${LIBCORE_TARGET} ${FRONTEND_TARGET} ${EVAL_TARGET})
  
  if(OPT_APP)
    add_dependencies(${TARGET_NAME} ${APP_TARGET})
  endif()
  
  set_target_properties(gtest PROPERTIES FOLDER ${THIRD_PARTY_DIR}/gtest)
  set_target_properties(gmock PROPERTIES FOLDER ${THIRD_PARTY_DIR}/gtest)
  set_target_properties(gtest_main PROPERTIES FOLDER ${THIRD_PARTY_DIR}/gtest)
  set_target_properties(gmock_main PROPERTIES FOLDER ${THIRD_PARTY_DIR}/gtest)
  
  target_compile_definitions(${TARGET_NAME} PUBLIC GTEST_BUILT)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(OPT_DATA_DIR "data")
endif()

set(TARGET_NAME utils)
set(OPT_TESTS OFF)
set(OPT_APP OFF)
add_subdirectory("${OPT_UTILS_DIR}/${THIRD_PARTY_DIR}/utils" ${TARGET_NAME})
target_link_libraries(${LIBCORE_TARGET} ${TARGET_NAME})
target_link_libraries(${RUNTIME_TARGET} ${TARGET_NAME})
target_link_libraries(${FRONTEND_TARGET} ${TARGET_NAME})
target_link_libraries(${EVAL_TARGET} ${TARGET_NAME})
target_link_libraries(${COMMON_TARGET} ${TARGET_NAME})

if(OPT_APP)
  target_link_libraries(${APP_TARGET} ${TARGET_NAME})
endif()

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER additional_deps/utils)

if(BUILT_FROM_ROOT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/_bin" CACHE PATH "Install dir" FORCE)
endif()
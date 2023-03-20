cmake_minimum_required(VERSION 3.23)
include("${OPT_UTILS_DIR}/utils.cmake")

project(${TARGET_NAME} CXX)

collect_sources(SOURCE_FILES HEADERS ADDITIONAL_FILES)
add_executable(${TARGET_NAME} ${SOURCE_FILES} ${HEADERS} ${OPT_PCH_NAME} ${ADDITIONAL_FILES})
set_build_opts(${TARGET_NAME} "${ADDITIONAL_FILES}")
make_src_groups("${SOURCE_FILES}" "${HEADERS}" "${ADDITIONAL_FILES}")

include(GoogleTest)

target_link_libraries(${TARGET_NAME} gtest gmock gtest_main gmock_main)
gtest_discover_tests(${TARGET_NAME})
add_compile_definitions(TEST_BASE_DIR="$<TARGET_FILE_DIR:${TARGET_NAME}>")

if(MSVC)
  set_target_properties(
    ${TARGET_NAME} PROPERTIES
    VS_DEBUGGER_COMMAND_ARGUMENTS "--gtest_filter=*")
endif()
cmake_minimum_required(VERSION 3.23)
include("${OPT_UTILS_DIR}/utils.cmake")

project(${TARGET_NAME} CXX)

collect_sources(SOURCE_FILES HEADERS ADDITIONAL_FILES)
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} ${HEADERS} ${OPT_PCH_NAME} ${ADDITIONAL_FILES})
set_build_opts(${TARGET_NAME} "${ADDITIONAL_FILES}")
make_src_groups("${SOURCE_FILES}" "${HEADERS}" "${ADDITIONAL_FILES}")
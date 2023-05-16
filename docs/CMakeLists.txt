cmake_minimum_required(VERSION 3.23)

set(DOCS_DIR ${CMAKE_SOURCE_DIR})
file(GLOB_RECURSE DOC_FILES
     LIST_DIRECTORIES false
     "${DOCS_DIR}/*.md"
     "${DOCS_DIR}/*/*.md" )

add_custom_target(${TARGET_NAME} SOURCES ${DOC_FILES})
source_group(TREE ${DOCS_DIR} FILES ${DOC_FILES})
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER documentation)
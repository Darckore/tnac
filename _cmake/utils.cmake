# Collect suitable source and header files inside the specified dir
function(collect_sources SOURCE_FILES HEADERS ADDITIONAL_FILES)
  file( GLOB_RECURSE SRC
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.c*")

  set(SOURCE_FILES "${SRC}" PARENT_SCOPE)
  set(SRC)

  file( GLOB_RECURSE SRC
        LIST_DIRECTORIES false      
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h*"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.inl"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*/*.h*"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*/*.inl")
  
  set(HEADERS "${SRC}" PARENT_SCOPE)
  set(SRC)
  
  file( GLOB_RECURSE SRC
        LIST_DIRECTORIES false      
        "${CMAKE_CURRENT_SOURCE_DIR}/${OPT_DATA_DIR}/*"
        "${CMAKE_CURRENT_SOURCE_DIR}/${OPT_DATA_DIR}/*/*" )
        
  set(ADDITIONAL_FILES "${SRC}" PARENT_SCOPE)
endfunction()

# Group items according to their locations and purpose (for IDEs)
function(make_src_groups SOURCE_FILES HEADERS ADDITIONAL_FILES)
  set(PCH_FILTER pch)
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS})
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ADDITIONAL_FILES})
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX ${PCH_FILTER} FILES ${OPT_PCH_NAME})
  
  file( GLOB_RECURSE GENERATED_PCH_FILES 
        LIST_DIRECTORIES false
        "${CMAKE_CURRENT_BINARY_DIR}/*pch.c*"
        "${CMAKE_CURRENT_BINARY_DIR}/*pch.h*")
  source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} PREFIX ${PCH_FILTER} FILES ${GENERATED_PCH_FILES})
endfunction()

# Build options
function(set_build_opts TARGET_NAME EXCLUDED_FILES)
  target_compile_features(${TARGET_NAME} PRIVATE ${OPT_STD})
  target_precompile_headers(${TARGET_NAME} PRIVATE ${OPT_PCH_NAME})
  target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

  if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX /JMC $<$<CONFIG:RELEASE>:/GL /Ot /Oi>)
    target_link_options(${TARGET_NAME} PRIVATE $<$<CONFIG:RELEASE>:/LTCG>)
    set_target_properties( ${TARGET_NAME} PROPERTIES
                           VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET_NAME}>")
    set_property(SOURCE ${EXCLUDED_FILES} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")
  else()
    target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
  endif()
  
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${OPT_DATA_DIR}")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/${OPT_DATA_DIR}"
                       "$<TARGET_FILE_DIR:${TARGET_NAME}>/${OPT_DATA_DIR}")
  endif()
endfunction()
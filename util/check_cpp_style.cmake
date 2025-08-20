cmake_minimum_required(VERSION 3.16)

if(NOT PROJECT_SOURCE_DIR)
  message(FATAL_ERROR "PROJECT_SOURCE_DIR not set")
endif()

if(NOT DEFINED APPLY)
  set(APPLY OFF)
endif()

find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format-10 clang-format-11 clang-format-12)
if(NOT CLANG_FORMAT_EXE)
  message(WARNING "clang-format not found; skipping C++ style check")
  return()
endif()

file(GLOB_RECURSE CPP_FILES
  "${PROJECT_SOURCE_DIR}/cpp/*.h"
  "${PROJECT_SOURCE_DIR}/cpp/*.hpp"
  "${PROJECT_SOURCE_DIR}/cpp/*.c"
  "${PROJECT_SOURCE_DIR}/cpp/*.cc"
  "${PROJECT_SOURCE_DIR}/cpp/*.cpp"
  "${PROJECT_SOURCE_DIR}/cpp/*.cxx"
  "${PROJECT_SOURCE_DIR}/cpp/*.cu"
  "${PROJECT_SOURCE_DIR}/cpp/*.cuh"
)

if(CPP_FILES)
  list(SORT CPP_FILES)
else()
  message(STATUS "No C++ sources found for style check")
  return()
endif()

set(DIFF_FOUND OFF)
foreach(f ${CPP_FILES})
  execute_process(
    COMMAND ${CLANG_FORMAT_EXE} -style=file ${f}
    OUTPUT_VARIABLE FORMATTED
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  file(READ ${f} ORIGINAL)
  if(NOT ORIGINAL STREQUAL FORMATTED)
    if(APPLY)
      file(WRITE ${f} "${FORMATTED}")
    else()
      message(STATUS "Needs formatting: ${f}")
      set(DIFF_FOUND ON)
    endif()
  endif()
endforeach()

if(DIFF_FOUND AND NOT APPLY)
  message(FATAL_ERROR "C++ style issues found. Run: make apply-cpp-style")
endif()

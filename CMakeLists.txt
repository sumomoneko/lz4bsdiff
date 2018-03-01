cmake_minimum_required(VERSION 3.0)

# for cmake debug,
# set(CMAKE_VERBOSE_MAKEFILE 1)

set(CMAKE_BUILD_TYPE Debug)
# for rtags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 11)

project(lz4bsdiff)

set(CLANG_WARNINIG_OPTIONS
  -Wall
  -Wextra
  )

set(CLANG_DEBUG_OPTIONS
  -g3
  )

set(GCC_WARNING_OPTIONS
  -Wall
  -Wextra
  -Wundef
  -Wcast-align
  #-Wcast-qual
  -Wfloat-equal
  -Winit-self
  -Wlogical-op
  #-Wmissing-declarations
  -Wpointer-arith
  -Wredundant-decls
  -Wshadow
  -Wswitch-enum
  -Wunsafe-loop-optimizations
  -Wwrite-strings
  )

set(GCC_CXX_WARNING_OPTIONS
  -Wctor-dtor-privacy
  -Weffc++
  -Wold-style-cast
  -Wreorder
  -Wuseless-cast
  -Wnon-virtual-dtor
  -Woverloaded-virtual
  -Wdelete-non-virtual-dtor
  )

set(GCC_C_WARNING_OPTIONS
  -Wbad-function-cast
  -Wno-sign-compare
  )

set(GCC_DEBUG_OPTIONS
  -g3
  -fstack-usage
  )

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${GCC_WARNING_OPTIONS};${GCC_CXX_WARNING_OPTIONS}>")
  add_compile_options("$<$<COMPILE_LANGUAGE:C>:${GCC_WARNING_OPTIONS};${GCC_C_WARNING_OPTIONS}>")
  add_compile_options("$<$<CONFIG:Debug>:${GCC_DEBUG_OPTIONS}>")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${CLANG_WARNINIG_OPTIONS}>")
  add_compile_options("$<$<COMPILE_LANGUAGE:C>:${CLANG_WARNINIG_OPTIONS}>")
  add_compile_options("$<$<CONFIG:Debug>:${CLANG_DEBUG_OPTIONS}>")
endif()

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/lz4/lib)
include_directories(${PROJECT_SOURCE_DIR}/bsdiff)


#
# lz4bsdiff lib
#
set (DIFFLIB_SOURCE_FILES
  ${PROJECT_SOURCE_DIR}/lz4/lib/lz4.h
  ${PROJECT_SOURCE_DIR}/lz4/lib/lz4.c
  ${PROJECT_SOURCE_DIR}/lz4/lib/lz4hc.h
  ${PROJECT_SOURCE_DIR}/lz4/lib/lz4hc.c

  ${PROJECT_SOURCE_DIR}/bsdiff/bsdiff.h
  ${PROJECT_SOURCE_DIR}/bsdiff/bsdiff.c

  ${PROJECT_SOURCE_DIR}/lz4bsdiff.h
  ${PROJECT_SOURCE_DIR}/lz4bsdiff.cpp
  )
add_library(lz4bsdiff ${DIFFLIB_SOURCE_FILES})


#
# lz4bsdiff executable
#
add_executable(lz4bsdiff-bin ${PROJECT_SOURCE_DIR}/lz4bsdiffmain.cpp)
target_link_libraries(lz4bsdiff-bin lz4bsdiff)
set_target_properties(lz4bsdiff-bin PROPERTIES OUTPUT_NAME lz4bsdiff)

#
# lz4bspatch lib
#
set(PATCHLIB_SOURCE_FILES
  ${PROJECT_SOURCE_DIR}/lz4/lib/lz4.h
  ${PROJECT_SOURCE_DIR}/lz4/lib/lz4.c

  ${PROJECT_SOURCE_DIR}/bsdiff/bspatch.h
  ${PROJECT_SOURCE_DIR}/bsdiff/bspatch.c

  ${PROJECT_SOURCE_DIR}/lz4bspatch.h
  ${PROJECT_SOURCE_DIR}/lz4bspatch.c
  )
add_library(lz4bspatch STATIC ${PATCHLIB_SOURCE_FILES})

#
# lz4bspatch executable
#
add_executable(lz4bspatch-bin ${PROJECT_SOURCE_DIR}/lz4bspatchmain.cpp)
target_link_libraries(lz4bspatch-bin lz4bspatch)
set_target_properties(lz4bspatch-bin PROPERTIES OUTPUT_NAME lz4bspatch)

#
# unitTest
#
include(CTest)
include(Dart)
enable_testing()

add_executable(unitTest ${PROJECT_SOURCE_DIR}/test/testmain.cpp )

target_link_libraries(unitTest lz4bspatch lz4bsdiff)
add_test(NAME unitTest COMMAND unitTest)

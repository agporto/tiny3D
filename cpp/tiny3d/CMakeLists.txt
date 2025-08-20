# tiny3d_set_tiny3d_lib_properties() sets properties for the Tiny3D lib itself.
# This should be used for all object libraries that make up the Tiny3D lib.
#
# In comparison, tiny3d_set_global_properties() sets properties for the Tiny3D
# lib, and targets that links to the Tiny3D lib, e.g pybind, unit tests, etc.
function(tiny3d_set_tiny3d_lib_properties target)
    cmake_parse_arguments(arg "HIDDEN" "" "" ${ARGN})
    set_target_properties(${target} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${TINY3D_ABI_VERSION}
    )
    if(NOT BUILD_SHARED_LIBS)
        target_compile_definitions(${target} PUBLIC TINY3D_STATIC)
    endif()
    if (arg_HIDDEN)
        set_target_properties(${target} PROPERTIES
                        CXX_VISIBILITY_PRESET hidden
                        VISIBILITY_INLINES_HIDDEN ON
                        )
    else ()
        target_compile_definitions(${target} PRIVATE TINY3D_ENABLE_DLL_EXPORTS)
    endif()

# Hide all 3rd party symbols, e.g. from header libraries.
get_target_property(TARGET_TYPE ${target} TYPE)
if (TARGET_TYPE STREQUAL SHARED_LIBRARY)
        if (APPLE)
            file(GENERATE OUTPUT libTiny3D.map CONTENT
                [=[*tiny3d*
                   *Tiny3D*
                ]=])
            target_link_options(${target} PRIVATE $<$<CONFIG:Release>:
                -Wl,-exported_symbols_list
                "${CMAKE_CURRENT_BINARY_DIR}/libTiny3D.map" >)
        elseif (UNIX)   # Linux
            file(GENERATE OUTPUT libTiny3D.map CONTENT
                [=[{
    global:
        *tiny3d*;
        extern "C++" {
             tiny3d::*;
        };
    local: *;
};]=])
            target_link_options(${target} PRIVATE $<$<CONFIG:Release>:
                "-Wl,--version-script=${CMAKE_CURRENT_BINARY_DIR}/libTiny3D.map" >)
        elseif (WIN32)
            # TODO(Sameer): Only export tiny3d symbols
        endif()
    endif()
endfunction()

# Configure a header file to pass the version settings to the source code
configure_file("${PROJECT_SOURCE_DIR}/cpp/tiny3d/Tiny3D.h.in"
               "${PROJECT_SOURCE_DIR}/cpp/tiny3d/Tiny3D.h")
configure_file("${PROJECT_SOURCE_DIR}/cpp/tiny3d/Tiny3DConfig.h.in"
               "${PROJECT_SOURCE_DIR}/cpp/tiny3d/Tiny3DConfig.h")


add_library(Tiny3D)

add_subdirectory(geometry)
add_subdirectory(io)
add_subdirectory(pipelines)
add_subdirectory(utility)


# note: adding at least one real source file to any target that references
# reference: https://cmake.org/cmake/help/v3.12/command/add_library.html#object-libraries
target_sources(Tiny3D PRIVATE
    Tiny3DConfig.cpp
)

target_sources(Tiny3D PRIVATE
    $<TARGET_OBJECTS:geometry>
    $<TARGET_OBJECTS:io>
    $<TARGET_OBJECTS:pipelines>
    $<TARGET_OBJECTS:utility>
)


# Source group for Visual Studio

add_source_group(geometry)
add_source_group(io)
add_source_group(pipelines)
add_source_group(utility)

tiny3d_show_and_abort_on_warning(Tiny3D)
tiny3d_set_global_properties(Tiny3D)
tiny3d_set_tiny3d_lib_properties(Tiny3D)
tiny3d_link_3rdparty_libraries(Tiny3D)

# If we are building a STATIC_LIBRARY, hide symbols coming from 3rd party static
# libraries that are not hidden during compilation. Don't propagate beyond
# direct consumers of libTiny3D.a
target_link_options(Tiny3D INTERFACE
    $<$<STREQUAL:$<TARGET_PROPERTY:Tiny3D,TYPE>,STATIC_LIBRARY>:$<LINK_ONLY:${TINY3D_HIDDEN_3RDPARTY_LINK_OPTIONS}>>)

add_library(Tiny3D::Tiny3D ALIAS Tiny3D)

include(CMakePackageConfigHelpers)

# find_package Tiny3D
configure_package_config_file(Tiny3DConfig.cmake.in
                              "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Tiny3DConfig.cmake"
                              INSTALL_DESTINATION ${Tiny3D_INSTALL_CMAKE_DIR}
                              PATH_VARS Tiny3D_INSTALL_INCLUDE_DIR
                              NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# find_package Tiny3D Version
write_basic_package_version_file("${PROJECT_BINARY_DIR}/Tiny3DConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY ExactVersion)

# Installation
install(TARGETS Tiny3D EXPORT Tiny3DTargets
        RUNTIME DESTINATION ${Tiny3D_INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${Tiny3D_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${Tiny3D_INSTALL_LIB_DIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION ${Tiny3D_INSTALL_INCLUDE_DIR}
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.cuh"
)

# Install the Tiny3DConfig.cmake and Tiny3DConfigVersion.cmake
install(FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Tiny3DConfig.cmake"
        "${PROJECT_BINARY_DIR}/Tiny3DConfigVersion.cmake"
        DESTINATION "${Tiny3D_INSTALL_CMAKE_DIR}" COMPONENT dev)

if (BUILD_SHARED_LIBS AND UNIX)
    file(CONFIGURE OUTPUT Tiny3D.pc.in
         CONTENT [=[
prefix=${pcfiledir}/../..
libdir=${prefix}/lib
includedir=${prefix}/include/

Name: Tiny3D
Description: @PROJECT_DESCRIPTION@
URL: @PROJECT_HOMEPAGE_URL@
Version: @PROJECT_VERSION@
Cflags: -std=c++@CMAKE_CXX_STANDARD@ -isystem${includedir} -isystem${includedir}/tiny3d/3rdparty -D$<JOIN:$<TARGET_PROPERTY:INTERFACE_COMPILE_DEFINITIONS>, -D>
Libs: -L${libdir} -Wl,-rpath,${libdir} -lTiny3D -ltbb]=]  @ONLY NEWLINE_STYLE LF)
    file(GENERATE OUTPUT Tiny3D.pc INPUT "${CMAKE_CURRENT_BINARY_DIR}/Tiny3D.pc.in"
        TARGET "Tiny3D::Tiny3D")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Tiny3D.pc"
        DESTINATION "${Tiny3D_INSTALL_LIB_DIR}/pkgconfig")
endif ()

# uninstall target
if(NOT TARGET uninstall)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
                   IMMEDIATE @ONLY)

    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P
                      ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()


#
# Tiny3D 3rd party library integration
#
set(Tiny3D_3RDPARTY_DIR "${CMAKE_CURRENT_LIST_DIR}")

# EXTERNAL_MODULES
# CMake modules we depend on in our public interface. These are modules we
# need to find_package() in our CMake config script, because we will use their
# targets.
set(Tiny3D_3RDPARTY_EXTERNAL_MODULES)

# XXX_FROM_CUSTOM vs. XXX_FROM_SYSTEM
# - "FROM_CUSTOM": downloaded or compiled
# - "FROM_SYSTEM": installed with a system package manager, deteced by CMake

# PUBLIC_TARGETS
# CMake targets we link against in our public interface. They are either locally
# defined and installed, or imported from an external module (see above).
set(Tiny3D_3RDPARTY_PUBLIC_TARGETS_FROM_CUSTOM)
set(Tiny3D_3RDPARTY_PUBLIC_TARGETS_FROM_SYSTEM)

# HEADER_TARGETS
# CMake targets we use in our public interface, but as a special case we only
# need to link privately against the library. This simplifies dependencies
# where we merely expose declared data types from other libraries in our
# public headers, so it would be overkill to require all library users to link
# against that dependency.
set(Tiny3D_3RDPARTY_HEADER_TARGETS_FROM_CUSTOM)
set(Tiny3D_3RDPARTY_HEADER_TARGETS_FROM_SYSTEM)

# PRIVATE_TARGETS
# CMake targets for dependencies which are not exposed in the public API. This
# will include anything else we use internally.
set(Tiny3D_3RDPARTY_PRIVATE_TARGETS_FROM_CUSTOM)
set(Tiny3D_3RDPARTY_PRIVATE_TARGETS_FROM_SYSTEM)

find_package(PkgConfig QUIET)

# tiny3d_build_3rdparty_library(name ...)
#
# Builds a third-party library from source
#
# Valid options:
#    PUBLIC
#        the library belongs to the public interface and must be installed
#    HEADER
#        the library headers belong to the public interface, but the library
#        itself is linked privately
#    INCLUDE_ALL
#        install all files in the include directories. Default is *.h, *.hpp
#    VISIBLE
#        Symbols from this library will be visible for use outside Tiny3D.
#        Required, for example, if it may throw exceptions that need to be
#        caught in client code.
#    DIRECTORY <dir>
#        the library source directory <dir> is either a subdirectory of
#        3rdparty/ or an absolute directory.
#    INCLUDE_DIRS <dir> [<dir> ...]
#        include headers are in the subdirectories <dir>. Trailing slashes
#        have the same meaning as with install(DIRECTORY). <dir> must be
#        relative to the library source directory.
#        If your include is "#include <x.hpp>" and the path of the file is
#        "path/to/libx/x.hpp" then you need to pass "path/to/libx/"
#        with the trailing "/". If you have "#include <libx/x.hpp>" then you
#        need to pass "path/to/libx".
#    SOURCES <src> [<src> ...]
#        the library sources. Can be omitted for header-only libraries.
#        All sources must be relative to the library source directory.
#    LIBS <target> [<target> ...]
#        extra link dependencies
#    DEPENDS <target> [<target> ...]
#        targets on which <name> depends on and that must be built before.
#
function(tiny3d_build_3rdparty_library name)
    cmake_parse_arguments(arg "PUBLIC;HEADER;INCLUDE_ALL;VISIBLE" "DIRECTORY" "INCLUDE_DIRS;SOURCES;LIBS;DEPENDS" ${ARGN})
    if(arg_UNPARSED_ARGUMENTS)
        message(STATUS "Unparsed: ${arg_UNPARSED_ARGUMENTS}")
        message(FATAL_ERROR "Invalid syntax: tiny3d_build_3rdparty_library(${name} ${ARGN})")
    endif()
    get_filename_component(arg_DIRECTORY "${arg_DIRECTORY}" ABSOLUTE BASE_DIR "${Tiny3D_3RDPARTY_DIR}")
    if(arg_SOURCES)
        add_library(${name} STATIC)
        set_target_properties(${name} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_${name}")
        tiny3d_set_global_properties(${name})
    else()
        add_library(${name} INTERFACE)
    endif()
    if(arg_INCLUDE_DIRS)
        set(include_dirs)
        foreach(incl IN LISTS arg_INCLUDE_DIRS)
            list(APPEND include_dirs "${arg_DIRECTORY}/${incl}")
        endforeach()
    else()
        set(include_dirs "${arg_DIRECTORY}/")
    endif()
    if(arg_SOURCES)
        foreach(src IN LISTS arg_SOURCES)
            get_filename_component(abs_src "${src}" ABSOLUTE BASE_DIR "${arg_DIRECTORY}")
            target_sources(${name} PRIVATE ${abs_src})
        endforeach()
        foreach(incl IN LISTS include_dirs)
            if (incl MATCHES "(.*)/$")
                set(incl_path ${CMAKE_MATCH_1})
            else()
                get_filename_component(incl_path "${incl}" DIRECTORY)
            endif()
            target_include_directories(${name} SYSTEM PUBLIC $<BUILD_INTERFACE:${incl_path}>)
        endforeach()
        # Do not export symbols from 3rd party libraries outside the Tiny3D DSO.
        if(NOT arg_PUBLIC AND NOT arg_HEADER AND NOT arg_VISIBLE)
            set_target_properties(${name} PROPERTIES
                C_VISIBILITY_PRESET hidden
                CXX_VISIBILITY_PRESET hidden
                CUDA_VISIBILITY_PRESET hidden
                VISIBILITY_INLINES_HIDDEN ON
            )
        endif()
        if(arg_LIBS)
            target_link_libraries(${name} PRIVATE ${arg_LIBS})
        endif()
    else()
        foreach(incl IN LISTS include_dirs)
            if (incl MATCHES "(.*)/$")
                set(incl_path ${CMAKE_MATCH_1})
            else()
                get_filename_component(incl_path "${incl}" DIRECTORY)
            endif()
            target_include_directories(${name} SYSTEM INTERFACE $<BUILD_INTERFACE:${incl_path}>)
        endforeach()
    endif()
    if(NOT BUILD_SHARED_LIBS OR arg_PUBLIC)
        install(TARGETS ${name} EXPORT ${PROJECT_NAME}Targets
            RUNTIME DESTINATION ${Tiny3D_INSTALL_BIN_DIR}
            ARCHIVE DESTINATION ${Tiny3D_INSTALL_LIB_DIR}
            LIBRARY DESTINATION ${Tiny3D_INSTALL_LIB_DIR}
        )
    endif()
    if(arg_PUBLIC OR arg_HEADER)
        foreach(incl IN LISTS include_dirs)
            if(arg_INCLUDE_ALL)
                install(DIRECTORY ${incl}
                    DESTINATION ${Tiny3D_INSTALL_INCLUDE_DIR}/tiny3d/3rdparty
                )
            else()
                install(DIRECTORY ${incl}
                    DESTINATION ${Tiny3D_INSTALL_INCLUDE_DIR}/tiny3d/3rdparty
                    FILES_MATCHING
                        PATTERN "*.h"
                        PATTERN "*.hpp"
                )
            endif()
            target_include_directories(${name} INTERFACE $<INSTALL_INTERFACE:${Tiny3D_INSTALL_INCLUDE_DIR}/tiny3d/3rdparty>)
        endforeach()
    endif()
    if(arg_DEPENDS)
        add_dependencies(${name} ${arg_DEPENDS})
    endif()
    add_library(${PROJECT_NAME}::${name} ALIAS ${name})
endfunction()

# CMake arguments for configuring ExternalProjects. Use the second _hidden
# version by default.
set(ExternalProject_CMAKE_ARGS
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CUDA_COMPILER=${CMAKE_CUDA_COMPILER}
    -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}
    -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
    -DCMAKE_CUDA_COMPILER_LAUNCHER=${CMAKE_CUDA_COMPILER_LAUNCHER}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    -DCMAKE_CUDA_FLAGS=${CMAKE_CUDA_FLAGS}
    -DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}
    -DCMAKE_INSTALL_LIBDIR=${Tiny3D_INSTALL_LIB_DIR}
    # Always build 3rd party code in Release mode. Ignored by multi-config
    # generators (XCode, MSVC). MSVC needs matching config anyway.
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW
    -DCMAKE_MSVC_RUNTIME_LIBRARY:STRING=${CMAKE_MSVC_RUNTIME_LIBRARY}
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    )
# Keep 3rd party symbols hidden from Tiny3D user code. Do not use if 3rd party
# libraries throw exceptions that escape Tiny3D.
set(ExternalProject_CMAKE_ARGS_hidden
    ${ExternalProject_CMAKE_ARGS}
    # Apply LANG_VISIBILITY_PRESET to static libraries and archives as well
    -DCMAKE_POLICY_DEFAULT_CMP0063:STRING=NEW
    -DCMAKE_CXX_VISIBILITY_PRESET=hidden
    -DCMAKE_CUDA_VISIBILITY_PRESET=hidden
    -DCMAKE_C_VISIBILITY_PRESET=hidden
    -DCMAKE_VISIBILITY_INLINES_HIDDEN=ON
    )

# tiny3d_pkg_config_3rdparty_library(name ...)
#
# Creates an interface library for a pkg-config dependency.
#
# The function will set ${name}_FOUND to TRUE or FALSE
# indicating whether or not the library could be found.
#
# Valid options:
#    PUBLIC
#        the library belongs to the public interface and must be installed
#    HEADER
#        the library headers belong to the public interface, but the library
#        itself is linked privately
#    SEARCH_ARGS
#        the arguments passed to pkg_search_module()
#
function(tiny3d_pkg_config_3rdparty_library name)
    cmake_parse_arguments(arg "PUBLIC;HEADER" "" "SEARCH_ARGS" ${ARGN})
    if(arg_UNPARSED_ARGUMENTS)
        message(STATUS "Unparsed: ${arg_UNPARSED_ARGUMENTS}")
        message(FATAL_ERROR "Invalid syntax: tiny3d_pkg_config_3rdparty_library(${name} ${ARGN})")
    endif()
    if(PKGCONFIG_FOUND)
        pkg_search_module(pc_${name} ${arg_SEARCH_ARGS})
    endif()
    if(pc_${name}_FOUND)
        message(STATUS "Using installed third-party library ${name} ${${name_uc}_VERSION}")
        add_library(${name} INTERFACE)
        target_include_directories(${name} SYSTEM INTERFACE ${pc_${name}_INCLUDE_DIRS})
        target_link_libraries(${name} INTERFACE ${pc_${name}_LINK_LIBRARIES})
        foreach(flag IN LISTS pc_${name}_CFLAGS_OTHER)
            if(flag MATCHES "-D(.*)")
                target_compile_definitions(${name} INTERFACE ${CMAKE_MATCH_1})
            endif()
        endforeach()
        if(NOT BUILD_SHARED_LIBS OR arg_PUBLIC)
            install(TARGETS ${name} EXPORT ${PROJECT_NAME}Targets)
        endif()
        set(${name}_FOUND TRUE PARENT_SCOPE)
        add_library(${PROJECT_NAME}::${name} ALIAS ${name})
    else()
        message(STATUS "Unable to find installed third-party library ${name}")
        set(${name}_FOUND FALSE PARENT_SCOPE)
    endif()
endfunction()

# tiny3d_find_package_3rdparty_library(name ...)
#
# Creates an interface library for a find_package dependency.
#
# The function will set ${name}_FOUND to TRUE or FALSE
# indicating whether or not the library could be found.
#
# Valid options:
#    PUBLIC
#        the library belongs to the public interface and must be installed
#    HEADER
#        the library headers belong to the public interface, but the library
#        itself is linked privately
#    REQUIRED
#        finding the package is required
#    QUIET
#        finding the package is quiet
#    PACKAGE <pkg>
#        the name of the queried package <pkg> forwarded to find_package()
#    PACKAGE_VERSION_VAR <pkg_version>
#        the variable <pkg_version> where to find the version of the queried package <pkg> find_package().
#        If not provided, PACKAGE_VERSION_VAR will default to <pkg>_VERSION.
#    TARGETS <target> [<target> ...]
#        the expected targets to be found in <pkg>
#    INCLUDE_DIRS
#        the expected include directory variable names to be found in <pkg>.
#        If <pkg> also defines targets, use them instead and pass them via TARGETS option.
#    LIBRARIES
#        the expected library variable names to be found in <pkg>.
#        If <pkg> also defines targets, use them instead and pass them via TARGETS option.
#    PATHS
#        Paths with hardcoded guesses. Same as in find_package.
#    DEPENDS
#        Adds targets that should be build before "name" as dependency.
#
function(tiny3d_find_package_3rdparty_library name)
    cmake_parse_arguments(arg "PUBLIC;HEADER;REQUIRED;QUIET"
        "PACKAGE;VERSION;PACKAGE_VERSION_VAR"
        "TARGETS;INCLUDE_DIRS;LIBRARIES;PATHS;DEPENDS" ${ARGN})
    if(arg_UNPARSED_ARGUMENTS)
        message(STATUS "Unparsed: ${arg_UNPARSED_ARGUMENTS}")
        message(FATAL_ERROR "Invalid syntax: tiny3d_find_package_3rdparty_library(${name} ${ARGN})")
    endif()
    if(NOT arg_PACKAGE)
        message(FATAL_ERROR "tiny3d_find_package_3rdparty_library: Expected value for argument PACKAGE")
    endif()
    if(NOT arg_PACKAGE_VERSION_VAR)
        set(arg_PACKAGE_VERSION_VAR "${arg_PACKAGE}_VERSION")
    endif()
    set(find_package_args "")
    if(arg_VERSION)
        list(APPEND find_package_args "${arg_VERSION}")
    endif()
    if(arg_REQUIRED)
        list(APPEND find_package_args "REQUIRED")
    endif()
    if(arg_QUIET)
        list(APPEND find_package_args "QUIET")
    endif()
    if (arg_PATHS)
        list(APPEND find_package_args PATHS ${arg_PATHS} NO_DEFAULT_PATH)
    endif()
    find_package(${arg_PACKAGE} ${find_package_args})
    if(${arg_PACKAGE}_FOUND)
        message(STATUS "Using installed third-party library ${name} ${${arg_PACKAGE}_VERSION}")
        add_library(${name} INTERFACE)
        if(arg_TARGETS)
            foreach(target IN LISTS arg_TARGETS)
                if (TARGET ${target})
                    target_link_libraries(${name} INTERFACE ${target})
                else()
                    message(WARNING "Skipping undefined target ${target}")
                endif()
            endforeach()
        endif()
        if(arg_INCLUDE_DIRS)
            foreach(incl IN LISTS arg_INCLUDE_DIRS)
                target_include_directories(${name} INTERFACE ${${incl}})
            endforeach()
        endif()
        if(arg_LIBRARIES)
            foreach(lib IN LISTS arg_LIBRARIES)
                target_link_libraries(${name} INTERFACE ${${lib}})
            endforeach()
        endif()
        if(NOT BUILD_SHARED_LIBS OR arg_PUBLIC)
            install(TARGETS ${name} EXPORT ${PROJECT_NAME}Targets)
            # Ensure that imported targets will be found again.
            if(arg_TARGETS)
                list(APPEND Tiny3D_3RDPARTY_EXTERNAL_MODULES ${arg_PACKAGE})
                set(Tiny3D_3RDPARTY_EXTERNAL_MODULES ${Tiny3D_3RDPARTY_EXTERNAL_MODULES} PARENT_SCOPE)
            endif()
        endif()
        if(arg_DEPENDS)
            add_dependencies(${name} ${arg_DEPENDS})
        endif()
        set(${name}_FOUND TRUE PARENT_SCOPE)
        set(${name}_VERSION ${${arg_PACKAGE_VERSION_VAR}} PARENT_SCOPE)
        add_library(${PROJECT_NAME}::${name} ALIAS ${name})
    else()
        message(STATUS "Unable to find installed third-party library ${name}")
        set(${name}_FOUND FALSE PARENT_SCOPE)
    endif()
endfunction()

# List of linker options for libTiny3D client binaries (eg: pybind) to hide Tiny3D 3rd
# party dependencies. Only needed with GCC, not AppleClang.
set(TINY3D_HIDDEN_3RDPARTY_LINK_OPTIONS)

if (CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
    find_library(LexLIB libl.a)    # test archive in macOS
    if (LexLIB)
        include(CheckCXXSourceCompiles)
        set(CMAKE_REQUIRED_LINK_OPTIONS -load_hidden ${LexLIB})
        check_cxx_source_compiles("int main() {return 0;}" FLAG_load_hidden)
        unset(CMAKE_REQUIRED_LINK_OPTIONS)
    endif()
endif()
if (NOT FLAG_load_hidden)
    set(FLAG_load_hidden 0)
endif()

# tiny3d_import_3rdparty_library(name ...)
#
# Imports a third-party library that has been built independently in a sub project.
#
# Valid options:
#    PUBLIC
#        the library belongs to the public interface and must be installed
#    HEADER
#        the library headers belong to the public interface and will be
#        installed, but the library is linked privately.
#    INCLUDE_ALL
#        install all files in the include directories. Default is *.h, *.hpp
#    HIDDEN
#        Symbols from this library will not be exported to client code during
#        linking with Tiny3D. This is the opposite of the VISIBLE option in
#        tiny3d_build_3rdparty_library.  Prefer hiding symbols during building 3rd
#        party libraries, since this option is not supported by the MSVC linker.
#    GROUPED
#        add "-Wl,--start-group" libx.a liby.a libz.a "-Wl,--end-group" around
#        the libraries.
#    INCLUDE_DIRS
#        the temporary location where the library headers have been installed.
#        Trailing slashes have the same meaning as with install(DIRECTORY).
#        If your include is "#include <x.hpp>" and the path of the file is
#        "/path/to/libx/x.hpp" then you need to pass "/path/to/libx/"
#        with the trailing "/". If you have "#include <libx/x.hpp>" then you
#        need to pass "/path/to/libx".
#    LIBRARIES
#        the built library name(s). It is assumed that the library is static.
#        If the library is PUBLIC, it will be renamed to Tiny3D_${name} at
#        install time to prevent name collisions in the install space.
#    LIB_DIR
#        the temporary location of the library. Defaults to
#        CMAKE_ARCHIVE_OUTPUT_DIRECTORY.
#    DEPENDS <target> [<target> ...]
#        targets on which <name> depends on and that must be built before.
#
function(tiny3d_import_3rdparty_library name)
    cmake_parse_arguments(arg "PUBLIC;HEADER;INCLUDE_ALL;HIDDEN;GROUPED" "LIB_DIR" "INCLUDE_DIRS;LIBRARIES;DEPENDS" ${ARGN})
    if(arg_UNPARSED_ARGUMENTS)
        message(STATUS "Unparsed: ${arg_UNPARSED_ARGUMENTS}")
        message(FATAL_ERROR "Invalid syntax: tiny3d_import_3rdparty_library(${name} ${ARGN})")
    endif()
    if(NOT arg_LIB_DIR)
        set(arg_LIB_DIR "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    endif()
    add_library(${name} INTERFACE)
    if(arg_INCLUDE_DIRS)
        foreach(incl IN LISTS arg_INCLUDE_DIRS)
            if (incl MATCHES "(.*)/$")
                set(incl_path ${CMAKE_MATCH_1})
            else()
                get_filename_component(incl_path "${incl}" DIRECTORY)
            endif()
            target_include_directories(${name} SYSTEM INTERFACE $<BUILD_INTERFACE:${incl_path}>)
            if(arg_PUBLIC OR arg_HEADER)
                if(arg_INCLUDE_ALL)
                    install(DIRECTORY ${incl}
                        DESTINATION ${Tiny3D_INSTALL_INCLUDE_DIR}/tiny3d/3rdparty
                    )
                else()
                    install(DIRECTORY ${incl}
                        DESTINATION ${Tiny3D_INSTALL_INCLUDE_DIR}/tiny3d/3rdparty
                        FILES_MATCHING
                            PATTERN "*.h"
                            PATTERN "*.hpp"
                    )
                endif()
                target_include_directories(${name} INTERFACE $<INSTALL_INTERFACE:${Tiny3D_INSTALL_INCLUDE_DIR}/tiny3d/3rdparty>)
            endif()
        endforeach()
    endif()
    if(arg_LIBRARIES)
        list(LENGTH arg_LIBRARIES libcount)
        if(arg_HIDDEN AND NOT arg_PUBLIC AND NOT arg_HEADER)
            set(HIDDEN 1)
        else()
            set(HIDDEN 0)
        endif()
        if(arg_GROUPED AND UNIX AND NOT APPLE)
            target_link_libraries(${name} INTERFACE "-Wl,--start-group")
        endif()
        foreach(arg_LIBRARY IN LISTS arg_LIBRARIES)
            set(library_filename ${CMAKE_STATIC_LIBRARY_PREFIX}${arg_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX})
            if(libcount EQUAL 1)
                set(installed_library_filename ${CMAKE_STATIC_LIBRARY_PREFIX}${PROJECT_NAME}_${name}${CMAKE_STATIC_LIBRARY_SUFFIX})
            else()
                set(installed_library_filename ${CMAKE_STATIC_LIBRARY_PREFIX}${PROJECT_NAME}_${name}_${arg_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX})
            endif()
            # Apple compiler ld
            target_link_libraries(${name} INTERFACE
                "$<BUILD_INTERFACE:$<$<AND:${HIDDEN},${FLAG_load_hidden}>:-load_hidden >${arg_LIB_DIR}/${library_filename}>")
            if(NOT BUILD_SHARED_LIBS OR arg_PUBLIC)
                install(FILES ${arg_LIB_DIR}/${library_filename}
                    DESTINATION ${Tiny3D_INSTALL_LIB_DIR}
                    RENAME ${installed_library_filename}
                )
                target_link_libraries(${name} INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${Tiny3D_INSTALL_LIB_DIR}/${installed_library_filename}>)
            endif()
            if (HIDDEN)
                # GNU compiler ld
                target_link_options(${name} INTERFACE
                    $<$<CXX_COMPILER_ID:GNU>:LINKER:--exclude-libs,${library_filename}>)
                list(APPEND TINY3D_HIDDEN_3RDPARTY_LINK_OPTIONS $<$<CXX_COMPILER_ID:GNU>:LINKER:--exclude-libs,${library_filename}>)
                set(TINY3D_HIDDEN_3RDPARTY_LINK_OPTIONS
                    ${TINY3D_HIDDEN_3RDPARTY_LINK_OPTIONS} PARENT_SCOPE)
            endif()
        endforeach()
        if(arg_GROUPED AND UNIX AND NOT APPLE)
            target_link_libraries(${name} INTERFACE "-Wl,--end-group")
        endif()
    endif()
    if(NOT BUILD_SHARED_LIBS OR arg_PUBLIC)
        install(TARGETS ${name} EXPORT ${PROJECT_NAME}Targets)
    endif()
    if(arg_DEPENDS)
        add_dependencies(${name} ${arg_DEPENDS})
    endif()
    add_library(${PROJECT_NAME}::${name} ALIAS ${name})
endfunction()

include(ProcessorCount)
ProcessorCount(NPROC)

# Threads
tiny3d_find_package_3rdparty_library(3rdparty_threads
    REQUIRED
    PACKAGE Threads
    TARGETS Threads::Threads
)


# OpenMP
if(WITH_OPENMP)
    tiny3d_find_package_3rdparty_library(3rdparty_openmp
        PACKAGE OpenMP
        PACKAGE_VERSION_VAR OpenMP_CXX_VERSION
        TARGETS OpenMP::OpenMP_CXX
    )
    if(3rdparty_openmp_FOUND)
        message(STATUS "Building with OpenMP")
        list(APPEND Tiny3D_3RDPARTY_PRIVATE_TARGETS_FROM_SYSTEM Tiny3D::3rdparty_openmp)
    else()
        set(WITH_OPENMP OFF)
    endif()
endif()

# X11
if(UNIX AND NOT APPLE)
    tiny3d_find_package_3rdparty_library(3rdparty_x11
        QUIET
        PACKAGE X11
        TARGETS X11::X11
    )
endif()


# Eigen3
if(USE_SYSTEM_EIGEN3)
    tiny3d_find_package_3rdparty_library(3rdparty_eigen3
        PUBLIC
        PACKAGE Eigen3
        TARGETS Eigen3::Eigen
    )
    if(NOT 3rdparty_eigen3_FOUND)
        set(USE_SYSTEM_EIGEN3 OFF)
    endif()
endif()
if(NOT USE_SYSTEM_EIGEN3)
    include(${Tiny3D_3RDPARTY_DIR}/eigen/eigen.cmake)
    tiny3d_import_3rdparty_library(3rdparty_eigen3
        PUBLIC
        INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS}
        INCLUDE_ALL
        DEPENDS      ext_eigen
    )
    list(APPEND Tiny3D_3RDPARTY_PUBLIC_TARGETS_FROM_CUSTOM Tiny3D::3rdparty_eigen3)
else()
    list(APPEND Tiny3D_3RDPARTY_PUBLIC_TARGETS_FROM_SYSTEM Tiny3D::3rdparty_eigen3)
endif()

# Nanoflann
if(USE_SYSTEM_NANOFLANN)
    tiny3d_find_package_3rdparty_library(3rdparty_nanoflann
        PACKAGE nanoflann
        VERSION 1.5.0
        TARGETS nanoflann::nanoflann
    )
    if(NOT 3rdparty_nanoflann_FOUND)
        set(USE_SYSTEM_NANOFLANN OFF)
    endif()
endif()
if(NOT USE_SYSTEM_NANOFLANN)
    include(${Tiny3D_3RDPARTY_DIR}/nanoflann/nanoflann.cmake)
    tiny3d_import_3rdparty_library(3rdparty_nanoflann
        INCLUDE_DIRS ${NANOFLANN_INCLUDE_DIRS}
        DEPENDS      ext_nanoflann
    )
    list(APPEND Tiny3D_3RDPARTY_PRIVATE_TARGETS_FROM_CUSTOM Tiny3D::3rdparty_nanoflann)
else()
    list(APPEND Tiny3D_3RDPARTY_PRIVATE_TARGETS_FROM_SYSTEM Tiny3D::3rdparty_nanoflann)
endif()


# jsoncpp
if(USE_SYSTEM_JSONCPP)
    tiny3d_find_package_3rdparty_library(3rdparty_jsoncpp
        PACKAGE jsoncpp
        TARGETS jsoncpp_lib
    )
    if(NOT 3rdparty_jsoncpp_FOUND)
        set(USE_SYSTEM_JSONCPP OFF)
    endif()
endif()
if(NOT USE_SYSTEM_JSONCPP)
    include(${Tiny3D_3RDPARTY_DIR}/jsoncpp/jsoncpp.cmake)
    tiny3d_import_3rdparty_library(3rdparty_jsoncpp
        INCLUDE_DIRS ${JSONCPP_INCLUDE_DIRS}
        LIB_DIR      ${JSONCPP_LIB_DIR}
        LIBRARIES    ${JSONCPP_LIBRARIES}
        DEPENDS      ext_jsoncpp
    )
    list(APPEND Tiny3D_3RDPARTY_PRIVATE_TARGETS_FROM_CUSTOM Tiny3D::3rdparty_jsoncpp)
else()
    list(APPEND Tiny3D_3RDPARTY_PRIVATE_TARGETS_FROM_SYSTEM Tiny3D::3rdparty_jsoncpp)
endif()



# rply
tiny3d_build_3rdparty_library(3rdparty_rply DIRECTORY rply
    SOURCES
        rply/rply.c
    INCLUDE_DIRS
        rply/
)
list(APPEND Tiny3D_3RDPARTY_PRIVATE_TARGETS_FROM_CUSTOM Tiny3D::3rdparty_rply)

# fmt
if(USE_SYSTEM_FMT)
    # MSVC >= 17.x required for building fmt 8+
    # SYCL / DPC++ needs fmt ver <8 or >= 9.2: https://github.com/fmtlib/fmt/issues/3005
    tiny3d_find_package_3rdparty_library(3rdparty_fmt
        PUBLIC
        PACKAGE fmt
        TARGETS fmt::fmt
    )
    if(NOT 3rdparty_fmt_FOUND)
        set(USE_SYSTEM_FMT OFF)
    endif()
endif()
if(NOT USE_SYSTEM_FMT)
    include(${Tiny3D_3RDPARTY_DIR}/fmt/fmt.cmake)
    tiny3d_import_3rdparty_library(3rdparty_fmt
        HEADER
        INCLUDE_DIRS ${FMT_INCLUDE_DIRS}
        LIB_DIR      ${FMT_LIB_DIR}
        LIBRARIES    ${FMT_LIBRARIES}
        DEPENDS      ext_fmt
    )
    # FMT 6.0, newer versions may require different flags
    target_compile_definitions(3rdparty_fmt INTERFACE FMT_HEADER_ONLY=0)
    target_compile_definitions(3rdparty_fmt INTERFACE FMT_USE_WINDOWS_H=0)
    target_compile_definitions(3rdparty_fmt INTERFACE FMT_STRING_ALIAS=1)
    list(APPEND Tiny3D_3RDPARTY_HEADER_TARGETS_FROM_CUSTOM Tiny3D::3rdparty_fmt)
else()
    list(APPEND Tiny3D_3RDPARTY_PUBLIC_TARGETS_FROM_SYSTEM Tiny3D::3rdparty_fmt)
endif()

# Pybind11
if (BUILD_PYTHON_MODULE)
    if(USE_SYSTEM_PYBIND11)
        find_package(pybind11)
    endif()
    if (NOT USE_SYSTEM_PYBIND11 OR NOT TARGET pybind11::module)
        set(USE_SYSTEM_PYBIND11 OFF)
        include(${Tiny3D_3RDPARTY_DIR}/pybind11/pybind11.cmake)
        # pybind11 will automatically become available.
    endif()
endif()

# Compactify list of external modules.
# This must be called after all dependencies are processed.
list(REMOVE_DUPLICATES Tiny3D_3RDPARTY_EXTERNAL_MODULES)

# Target linking order matters. When linking a target, the link libraries and
# include directories are set. If the order is wrong, it can cause missing
# symbols or wrong header versions. Generally, we want to link custom libs
# before system libs; the order among differnt libs can also matter.
#
# Current rules:
# 1. 3rdparty_curl should be linked before 3rdparty_png.
# 2. Link "FROM_CUSTOM" before "FROM_SYSTEM" targets.
# 3. ...
set(Tiny3D_3RDPARTY_PUBLIC_TARGETS
    ${Tiny3D_3RDPARTY_PUBLIC_TARGETS_FROM_CUSTOM}
    ${Tiny3D_3RDPARTY_PUBLIC_TARGETS_FROM_SYSTEM})
set(Tiny3D_3RDPARTY_HEADER_TARGETS
    ${Tiny3D_3RDPARTY_HEADER_TARGETS_FROM_CUSTOM}
    ${Tiny3D_3RDPARTY_HEADER_TARGETS_FROM_SYSTEM})
set(Tiny3D_3RDPARTY_PRIVATE_TARGETS
    ${Tiny3D_3RDPARTY_PRIVATE_TARGETS_FROM_CUSTOM}
    ${Tiny3D_3RDPARTY_PRIVATE_TARGETS_FROM_SYSTEM})
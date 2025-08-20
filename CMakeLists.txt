cmake_minimum_required(VERSION 3.16)
# If you're using Ubuntu 20.04, we suggest you install the latest CMake from the
# official repository https://apt.kitware.com/.


# CMAKE_HOST_SYSTEM_PROCESSOR is only available after calling project(),
# which depends on ${TINY3D_VERSION}, which depends on ${DEVELOPER_BUILD}.
if(UNIX AND NOT APPLE)
    execute_process(COMMAND uname -m
        OUTPUT_VARIABLE PROCESSOR_ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(PROCESSOR_ARCH STREQUAL "aarch64")
        set(LINUX_AARCH64 TRUE)
    endif()
endif()
if(APPLE)
    execute_process(COMMAND uname -m
        OUTPUT_VARIABLE PROCESSOR_ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(PROCESSOR_ARCH STREQUAL "arm64")
        set(APPLE_AARCH64 TRUE)
        set (CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING
            "Minimum OS X deployment version" FORCE)
    else()
        set (CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING
            "Minimum OS X deployment version" FORCE)
    endif()
endif()

include(CMakeDependentOption)

# Tiny3D build options
option(BUILD_SHARED_LIBS          "Build shared libraries"                   ON)
option(BUILD_UNIT_TESTS           "Build Tiny3D unit tests"                  OFF)
option(BUILD_BENCHMARKS           "Build the micro benchmarks"               OFF)
option(BUILD_PYTHON_MODULE        "Build the python module"                  ON )
option(WITH_OPENMP                "Use OpenMP multi-threading"               ON )
if(BUILD_SHARED_LIBS)
    option(STATIC_WINDOWS_RUNTIME "Use static (MT/MTd) Windows runtime"      OFF)
else()
    option(STATIC_WINDOWS_RUNTIME "Use static (MT/MTd) Windows runtime"      ON )
endif()
option(GLIBCXX_USE_CXX11_ABI      "Set -D_GLIBCXX_USE_CXX11_ABI=1"           ON )


# 3rd-party build options
if(LINUX_AARCH64 OR APPLE_AARCH64)
    option(USE_BLAS               "Use BLAS/LAPACK instead of MKL"           ON )
else()
    option(USE_BLAS               "Use BLAS/LAPACK instead of MKL"           OFF)
endif()
if(USE_BLAS)
    option(USE_SYSTEM_BLAS        "Use system pre-installed openblas"        OFF)
else()
    option(USE_SYSTEM_BLAS        "Use system pre-installed openblas"        ON )
endif()

option(USE_SYSTEM_EIGEN3          "Use system pre-installed eigen3"          OFF)
option(USE_SYSTEM_FMT             "Use system pre-installed fmt"             OFF)
option(USE_SYSTEM_JSONCPP         "Use system pre-installed jsoncpp"         OFF)
option(USE_SYSTEM_NANOFLANN       "Use system pre-installed nanoflann"       OFF)
option(USE_SYSTEM_PYBIND11        "Use system pre-installed pybind11"        OFF)

option(PREFER_OSX_HOMEBREW        "Prefer Homebrew libs over frameworks"     ON )


# Release build options
option(DEVELOPER_BUILD      "Add +commit_hash to the project version number" ON )

# Default build type on single-config generators.
# For multi-config generators (e.g. Visual Studio), CMAKE_CONFIGURATION_TYPES
# will be set, and we don't specify a default CMAKE_BUILD_TYPE.
# https://blog.kitware.com/cmake-and-the-default-build-type/
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to Release as none was specified.")
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
        # Set the possible values of build type for cmake-gui.
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
    endif()
    message(STATUS "CMAKE_BUILD_TYPE is set to ${CMAKE_BUILD_TYPE}.")
endif()

find_program(CCACHE "ccache")
if (CCACHE)
    message(STATUS "ccache found at ${CCACHE}")
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif()

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)  # URL contents timestamped by download time
endif()
# In ExternalProject_Add, if TINY3D_THIRD_PARTY_DOWNLOAD_DIR is specified, CMake will
# use this directory to cache downloaded 3rd party dependencies and automatically skip
# downloading from the Internet if the files are available. This is only supported by
# a limited number of 3rd party libraries.
set(TINY3D_THIRD_PARTY_DOWNLOAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty_downloads"
    CACHE PATH "Third-party download directory for caching.")
message(STATUS "Downloading third-party dependencies to ${TINY3D_THIRD_PARTY_DOWNLOAD_DIR}")

if (PREFER_OSX_HOMEBREW)
    set(CMAKE_FIND_FRAMEWORK LAST)
    set(CMAKE_FIND_APPBUNDLE LAST)
endif()


if ((LINUX_AARCH64 OR APPLE_AARCH64) AND NOT USE_BLAS)
    message(FATAL_ERROR "ARM CPU detected, you must set USE_BLAS=ON.")
endif()


# Parse Tiny3D version number
file(STRINGS "cpp/tiny3d/version.txt" TINY3D_VERSION_READ)
foreach(ver ${TINY3D_VERSION_READ})
    if (ver MATCHES "TINY3D_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(TINY3D_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(TINY3D_VERSION_DEVHASH "")
if(DEVELOPER_BUILD)
    execute_process(COMMAND git -C "${CMAKE_SOURCE_DIR}" log --pretty=format:%h -n 1
        OUTPUT_VARIABLE GIT_REV)
    if (GIT_REV)
        set(TINY3D_VERSION_DEVHASH "+${GIT_REV}")
    endif()
endif()
string(CONCAT TINY3D_VERSION
    "${TINY3D_VERSION_MAJOR}"
    ".${TINY3D_VERSION_MINOR}"
    ".${TINY3D_VERSION_PATCH}"
)
set(TINY3D_VERSION_FULL "${TINY3D_VERSION}${TINY3D_VERSION_DEVHASH}" CACHE
    STRING "Tiny3D full version.")
set(TINY3D_ABI_VERSION "${TINY3D_VERSION_MAJOR}.${TINY3D_VERSION_MINOR}" CACHE
    STRING "Tiny3D ABI version / SOVERSION (for releases only).")
# Set additional info
set(PROJECT_EMAIL       "tiny3d@intel.com")
set(PROJECT_DOCS        "https://www.tiny3d.org/docs")
set(PROJECT_CODE        "https://github.com/isl-org/Tiny3D")
set(PROJECT_ISSUES      "https://github.com/isl-org/Tiny3D/issues")

project(Tiny3D
    VERSION ${TINY3D_VERSION}
    # Set PROJECT_DESCRIPTION
    DESCRIPTION "Tiny3D: A Modern Library for 3D Data Processing."
    # Set PROJECT_HOMEPAGE_URL
    HOMEPAGE_URL "https://www.tiny3d.org"
    LANGUAGES C CXX)
message(STATUS "Tiny3D ${TINY3D_VERSION_FULL}")


# Global flag to set CXX standard.
# This does not affect 3rd party libraries.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)   # Improved compatibility

# Suppress warnings for deprecated C++17 functions (stdgpu->thrust with CUDA 11 for MSVC).
add_compile_definitions($<$<COMPILE_LANGUAGE:CUDA>:_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING>)
# CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/cmake")

# Setup Python executable
if(NOT DEFINED Python3_FIND_REGISTRY)
    # Only consider PATH variable on Windows by default
    set(Python3_FIND_REGISTRY NEVER)
endif()
# Requires Python 3.6+
find_package(Python3 3.6
             COMPONENTS Interpreter Development)
if (Python3_FOUND)
    # Setup PYTHON_EXECUTABLE for 3rdparty modules
    # which still use the deprecated find_package(PythonInterp)
    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE} CACHE STRING
        "Deprecated path to the Python executable (for 3rdparty only)" FORCE)
else()
    if (BUILD_PYTHON_MODULE)
        message(FATAL_ERROR "BUILD_PYTHON_MODULE=ON requires Python >= 3.6. Please ensure it is in PATH.")
    endif()
endif()

# npm version has to be MAJOR.MINOR.PATCH
string(CONCAT PROJECT_VERSION_THREE_NUMBER "${TINY3D_VERSION_MAJOR}"
                                           ".${TINY3D_VERSION_MINOR}"
                                           ".${TINY3D_VERSION_PATCH}")

if(NOT DEFINED PYPI_PACKAGE_NAME)
    set(PYPI_PACKAGE_NAME "tiny3d")
endif()

# Set installation paths
if(UNIX OR CYGWIN)
    include(GNUInstallDirs)
    set(Tiny3D_INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
    set(Tiny3D_INSTALL_BIN_DIR "${CMAKE_INSTALL_BINDIR}")
    set(Tiny3D_INSTALL_LIB_DIR "${CMAKE_INSTALL_LIBDIR}")
    # Put resources in */share/
    set(Tiny3D_INSTALL_RESOURCE_DIR "${CMAKE_INSTALL_DATADIR}")
    set(Tiny3D_INSTALL_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
else()
    set(Tiny3D_INSTALL_INCLUDE_DIR include)
    set(Tiny3D_INSTALL_BIN_DIR bin)
    set(Tiny3D_INSTALL_LIB_DIR lib)
    # Put resources in */bin, with executables / DLLs
    set(Tiny3D_INSTALL_RESOURCE_DIR bin)
    set(Tiny3D_INSTALL_CMAKE_DIR CMake)
endif()

# Put build results in some predictable places
# The $<CONFIG> generator expression makes sure that XCode or Visual Studio do not
# append additional path components, as we need to know *exactly* where the build results
# end up.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/$<CONFIG>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Global Security options (including 3rd party code)
# Add -fPIC for library and -fPIE for executable to compiler and linker. Does not add -pie !
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Explicitly specify the preference of using -pthread over -lpthread.
# This must be defined here since CUDA calls find_package(Threads) internally.
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

# Overwrites property for Thread::Threads in find_package(Threads)
# For CUDA, "-pthread" is replaced with "-Xcompiler -pthread" (CMake's default)
# For ISPC, "-pthread" is disabled
macro(tiny3d_patch_findthreads_module_)
    if(TARGET Threads::Threads AND THREADS_HAVE_PTHREAD_ARG)
        set_property(TARGET Threads::Threads
                     PROPERTY INTERFACE_COMPILE_OPTIONS
                     "$<$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>:SHELL:-Xcompiler -pthread>"
                     "$<$<AND:$<NOT:$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>>,$<NOT:$<COMPILE_LANGUAGE:ISPC>>>:-pthread>")
    endif()
endmacro()
cmake_language(EVAL CODE "cmake_language(DEFER CALL tiny3d_patch_findthreads_module_)")


# OS specific settings
if(WIN32)
    # Windows defaults to hidden symbol visibility, override that
    # TODO: It would be better to explicitly export symbols.
    #       Then, we could use -fvisibility=hidden for Linux as well
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    if(MSVC)
        # MSVC standards-compliant mode: Error in PoissonRecon 3rd party header
        # add_compile_options($<$<COMPILE_LANGUAGE:CXX>:"/permissive-">)   
        # Make sure we don't hit the 65535 object member limit with MSVC
        #
        # /bigobj allows object files with more than 65535 members
        # /Ob2 enables function inlining, because MSVC is particularly
        # verbose with inline members
        #
        # See: https://github.com/tensorflow/tensorflow/pull/10962
        add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/bigobj;/Ob2>")
    endif()
    if (STATIC_WINDOWS_RUNTIME)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
endif()

# Folder view for project files
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(add_source_group module_name)
    file(GLOB MODULE_HEADER_FILES "${module_name}/*.h")
    source_group("Header Files\\${module_name}" FILES ${MODULE_HEADER_FILES})
    file(GLOB MODULE_SOURCE_FILES "${module_name}/*.cpp")
    source_group("Source Files\\${module_name}" FILES ${MODULE_SOURCE_FILES})
endmacro()

if (LINUX_AARCH64)
# Fix for ImportError: ... /pybind.cpython-310-aarch64-linux-gnu.so: cannot allocate memory in static TLS block
# https://bugs.launchpad.net/ubuntu/+source/mysql-8.0/+bug/1889851
    add_compile_options("-ftls-model=global-dynamic")
endif()

# Include convenience functions
include(Tiny3DLink3rdpartyLibraries)
include(Tiny3DSetGlobalProperties)
include(Tiny3DShowAndAbortOnWarning)
include(Tiny3DSYCLTargetSources)

# Enumerate all third-party libraries which we need later
# This creates the necessary targets and sets the
# Tiny3D_3RDPARTY_*_TARGETS variables we use in tiny3d_link_3rdparty_libraries
include(3rdparty/find_dependencies.cmake)

# Tiny3D library

# --- Configure Python packaging files for versioning and portability ---
if(BUILD_PYTHON_MODULE)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/python/setup.py.in"
        "${CMAKE_CURRENT_SOURCE_DIR}/python/setup.py"
        @ONLY)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/python/tiny3d/__init__.py.in"
        "${CMAKE_CURRENT_SOURCE_DIR}/python/tiny3d/__init__.py"
        @ONLY)
endif()

add_subdirectory(cpp)

# Install CMake configuration files
install(EXPORT ${PROJECT_NAME}Targets NAMESPACE ${PROJECT_NAME}:: DESTINATION ${Tiny3D_INSTALL_CMAKE_DIR})
export(EXPORT ${PROJECT_NAME}Targets NAMESPACE ${PROJECT_NAME}::)

if (Python3_EXECUTABLE)
    # `make check-style` checks style for c++/cuda/python/ipynb files
    add_custom_target(check-style
        COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/util/check_style.py
        COMMENT "Python executable used for style check: ${Python3_EXECUTABLE}."
    )

    # `make apply-style` applies style for c++/cuda/python/ipynb files
    add_custom_target(apply-style
        COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/util/check_style.py --apply
        COMMENT "Python executable used for style check: ${Python3_EXECUTABLE}."
    )
endif()

include(Tiny3DPackaging)

# `make check-cpp-style` checks style for c++/cuda files.
# This works outside of python virtualenv.
add_custom_target(check-cpp-style
    COMMAND ${CMAKE_COMMAND}
    -DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    -DAPPLY=OFF
    -P ${CMAKE_CURRENT_SOURCE_DIR}/util/check_cpp_style.cmake
)

# `make apply-cpp-style` applies style for c++/cuda files.
# This works outside of python virtualenv.
add_custom_target(apply-cpp-style
    COMMAND ${CMAKE_COMMAND}
    -DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    -DAPPLY=ON
    -P ${CMAKE_CURRENT_SOURCE_DIR}/util/check_cpp_style.cmake
)

include(Tiny3DPrintConfigurationSummary)
tiny3d_print_configuration_summary()
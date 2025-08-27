# Make hardening flags
include(Tiny3DMakeHardeningFlags)
tiny3d_make_hardening_flags(HARDENING_CFLAGS HARDENING_LDFLAGS)
tiny3d_make_hardening_definitions(HARDENING_DEFINITIONS)
message(STATUS "Using security hardening compiler flags: ${HARDENING_CFLAGS}")
message(STATUS "Using security hardening linker flags: ${HARDENING_LDFLAGS}")
message(STATUS "Using security hardening compiler definitions: ${HARDENING_DEFINITIONS}")

# tiny3d_enable_strip(target)
#
# Enable binary strip. Only effective on Linux or macOS.
function(tiny3d_enable_strip target)
    # Strip unnecessary sections of the binary on Linux/macOS for Release builds
    # (from pybind11)
    # macOS: -x: strip local symbols
    # Linux: defaults
    if(NOT DEVELOPER_BUILD AND UNIX AND CMAKE_STRIP)
        get_target_property(target_type ${target} TYPE)
        if(target_type MATCHES MODULE_LIBRARY|SHARED_LIBRARY|EXECUTABLE)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND $<IF:$<CONFIG:Release>,${CMAKE_STRIP},true>
                        $<$<PLATFORM_ID:Darwin>:-x> $<TARGET_FILE:${target}>
                        COMMAND_EXPAND_LISTS)
        endif()
    endif()
endfunction()

# RPATH handling (see below). We don't install targets such as pybind, so BUILD_RPATH must be relative as well.
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

# tiny3d_set_global_properties(target)
#
# Sets important project-related properties to <target>.
function(tiny3d_set_global_properties target)
    # Tell CMake we want a compiler that supports C++17 features
    target_compile_features(${target} PUBLIC cxx_std_17)

    # Detect compiler id and version for utility::CompilerInfo
    # - TINY3D_CXX_STANDARD
    # - TINY3D_CXX_COMPILER_ID
    # - TINY3D_CXX_COMPILER_VERSION
    # - TINY3D_CUDA_COMPILER_ID       # Empty if not BUILD_CUDA_MODULE
    # - TINY3D_CUDA_COMPILER_VERSION  # Empty if not BUILD_CUDA_MODULE
    if (NOT CMAKE_CXX_STANDARD)
        message(FATAL_ERROR "CMAKE_CXX_STANDARD must be defined globally.")
    endif()
    target_compile_definitions(${target} PRIVATE TINY3D_CXX_STANDARD="${CMAKE_CXX_STANDARD}")
    target_compile_definitions(${target} PRIVATE TINY3D_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}")
    target_compile_definitions(${target} PRIVATE TINY3D_CXX_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}")
    target_compile_definitions(${target} PRIVATE TINY3D_CUDA_COMPILER_ID="${CMAKE_CUDA_COMPILER_ID}")
    target_compile_definitions(${target} PRIVATE TINY3D_CUDA_COMPILER_VERSION="${CMAKE_CUDA_COMPILER_VERSION}")

    # std::filesystem (C++17) or std::experimental::filesystem (C++14)
    #
    # Ref: https://en.cppreference.com/w/cpp/filesystem:
    #      Using this library may require additional compiler/linker options.
    #      GNU implementation prior to 9.1 requires linking with -lstdc++fs and
    #      LLVM implementation prior to LLVM 9.0 requires linking with -lc++fs.
    # Ref: https://gitlab.kitware.com/cmake/cmake/-/issues/17834
    #      It's non-trivial to determine the link flags for CMake.
    #
    # The linkage can be "-lstdc++fs" or "-lc++fs" or ""(empty). In our
    # experiments, the behaviour doesn't quite match the specifications.
    #
    # - On Ubuntu 20.04:
    #   - "-lstdc++fs" works with with GCC 7/10 and Clang 7/12
    #   - "" does not work with GCC 7/10 and Clang 7/12
    #
    # - On latest macOS/Windows with the default compiler:
    #   - "" works.
    if(UNIX AND NOT APPLE)
        target_link_libraries(${target} PRIVATE stdc++fs)
    endif()

    # Colorize GCC/Clang terminal outputs
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fdiagnostics-color=always>)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fcolor-diagnostics>)
    endif()

    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/cpp>
        $<INSTALL_INTERFACE:${Tiny3D_INSTALL_INCLUDE_DIR}>
    )

    # Required for static linking zeromq
    target_compile_definitions(${target} PRIVATE ZMQ_STATIC)

    # Propagate build configuration into source code
    if (BUILD_CUDA_MODULE)
        target_compile_definitions(${target} PRIVATE BUILD_CUDA_MODULE)
        if (ENABLE_CACHED_CUDA_MANAGER)
            target_compile_definitions(${target} PRIVATE ENABLE_CACHED_CUDA_MANAGER)
        endif()
    endif()
    if (BUILD_SYCL_MODULE)
        target_compile_definitions(${target} PRIVATE BUILD_SYCL_MODULE)
        if (ENABLE_SYCL_UNIFIED_SHARED_MEMORY)
            target_compile_definitions(${target} PRIVATE ENABLE_SYCL_UNIFIED_SHARED_MEMORY)
        endif()
    endif()
    if (USE_BLAS)
        target_compile_definitions(${target} PRIVATE USE_BLAS)
    endif()
    if (WITH_IPP)
        target_compile_definitions(${target} PRIVATE WITH_IPP)
    endif()
    if (GLIBCXX_USE_CXX11_ABI)
        target_compile_definitions(${target} PUBLIC _GLIBCXX_USE_CXX11_ABI=1)
    else()
        target_compile_definitions(${target} PUBLIC _GLIBCXX_USE_CXX11_ABI=0)
    endif()

    if(UNIX AND NOT WITH_OPENMP)
        target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-Wno-unknown-pragmas>")
    endif()
    if(WIN32)
        target_compile_definitions(${target} PRIVATE
            WINDOWS
            _CRT_SECURE_NO_DEPRECATE
            _CRT_NONSTDC_NO_DEPRECATE
            _SCL_SECURE_NO_WARNINGS
        )
        if(MSVC)
            target_compile_definitions(${target} PRIVATE NOMINMAX _USE_MATH_DEFINES _ENABLE_EXTENDED_ALIGNED_STORAGE)
            target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/EHsc>)
            # Multi-thread compile, two ways to enable
            # Option 1, at build time: cmake --build . --parallel %NUMBER_OF_PROCESSORS%
            # https://stackoverflow.com/questions/36633074/set-the-number-of-threads-in-a-cmake-build
            # Option 2, at configure time: add /MP flag, no need to use Option 1
            # https://docs.microsoft.com/en-us/cpp/build/reference/mp-build-with-multiple-processes?view=vs-2019
            target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/MP>)
            # Add critical Windows optimization flags for performance parity with Open3D
            target_compile_options(${target} PRIVATE 
                "$<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:/Oi>"    # Enable intrinsic functions
                "$<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:/Ot>"    # Favor fast code over small code  
                "$<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:/Oy->"   # Suppress frame pointer omission (for debugging)
                "$<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:/GT>"    # Supports fiber-safe thread-local storage
                "$<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:/favor:INTEL64>"  # Optimize for x64 processors
                "$<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:/arch:AVX2>"      # Enable AVX2 vectorization
            )
            # Add linker optimizations for release builds
            target_link_options(${target} PRIVATE 
                "$<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:/OPT:REF>"    # Eliminate unreferenced functions/data
                "$<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:/OPT:ICF>"    # Enable COMDAT folding
            )
            # The examples' .pdb files use up a lot of space and cause us to run
            # out of space on GitHub Actions. Compressing gives us another couple of GB.
            target_link_options(${target} PRIVATE /pdbcompress)
        endif()
    elseif(APPLE)
        target_compile_definitions(${target} PRIVATE UNIX APPLE)
    elseif(UNIX)
        target_compile_definitions(${target} PRIVATE UNIX)
    endif()
    if(LINUX_AARCH64)
        target_compile_definitions(${target} PRIVATE LINUX_AARCH64)
    endif()
    target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CUDA>:--expt-extended-lambda>")

    # Require 64-bit indexing in vectorized code
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:ISPC>:--addressing=64>)

    # Set architecture flag
    if(LINUX_AARCH64)
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:ISPC>:--arch=aarch64>)
    else()
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:ISPC>:--arch=x86-64>)
    endif()

    # Turn off fast math for IntelLLVM DPC++ compiler.
    # Fast math does not work with some of our NaN handling logics.
    target_compile_options(${target} PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:IntelLLVM>,$<NOT:$<COMPILE_LANGUAGE:ISPC>>>:-ffp-contract=on>)
    target_compile_options(${target} PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:IntelLLVM>,$<NOT:$<COMPILE_LANGUAGE:ISPC>>>:-fno-fast-math>)

    # Enable strip
    tiny3d_enable_strip(${target})

    # Harderning flags
    target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${HARDENING_CFLAGS}>")
    target_link_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${HARDENING_LDFLAGS}>")
    target_compile_definitions(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${HARDENING_DEFINITIONS}>")

    # RPATH handling. Check current folder, one folder above (cpu/pybind ->
    # libtbb) and the lib sibling folder (bin/Tiny3D -> lib/libTiny3D).  Also
    # check the Python virtual env /lib folder for 3rd party dependency
    # libraries installed with `pip install`
    if (APPLE)
    # Add options to cover the various ways in which tiny3d shared lib or apps can be installed wrt dependent DSOs
        set_target_properties(${target} PROPERTIES 
            INSTALL_RPATH "@loader_path;@loader_path/../;@loader_path/../lib/:@loader_path/../../../../"
    # pybind with tiny3d shared lib is copied, not cmake-installed, so we need to add .. to build rpath 
            BUILD_RPATH "@loader_path;@loader_path/../;@loader_path/../lib/:@loader_path/../../../../")
    elseif(UNIX)
        message(STATUS "Setting RPATH for ${target}")
        # INSTALL_RPATH for C++ binaries.
        set_target_properties(${target} PROPERTIES 
            INSTALL_RPATH "$ORIGIN;$ORIGIN/../;$ORIGIN/../lib/;$ORIGIN/../../../../"
        # BUILD_RPATH for Python wheel libs and app.
            BUILD_RPATH "$ORIGIN;$ORIGIN/../;$ORIGIN/../lib/;$ORIGIN/../../../../")
    endif()

endfunction()

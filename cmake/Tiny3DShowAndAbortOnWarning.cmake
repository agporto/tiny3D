# ----------------------------------------------------------------------------
# -                        Tiny3D: www.tiny3d.org                            -
# ----------------------------------------------------------------------------
# Copyright (c) 2018-2024 www.tiny3d.org
# SPDX-License-Identifier: MIT
# ----------------------------------------------------------------------------

# tiny3d_show_and_abort_on_warning(target)
#
# Enables warnings when compiling <target> and enables treating warnings as errors.
function(tiny3d_show_and_abort_on_warning target)

    set(DISABLE_MSVC_WARNINGS
        /Wv:18         # ignore warnings introduced in Visual Studio 2015 and later.
        /wd4201        # non-standard extension nameless struct (filament includes)
        /wd4310        # cast truncates const value (filament)
        /wd4505        # unreferenced local function has been removed (dirent)
        /wd4127        # conditional expression is const (Eigen)
        /wd4146        # unary minus operator applied to unsigned type, result still unsigned (UnaryEWCPU)
        /wd4189        # local variable is initialized but not referenced (PoissonRecon)
        /wd4324        # structure was padded due to alignment specifier (qhull)
        /wd4706        # assignment within conditional expression (fileIO, ...)
        /wd4100        # unreferenced parameter (many places in Tiny3D code)
        /wd4702        # unreachable code (many places in Tiny3D code)
        /wd4244        # implicit data type conversion (many places in Tiny3D code)
        /wd4245        # signed/unsigned mismatch (visualization, PoissonRecon, ...)
        /wd4267        # conversion from size_t to smaller type (FixedRadiusSearchCUDA, tests)
        /wd4305        # conversion to smaller type in initialization or constructor argument (examples, tests)
        /wd4819        # suppress vs2019+ compiler build error C2220 (Windows)
    )
    set(DISABLE_GNU_CLANG_INTEL_WARNINGS
        -Wno-unused-parameter               # (many places in Tiny3D code)
    )

    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANG_AND_ID:C,MSVC>:/W4 /WX ${DISABLE_MSVC_WARNINGS}>
        $<$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang,Intel>:-Wall -Wextra -Werror ${DISABLE_GNU_CLANG_INTEL_WARNINGS}>
        $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX ${DISABLE_MSVC_WARNINGS}>
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang,Intel>:-Wall -Wextra -Werror ${DISABLE_GNU_CLANG_INTEL_WARNINGS}>
        $<$<COMPILE_LANGUAGE:CUDA>:SHELL:${CUDA_FLAGS}>
        $<$<COMPILE_LANGUAGE:ISPC>:--werror>
    )
endfunction()

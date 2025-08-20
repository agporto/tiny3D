// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#pragma once

/// TINY3D_FIX_MSVC_(...)
///
/// Internal helper function which defers the evaluation of the enclosed
/// expression.
///
/// Use this macro only to workaround non-compliant behaviour of the MSVC
/// preprocessor.
///
/// Note: Could be dropped in the future if the compile flag /Zc:preprocessor
/// can be applied.
#define TINY3D_FIX_MSVC_(...) __VA_ARGS__

/// TINY3D_CONCAT(s1, s2)
///
/// Concatenates the expanded expressions s1 and s2.
#define TINY3D_CONCAT_IMPL_(s1, s2) s1##s2
#define TINY3D_CONCAT(s1, s2) TINY3D_CONCAT_IMPL_(s1, s2)

/// TINY3D_STRINGIFY(s)
///
/// Converts the expanded expression s to a string.
#define TINY3D_STRINGIFY_IMPL_(s) #s
#define TINY3D_STRINGIFY(s) TINY3D_STRINGIFY_IMPL_(s)

/// TINY3D_NUM_ARGS(...)
///
/// Returns the number of supplied arguments.
///
/// Note: Only works for 1-10 arguments.
#define TINY3D_GET_NTH_ARG_(...) \
    TINY3D_FIX_MSVC_(TINY3D_GET_NTH_ARG_IMPL_(__VA_ARGS__))
#define TINY3D_GET_NTH_ARG_IMPL_(arg1, arg2, arg3, arg4, arg5, arg6, arg7, \
                                 arg8, arg9, arg10, N, ...)                \
    N
#define TINY3D_REVERSE_NUM_SEQUENCE_() 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define TINY3D_NUM_ARGS(...) \
    TINY3D_GET_NTH_ARG_(__VA_ARGS__, TINY3D_REVERSE_NUM_SEQUENCE_())

/// TINY3D_OVERLOAD(func, ...)
///
/// Overloads the enumerated macros func1, func2, etc. based on the number of
/// additional arguments.
///
/// Example:
///
/// \code
/// #define FOO_1(x1) foo(x1)
/// #define FOO_2(x1, x2) bar(x1, x2)
/// #define FOO(...) '\'
///     TINY3D_FIX_MSVC_(TINY3D_OVERLOAD(FOO_, __VA_ARGS__)(__VA_ARGS__))
///
/// FOO(1)    -> foo(1)
/// FOO(2, 3) -> bar(2, 3)
/// \endcode
#define TINY3D_OVERLOAD(func, ...) \
    TINY3D_CONCAT(func, TINY3D_NUM_ARGS(__VA_ARGS__))

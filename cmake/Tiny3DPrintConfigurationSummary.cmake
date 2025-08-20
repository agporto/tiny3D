# Internal helper function.
function(tiny3d_aligned_print printed_name printed_valued)
    string(LENGTH "${printed_name}" PRINTED_NAME_LENGTH)
    math(EXPR PRINTED_DOTS_LENGTH "40 - ${PRINTED_NAME_LENGTH}")
    string(REPEAT "." ${PRINTED_DOTS_LENGTH} PRINTED_DOTS)
    message(STATUS "  ${printed_name} ${PRINTED_DOTS} ${printed_valued}")
endfunction()


# tiny3d_print_configuration_summary()
#
# Prints a summary of the current configuration.
function(tiny3d_print_configuration_summary)
    message(STATUS "================================================================================")
    message(STATUS "tiny3d ${PROJECT_VERSION} Configuration Summary")
    message(STATUS "================================================================================")
    message(STATUS "Enabled Features:")
    tiny3d_aligned_print("OpenMP" "${WITH_OPENMP}")
    tiny3d_aligned_print("SYCL Support" "${BUILD_SYCL_MODULE}")
    if(ENABLE_SYCL_UNIFIED_SHARED_MEMORY)
        tiny3d_aligned_print("SYCL unified shared memory" "${ENABLE_SYCL_UNIFIED_SHARED_MEMORY}")
    endif()
    tiny3d_aligned_print("Build Shared Library" "${BUILD_SHARED_LIBS}")
    if(WIN32)
       tiny3d_aligned_print("Use Windows Static Runtime" "${STATIC_WINDOWS_RUNTIME}")
    endif()
    tiny3d_aligned_print("Build Python Module" "${BUILD_PYTHON_MODULE}")
    tiny3d_aligned_print("Build Jupyter Extension" "${BUILD_JUPYTER_EXTENSION}")
    if(GLIBCXX_USE_CXX11_ABI)
        tiny3d_aligned_print("Force GLIBCXX_USE_CXX11_ABI=" "1")
    else()
        tiny3d_aligned_print("Force GLIBCXX_USE_CXX11_ABI=" "0")
    endif()

    message(STATUS "================================================================================")
    message(STATUS "Third-Party Dependencies:")
    set(3RDPARTY_DEPENDENCIES
        Assimp
        BLAS
        curl
        Eigen3
        filament
        fmt
        GLEW
        GLFW
        googletest
        imgui
        ipp
        JPEG
        jsoncpp
        liblzf
        msgpack
        nanoflann
        OpenGL
        PNG
        qhullcpp
        librealsense
        TBB
        tinyfiledialogs
        TinyGLTF
        tinyobjloader
        VTK
        WebRTC
        ZeroMQ
    )
    foreach(dep IN LISTS 3RDPARTY_DEPENDENCIES)
        string(TOLOWER "${dep}" dep_lower)
        string(TOUPPER "${dep}" dep_upper)
        if(TARGET Tiny3D::3rdparty_${dep_lower})
            if(NOT USE_SYSTEM_${dep_upper})
                tiny3d_aligned_print("${dep}" "yes (build from source)")
            else()
                if(3rdparty_${dep_lower}_VERSION)
                    tiny3d_aligned_print("${dep}" "yes (v${3rdparty_${dep_lower}_VERSION})")
                else()
                    tiny3d_aligned_print("${dep}" "yes")
                endif()
            endif()
        else()
            tiny3d_aligned_print("${dep}" "no")
        endif()
    endforeach()
    message(STATUS "================================================================================")

endfunction()

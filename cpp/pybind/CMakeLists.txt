if (NOT Python3_EXECUTABLE)
    message(FATAL_ERROR "Python 3 not found in top level file")
endif()

# We need to get python version to configure some meta files
set(PYTHON_VERSION "${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")


pybind11_add_module(pybind)

add_subdirectory(geometry)
add_subdirectory(io)
add_subdirectory(pipelines)
add_subdirectory(utility)

target_sources(pybind PRIVATE
    docstring.cpp
    tiny3d_pybind.cpp
)

# Include with `#include "pybind/geometry/xxx.h`
target_include_directories(pybind PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)


tiny3d_show_and_abort_on_warning(pybind)
tiny3d_set_global_properties(pybind)
target_link_libraries(pybind PRIVATE Tiny3D::Tiny3D)

tiny3d_link_3rdparty_libraries(pybind)


if (WIN32)
    target_link_options(pybind PUBLIC "/force:multiple")
elseif (APPLE)
    file(GENERATE OUTPUT pybind.map CONTENT
        [=[_PyInit_pybind
        ]=])
    target_link_options(pybind PRIVATE $<$<CONFIG:Release>:
        -Wl,-exported_symbols_list
        "${CMAKE_CURRENT_BINARY_DIR}/pybind.map" >)
elseif (UNIX)   # Linux
    file(GENERATE OUTPUT pybind.map CONTENT
        [=[{
    global:
        PyInit_pybind;
    local:
        *;
};]=])
    target_link_options(pybind PRIVATE $<$<CONFIG:Release>:
        "-Wl,--version-script=${CMAKE_CURRENT_BINARY_DIR}/pybind.map" >)
    target_link_options(pybind PRIVATE "-flto=auto")
endif()


# At `make`: tiny3d.so (or the equivalents) will be created at
# PYTHON_COMPILED_MODULE_DIR. The default location is
# `build/lib/${CMAKE_BUILD_TYPE}/Python/{cpu|cuda}`
set(PYTHON_COMPILED_MODULE_DIR
    "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Python/cpu")

set_target_properties(pybind PROPERTIES
                      FOLDER "Python"
                      LIBRARY_OUTPUT_DIRECTORY "${PYTHON_COMPILED_MODULE_DIR}"
                      ARCHIVE_OUTPUT_DIRECTORY "${PYTHON_COMPILED_MODULE_DIR}")


if (BUILD_SHARED_LIBS)
    if (WIN32)    # CMake does not add soversion suffix to WIN32 DLLs
        set(_libtiny3d_soname "Tiny3D.dll")
    elseif(APPLE)
        set(_libtiny3d_soname "libTiny3D.${TINY3D_ABI_VERSION}.dylib")
    else()
        set(_libtiny3d_soname "libTiny3D.so.${TINY3D_ABI_VERSION}")
    endif()
    add_custom_command(TARGET pybind POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Tiny3D> $<TARGET_FILE_DIR:pybind>/${_libtiny3d_soname}
    )
endif()

# Use `make python-package` to create the python package in the build directory
# The python package will be created at PYTHON_PACKAGE_DST_DIR. It contains:
# 1) Pure-python code and misc files, copied from python/package
# 2) The compiled python-C++ module, i.e. tiny3d.so (or the equivalents)
# 3) Configured files and supporting files
# Note: `make python-package` clears PYTHON_COMPILED_MODULE_DIR first every time
set(PYTHON_PACKAGE_SRC_DIR "${PROJECT_SOURCE_DIR}/python")
set(PYTHON_PACKAGE_DST_DIR "${CMAKE_BINARY_DIR}/lib/python_package")
message(STATUS "PYPI_PACKAGE_NAME: ${PYPI_PACKAGE_NAME}")


# add the tiny3d python module first
set(COMPILED_MODULE_PATH_LIST $<TARGET_FILE:pybind>)
# add the tiny3d DSO / DLL if shared
if (BUILD_SHARED_LIBS)
    list(APPEND COMPILED_MODULE_PATH_LIST $<TARGET_FILE_DIR:pybind>/${_libtiny3d_soname})
endif()

set(GENERATED_OUTPUTS "")

configure_file("_build_config.py.in"
               "${CMAKE_BINARY_DIR}/lib/_build_config.py.in")
file(GENERATE
    OUTPUT "${PYTHON_COMPILED_MODULE_DIR}/_build_config.py"
    INPUT "${CMAKE_BINARY_DIR}/lib/_build_config.py.in"
)

add_custom_target(python-package
    COMMAND ${CMAKE_COMMAND}
            -DPYTHON_PACKAGE_SRC_DIR=${PYTHON_PACKAGE_SRC_DIR}
            -DPYTHON_PACKAGE_DST_DIR=${PYTHON_PACKAGE_DST_DIR}
            -DPYTHON_COMPILED_MODULE_DIR=${PYTHON_COMPILED_MODULE_DIR}
            -DPYTHON_VERSION=${PYTHON_VERSION}
            "-DCOMPILED_MODULE_PATH_LIST=${COMPILED_MODULE_PATH_LIST}"
            -DPROJECT_EMAIL=${PROJECT_EMAIL}
            -DPROJECT_HOMEPAGE_URL=${PROJECT_HOMEPAGE_URL}
            -DPROJECT_DOCS=${PROJECT_DOCS}
            -DPROJECT_CODE=${PROJECT_CODE}
            -DPROJECT_ISSUES=${PROJECT_ISSUES}
            -DPROJECT_VERSION=${TINY3D_VERSION_FULL}
            -DPROJECT_DESCRIPTION=${PROJECT_DESCRIPTION}
            -DPROJECT_VERSION_THREE_NUMBER=${PROJECT_VERSION_THREE_NUMBER}
            -DPYPI_PACKAGE_NAME=${PYPI_PACKAGE_NAME}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/make_python_package.cmake
    VERBATIM
    DEPENDS ${GENERATED_OUTPUTS}
)

# Use `make pip-package` to create the pip package in the build directory
add_custom_target(pip-package
    COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --dist-dir pip_package
    COMMAND echo "pip wheel created at ${PYTHON_PACKAGE_DST_DIR}/pip_package"
    WORKING_DIRECTORY ${PYTHON_PACKAGE_DST_DIR}
    DEPENDS python-package
)

# Use `make install-pip-package` to install pip wheel package to the current
# python environment.
add_custom_target(install-pip-package
    COMMAND ${CMAKE_COMMAND}
            -DPYTHON_PACKAGE_DST_DIR=${PYTHON_PACKAGE_DST_DIR}
            -DPython3_EXECUTABLE=${Python3_EXECUTABLE}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/make_install_pip_package.cmake
    DEPENDS pip-package
)

# FOR DEBUGGING ONLY Use `make install-python-package` to build and install
# python package in the current python environment. This is substantially
# faster than `make install-pip-package`. However this approach does not create
# wheel or egg files and does not take care of dependencies thus not suitable
# for deployment.
# Ref: https://stackoverflow.com/a/33791008/1255535
add_custom_target(install-python-package
    COMMAND ${Python3_EXECUTABLE} setup.py install --single-version-externally-managed --root=/
    WORKING_DIRECTORY ${PYTHON_PACKAGE_DST_DIR}
    DEPENDS python-package
)

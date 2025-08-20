# This is packaging for the Tiny3D library. See
# cpp/apps/Tiny3DViewer/Debian/CMakeLists.txt for packaging the Debian Tiny3D
# viewer
set(CPACK_GENERATOR TXZ)
if(WIN32)
    set(CPACK_GENERATOR ZIP)
endif()
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${TINY3D_VERSION})
set(CPACK_PACKAGE_VENDOR "Tiny3D Team")
set(CPACK_PACKAGE_CONTACT "${PROJECT_EMAIL}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/docs/_static/tiny3d_logo.ico")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/package")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
string(TOLOWER ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR} _sys)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (GLIBCXX_USE_CXX11_ABI)  # Name follows libtorch convention
        set(_sys ${_sys}-cxx11-abi)
    else()
        set(_sys ${_sys}-pre-cxx11-abi)
    endif()
endif()
if (NOT MSVC)
    set(CPACK_STRIP_FILES ON)  # Don't strip MSVC Debug build
endif()
set(CPACK_PACKAGE_FILE_NAME
    "tiny3d-devel-${_sys}-${TINY3D_VERSION_FULL}")
set(CPACK_THREADS 0)  # Use all cores for compressing package

include(CPack)

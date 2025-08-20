#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Tiny3D::Tiny3D" for configuration "Release"
set_property(TARGET Tiny3D::Tiny3D APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Tiny3D::Tiny3D PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libTiny3D.so.0.0.0"
  IMPORTED_SONAME_RELEASE "libTiny3D.so.0.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS Tiny3D::Tiny3D )
list(APPEND _IMPORT_CHECK_FILES_FOR_Tiny3D::Tiny3D "${_IMPORT_PREFIX}/lib/libTiny3D.so.0.0.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

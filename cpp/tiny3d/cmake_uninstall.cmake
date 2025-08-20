# https://cmake.org/Wiki/CMake_FAQ#Can_I_do_.22make_uninstall.22_with_CMake.3F
if(NOT EXISTS "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest: /home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/install_manifest.txt")
endif(NOT EXISTS "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/install_manifest.txt")

file(READ "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach(file ${files})
    message(STATUS "Uninstalling $ENV{DESTDIR}${file}")
    if(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
        execute_process(COMMAND "/usr/bin/cmake" -E remove "$ENV{DESTDIR}${file}"
        RESULT_VARIABLE rm_retval)
        if(NOT "${rm_retval}" STREQUAL 0)
            message(FATAL_ERROR "Problem when removing $ENV{DESTDIR}${file}")
        endif(NOT "${rm_retval}" STREQUAL 0)
    else()
        message(STATUS "File $ENV{DESTDIR}${file} does not exist.")
    endif()
endforeach()

# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.22.1)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(ext_pybind11-populate NONE)



include(ExternalProject)
ExternalProject_Add(ext_pybind11-populate
                     "UPDATE_DISCONNECTED" "False" "PREFIX" "pybind11" "URL" "https://github.com/pybind/pybind11/archive/refs/tags/v2.13.1.tar.gz" "URL_HASH" "SHA256=51631e88960a8856f9c497027f55c9f2f9115cafb08c0005439838a05ba17bfc" "DOWNLOAD_DIR" "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/3rdparty_downloads/pybind11"
                    SOURCE_DIR          "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/_deps/ext_pybind11-src"
                    BINARY_DIR          "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/_deps/ext_pybind11-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)



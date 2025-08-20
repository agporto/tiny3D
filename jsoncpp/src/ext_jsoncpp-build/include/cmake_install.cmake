# Install script for directory: /home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp/src/ext_jsoncpp/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/json" TYPE FILE FILES
    "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp/src/ext_jsoncpp/include/json/allocator.h"
    "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp/src/ext_jsoncpp/include/json/assertions.h"
    "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp/src/ext_jsoncpp/include/json/config.h"
    "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp/src/ext_jsoncpp/include/json/forwards.h"
    "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp/src/ext_jsoncpp/include/json/json.h"
    "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp/src/ext_jsoncpp/include/json/json_features.h"
    "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp/src/ext_jsoncpp/include/json/reader.h"
    "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp/src/ext_jsoncpp/include/json/value.h"
    "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp/src/ext_jsoncpp/include/json/version.h"
    "/home/arthur.porto/UFL Dropbox/Arthur Porto/Year_2025/fastmorph/tiny3D/jsoncpp/src/ext_jsoncpp/include/json/writer.h"
    )
endif()


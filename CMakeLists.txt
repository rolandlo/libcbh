# Copyright (C) 2026  Roland Lötscher
# This file is part of libcbh
#
# libcbh is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation.
#
# libcbh is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libcbh. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.15)
project(libcbh VERSION 1.0.0 DESCRIPTION "cbh chess database decoder")

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CXX_FLAGS)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# cbh
file(GLOB cbh_SRC
  src/cbh_decode_annotation.cpp
  src/cbh_decode_annotator.cpp
  src/cbh_decode_game.cpp
  src/cbh_decode_player.cpp
  src/cbh_decode_source.cpp
  src/cbh_decode_tournament.cpp
  src/cbh_position.cpp
  src/cbh_codec.cpp
  src/position.cpp)
add_library(cbh SHARED ${cbh_SRC})

file(GLOB cbh_HDR
  include/cbh.h
  include/error.h
  include/interface.h
  include/nags.h
)
set_target_properties(cbh PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  PUBLIC_HEADER "${cbh_HDR}"
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN 1
)

configure_file(cbh.pc.in cbh.pc @ONLY)
target_include_directories(cbh PUBLIC include)
target_include_directories(cbh PRIVATE .)
install(TARGETS cbh
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libcbh)
install(FILES ${CMAKE_BINARY_DIR}/cbh.pc
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

option(GTEST "Build unit tests" OFF)

if(GTEST)
  add_subdirectory(gtest)
endif()

# Copyright (C) 2026  Roland Lötscher
# This file is part of libcbh
#
# Libcbh is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation.
#
# Libcbh is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Libcbh. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.15)
project(libcbh)
set(CPACK_PACKAGE_VERSION 1.0)
set(
  CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "cbh chess database decoder"
)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CXX_FLAGS)
  set(CMAKE_BUILD_TYPE "Release")
endif()


# libcbh
file(GLOB LIBCBH_SRC src/*.h src/*.cpp)
add_executable(libcbh ${LIBCBH_SRC})

install(TARGETS libcbh DESTINATION libcbh)

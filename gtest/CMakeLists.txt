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

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "")

# googletest
if(NOT IS_DIRECTORY "${CMAKE_BINARY_DIR}/googletest")
  find_package(Git)
  execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/google/googletest.git)
endif()

add_subdirectory(${CMAKE_BINARY_DIR}/googletest ${CMAKE_BINARY_DIR}/googletest/Build EXCLUDE_FROM_ALL)

# codec_tests
file(GLOB GTEST_SRC *.cpp)
add_executable(codec_tests ${GTEST_SRC})
target_compile_definitions(codec_tests PRIVATE -DTESTDIR=\"${CMAKE_CURRENT_LIST_DIR}/\")
target_link_libraries(codec_tests PRIVATE cbh gtest_main)
target_include_directories(codec_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(codec_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

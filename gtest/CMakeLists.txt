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

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "")

# googletest
if(NOT IS_DIRECTORY "${CMAKE_BINARY_DIR}/googletest")
  find_package(Git)
  execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/google/googletest.git)
endif()
set(gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll")
add_subdirectory(${CMAKE_BINARY_DIR}/googletest ${CMAKE_BINARY_DIR}/googletest/Build EXCLUDE_FROM_ALL)

# cbh_codec_sources
set(CODEC_BASE
  ../src/codec_cbh.cpp
  ../src/cbh_position.cpp
  ../src/cbh_decode_game.cpp
  ../src/cbh_decode_player.cpp
  ../src/cbh_decode_tournament.cpp
  ../src/cbh_decode_annotator.cpp
  ../src/cbh_decode_annotation.cpp
  ../src/cbh_decode_source.cpp
  ../src/game.cpp ../src/position.cpp
)
add_library(codec_base ${CODEC_BASE})
target_include_directories(codec_base PUBLIC ../src)
find_package(Threads REQUIRED)
target_link_libraries(codec_base PUBLIC ${CMAKE_THREAD_LIBS_INIT})

# codec_tests
file(GLOB GTEST_SRC *.cpp)
add_executable(codec_tests ${GTEST_SRC})
target_compile_definitions(codec_tests PRIVATE -DSCID_TESTDIR=\"${CMAKE_CURRENT_LIST_DIR}/\")
target_link_libraries(codec_tests PRIVATE codec_base gtest_main)

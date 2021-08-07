# Stone of Orthanc
# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2021 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.



#####################################################################
## Sanity check of the configuration
#####################################################################

include(${ORTHANC_STONE_ROOT}/../Resources/CMake/OrthancStoneConfiguration.cmake)

if (ORTHANC_SANDBOXED)
  message(FATAL_ERROR "Cannot enable SDL in sandboxed environments")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten" OR
    CMAKE_SYSTEM_NAME STREQUAL "PNaCl" OR
    CMAKE_SYSTEM_NAME STREQUAL "NaCl32" OR
    CMAKE_SYSTEM_NAME STREQUAL "NaCl64")
  message(FATAL_ERROR "Trying to use a Web compiler for a native build")
endif()



#####################################################################
## Configure SDL
#####################################################################

message("SDL is enabled")
include(${CMAKE_CURRENT_LIST_DIR}/SdlConfiguration.cmake)

add_definitions(
  -DORTHANC_ENABLE_SDL=1
  -DORTHANC_ENABLE_WASM=0
  )

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND
    NOT MSVC)
  # This is necessary when compiling EXE for Windows using MinGW
  link_libraries(mingw32)
endif()



#####################################################################
## Additional source files for SDL
#####################################################################

list(APPEND ORTHANC_STONE_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/SdlViewport.cpp
  ${CMAKE_CURRENT_LIST_DIR}/SdlWindow.cpp
  ${SDL_SOURCES}
  )

if (ENABLE_OPENGL)
  list(APPEND ORTHANC_STONE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/SdlOpenGLContext.cpp
    )
endif()

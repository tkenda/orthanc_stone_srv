# Stone of Orthanc
# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2021 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program. If not, see
# <http://www.gnu.org/licenses/>.


if (STATIC_BUILD OR NOT USE_SYSTEM_GLEW)
  SET(GLEW_SOURCES_DIR ${CMAKE_BINARY_DIR}/glew-2.1.0)
  SET(GLEW_URL "http://orthanc.osimis.io/ThirdPartyDownloads/glew-2.1.0.tgz")
  SET(GLEW_MD5 "b2ab12331033ddfaa50dc39345343980")
  DownloadPackage(${GLEW_MD5} ${GLEW_URL} "${GLEW_SOURCES_DIR}")

  set(GLEW_SOURCES
    ${GLEW_SOURCES_DIR}/src/glew.c
    )

  include_directories(${GLEW_SOURCES_DIR}/include)

  add_definitions(
    -DGLEW_STATIC=1
    )

else()
  include(FindGLEW)
  if (NOT GLEW_FOUND)
    message(FATAL_ERROR "Please install the libglew-dev package")
  endif()
  
  include_directories(${GLEW_INCLUDE_DIRS})
  link_libraries(${GLEW_LIBRARIES})
endif()

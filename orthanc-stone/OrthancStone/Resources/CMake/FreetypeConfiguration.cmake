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


if (STATIC_BUILD OR NOT USE_SYSTEM_FREETYPE)
  set(FREETYPE_SOURCES_DIR ${CMAKE_BINARY_DIR}/freetype-2.9.1)
  set(FREETYPE_URL "http://orthanc.osimis.io/ThirdPartyDownloads/freetype-2.9.1.tar.gz")
  set(FREETYPE_MD5 "3adb0e35d3c100c456357345ccfa8056")

  DownloadPackage(${FREETYPE_MD5} ${FREETYPE_URL} "${FREETYPE_SOURCES_DIR}")

  include_directories(BEFORE
    ${FREETYPE_SOURCES_DIR}/include/
    )

  add_definitions(
    -DFT2_BUILD_LIBRARY
    -DFT_CONFIG_OPTION_NO_ASSEMBLER
    )
    
  set(FREETYPE_SOURCES
    ${FREETYPE_SOURCES_DIR}/src/autofit/autofit.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftbase.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftbbox.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftbdf.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftbitmap.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftcid.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftfstype.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftgasp.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftglyph.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftgxval.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftinit.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftmm.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftotval.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftpatent.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftpfr.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftstroke.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftsynth.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftsystem.c
    ${FREETYPE_SOURCES_DIR}/src/base/fttype1.c
    ${FREETYPE_SOURCES_DIR}/src/base/ftwinfnt.c
    ${FREETYPE_SOURCES_DIR}/src/bdf/bdf.c
    ${FREETYPE_SOURCES_DIR}/src/bzip2/ftbzip2.c
    ${FREETYPE_SOURCES_DIR}/src/cache/ftcache.c
    ${FREETYPE_SOURCES_DIR}/src/cff/cff.c
    ${FREETYPE_SOURCES_DIR}/src/cid/type1cid.c
    ${FREETYPE_SOURCES_DIR}/src/gzip/ftgzip.c
    ${FREETYPE_SOURCES_DIR}/src/lzw/ftlzw.c
    ${FREETYPE_SOURCES_DIR}/src/pcf/pcf.c
    ${FREETYPE_SOURCES_DIR}/src/pfr/pfr.c
    ${FREETYPE_SOURCES_DIR}/src/psaux/psaux.c
    ${FREETYPE_SOURCES_DIR}/src/pshinter/pshinter.c
    ${FREETYPE_SOURCES_DIR}/src/psnames/psnames.c
    ${FREETYPE_SOURCES_DIR}/src/raster/raster.c
    ${FREETYPE_SOURCES_DIR}/src/sfnt/sfnt.c
    ${FREETYPE_SOURCES_DIR}/src/smooth/smooth.c
    ${FREETYPE_SOURCES_DIR}/src/truetype/truetype.c
    ${FREETYPE_SOURCES_DIR}/src/type1/type1.c
    ${FREETYPE_SOURCES_DIR}/src/type42/type42.c
    ${FREETYPE_SOURCES_DIR}/src/winfonts/winfnt.c
    )

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND FREETYPE_SOURCES
      ${FREETYPE_SOURCES_DIR}/builds/windows/ftdebug.c
      )
  endif()

  foreach(header
      ${FREETYPE_SOURCES_DIR}/include/freetype/config/ftconfig.h
      ${FREETYPE_SOURCES_DIR}/include/freetype/config/ftoption.h
      )

    set_source_files_properties(
      ${FREETYPE_SOURCES}
      PROPERTIES OBJECT_DEPENDS ${header}
      )
  endforeach()

  source_group(ThirdParty\\Freetype REGULAR_EXPRESSION ${FREETYPE_SOURCES_DIR}/.*)

else()
  include(FindFreetype)

  if (NOT FREETYPE_FOUND)
    message(FATAL_ERROR "Please install the libfreetype6-dev package")
  endif()

  include_directories(${FREETYPE_INCLUDE_DIRS})
  link_libraries(${FREETYPE_LIBRARIES})
endif()

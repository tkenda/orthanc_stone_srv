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


if (STATIC_BUILD OR NOT USE_SYSTEM_PIXMAN)
  SET(PIXMAN_SOURCES_DIR ${CMAKE_BINARY_DIR}/pixman-0.34.0)
  SET(PIXMAN_URL "http://orthanc.osimis.io/ThirdPartyDownloads/pixman-0.34.0.tar.gz")
  SET(PIXMAN_MD5 "e80ebae4da01e77f68744319f01d52a3")

  if (IS_DIRECTORY "${PIXMAN_SOURCES_DIR}")
    set(FirstRun OFF)
  else()
    set(FirstRun ON)
  endif()

  DownloadPackage(${PIXMAN_MD5} ${PIXMAN_URL} "${PIXMAN_SOURCES_DIR}")

  # Apply a patch for NaCl32: This bypasses the custom implementation of
  # "cpuid" that makes use of assembly code leading to "unrecognized
  # instruction" when validating ".nexe" files using "ncval"
  execute_process(
    COMMAND ${PATCH_EXECUTABLE} -p0 -N -i ${CMAKE_CURRENT_LIST_DIR}/PixmanConfiguration.patch
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE Failure
    )

  if (Failure AND FirstRun)
    message(FATAL_ERROR "Error while patching a file")
  endif()

  set(PIXMAN_VERSION_MAJOR 0)
  set(PIXMAN_VERSION_MINOR 34)
  set(PIXMAN_VERSION_MICRO 0)
  configure_file(
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-version.h.in
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-version.h)

  list(APPEND PIXMAN_SOURCES 
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-access-accessors.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-access.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-arm.c
    #${PIXMAN_SOURCES_DIR}/pixman/pixman-arm-neon.c
    #${PIXMAN_SOURCES_DIR}/pixman/pixman-arm-simd.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-bits-image.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-combine32.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-combine-float.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-conical-gradient.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-edge-accessors.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-edge.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-fast-path.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-filter.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-general.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-glyph.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-gradient-walker.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-image.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-implementation.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-linear-gradient.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-matrix.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-mips.c
    #${PIXMAN_SOURCES_DIR}/pixman/pixman-mips-dspr2.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-mmx.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-noop.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-ppc.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-radial-gradient.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-region16.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-region32.c
    #${PIXMAN_SOURCES_DIR}/pixman/pixman-region.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-solid-fill.c
    #${PIXMAN_SOURCES_DIR}/pixman/pixman-sse2.c
    #${PIXMAN_SOURCES_DIR}/pixman/pixman-ssse3.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-timer.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-trap.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-utils.c
    #${PIXMAN_SOURCES_DIR}/pixman/pixman-vmx.c
    ${PIXMAN_SOURCES_DIR}/pixman/pixman-x86.c
    )
  
  set(PIXMAN_DEFINITIONS "PACKAGE=\"pixman\"")

  if (CMAKE_SYSTEM_PROCESSOR)
    message("Processor: ${CMAKE_SYSTEM_PROCESSOR}")
  else()
    message("Processor: Not applicable")
  endif()


  ##########################
  ## Portable Google NaCl
  ##########################

  if (CMAKE_SYSTEM_NAME STREQUAL "PNaCl")
    # No hardware acceleration
    set(PIXMAN_DEFINITIONS "${PIXMAN_DEFINITIONS};TLS=__thread")

  elseif (CMAKE_SYSTEM_NAME STREQUAL "Emscripten" OR
          CMAKE_SYSTEM_NAME STREQUAL "Android")
    ##########################
    ## Emscripten (asm.js)
    ##########################

    # No threading support
    set(PIXMAN_DEFINITIONS "${PIXMAN_DEFINITIONS};PIXMAN_NO_TLS=1;HAVE_GCC_VECTOR_EXTENSIONS")

  elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")

    ##########################
    ## Windows 32 or 64
    ##########################

    if (CMAKE_COMPILER_IS_GNUCXX)
      set(PIXMAN_DEFINITIONS "${PIXMAN_DEFINITIONS};TLS=__thread;HAVE_GCC_VECTOR_EXTENSIONS;HAVE_BUILTIN_CLZ;HAVE_FEDIVBYZERO=1;HAVE_FENV_H=1;HAVE_MPROTECT=1;HAVE_FLOAT128;HAVE_POSIX_MEMALIGN;USE_GCC_INLINE_ASM=1;HAVE_GETPAGESIZE=1")

      # The option "-mstackrealign" is necessary to avoid a crash on
      # Windows if enabling SSE2. As an alternative, it is possible to
      # fully disable hardware acceleration.
      # https://bugs.freedesktop.org/show_bug.cgi?id=68300#c4
      SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -mssse3 -mstackrealign")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -mssse3 -mstackrealign")
    endif()

    list(APPEND PIXMAN_SOURCES 
      ${PIXMAN_SOURCES_DIR}/pixman/pixman-sse2.c
      ${PIXMAN_SOURCES_DIR}/pixman/pixman-ssse3.c
      )

    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
      # Only enable MMX on Windows 32
      add_definitions(
        -DUSE_X86_MMX=1
        )
    endif()

    add_definitions(
      -DUSE_SSE2=1
      -DUSE_SSSE3=1
      )


    ##########################
    ## Generic x86 processor
    ##########################

  elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86" OR
      CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR
      CMAKE_SYSTEM_NAME STREQUAL "NaCl32" OR
      CMAKE_SYSTEM_NAME STREQUAL "NaCl64" OR
      CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    set(PIXMAN_DEFINITIONS "${PIXMAN_DEFINITIONS};TLS=__thread;HAVE_GCC_VECTOR_EXTENSIONS;HAVE_BUILTIN_CLZ;HAVE_MPROTECT=1;HAVE_FLOAT128;HAVE_POSIX_MEMALIGN;USE_GCC_INLINE_ASM;HAVE_GETPAGESIZE=1")

    if (${CMAKE_SYSTEM_NAME} STREQUAL "NaCl32" OR
        ${CMAKE_SYSTEM_NAME} STREQUAL "NaCl64")
      # The MMX instructions lead to "unrecognized instruction" when
      # validating ".nexe" files using "ncval", disable them
    else()
      #add_definitions(-DUSE_X86_MMX=1)
    endif()

    list(APPEND PIXMAN_SOURCES 
      ${PIXMAN_SOURCES_DIR}/pixman/pixman-sse2.c
      ${PIXMAN_SOURCES_DIR}/pixman/pixman-ssse3.c
      )
    add_definitions(
      -DUSE_SSE2=1
      -DUSE_SSSE3=1
      )
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -mssse3")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -mssse3")


    ##########################
    ## ARM processor
    ##########################

  elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv5te" OR
      CMAKE_SYSTEM_PROCESSOR STREQUAL "armv6" OR
      CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l" OR
      CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7-a" OR
      CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")

    set(PIXMAN_DEFINITIONS "${PIXMAN_DEFINITIONS};TLS=__thread")

    if (NEON)
      message("Processor with NEON instructions")
      list(APPEND PIXMAN_SOURCES 
        ${PIXMAN_SOURCES_DIR}/pixman/pixman-arm-neon.c
        ${PIXMAN_SOURCES_DIR}/pixman/pixman-arm-neon-asm.S
        ${PIXMAN_SOURCES_DIR}/pixman/pixman-arm-neon-asm-bilinear.S
        )
      add_definitions(
        -DUSE_ARM_NEON=1
        )
    elseif()
      message("Processor without NEON instructions")
    endif()

    add_definitions(
      -DUSE_ARM_SIMD=1
      )
    list(APPEND PIXMAN_SOURCES 
      ${PIXMAN_SOURCES_DIR}/pixman/pixman-arm.c
      ${PIXMAN_SOURCES_DIR}/pixman/pixman-arm-simd-asm.S
      ${PIXMAN_SOURCES_DIR}/pixman/pixman-arm-simd-asm-scaled.S
      ${PIXMAN_SOURCES_DIR}/pixman/pixman-arm-simd.c
      )

  else()
    message(FATAL_ERROR "Support your platform here")
  endif()


  include_directories(
    ${PIXMAN_SOURCES_DIR}/pixman
    )

  set_property(
    SOURCE ${PIXMAN_SOURCES}
    PROPERTY COMPILE_DEFINITIONS ${PIXMAN_DEFINITIONS}
    )

else()

  pkg_search_module(PIXMAN REQUIRED pixman-1)
  include_directories(${PIXMAN_INCLUDE_DIRS})
  link_libraries(${PIXMAN_LIBRARIES})

endif()

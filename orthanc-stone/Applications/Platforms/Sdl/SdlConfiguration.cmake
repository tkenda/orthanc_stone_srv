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


if (STATIC_BUILD OR NOT USE_SYSTEM_SDL)
  SET(SDL_SOURCES_DIR ${CMAKE_BINARY_DIR}/SDL2-2.0.4)
  SET(SDL_URL "http://orthanc.osimis.io/ThirdPartyDownloads/SDL2-2.0.4.tar.gz")
  SET(SDL_MD5 "44fc4a023349933e7f5d7a582f7b886e")
  DownloadPackage(${SDL_MD5} ${SDL_URL} "${SDL_SOURCES_DIR}")

  include_directories(${SDL_SOURCES_DIR}/include)

  set(TMP "${SDL_SOURCES_DIR}/include/SDL_config_premake.h")
  if (NOT EXISTS "${TMP}")
    file(WRITE "${TMP}" "
#include \"SDL_platform.h\"
#define HAVE_STDARG_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDINT_H 1
")
  endif()

  # General source files
  file(GLOB SDL_SOURCES
    ${SDL_SOURCES_DIR}/src/*.c
    ${SDL_SOURCES_DIR}/src/atomic/*.c
    ${SDL_SOURCES_DIR}/src/audio/*.c
    ${SDL_SOURCES_DIR}/src/cpuinfo/*.c
    ${SDL_SOURCES_DIR}/src/dynapi/*.c
    ${SDL_SOURCES_DIR}/src/events/*.c
    ${SDL_SOURCES_DIR}/src/file/*.c
    ${SDL_SOURCES_DIR}/src/haptic/*.c
    ${SDL_SOURCES_DIR}/src/joystick/*.c
    ${SDL_SOURCES_DIR}/src/libm/*.c
    ${SDL_SOURCES_DIR}/src/power/*.c
    ${SDL_SOURCES_DIR}/src/render/*.c
    ${SDL_SOURCES_DIR}/src/stdlib/*.c
    ${SDL_SOURCES_DIR}/src/thread/*.c
    ${SDL_SOURCES_DIR}/src/timer/*.c
    ${SDL_SOURCES_DIR}/src/video/*.c

    ${SDL_SOURCES_DIR}/src/loadso/dummy/*.c
    #${SDL_SOURCES_DIR}/src/timer/dummy/*.c
    ${SDL_SOURCES_DIR}/src/audio/dummy/*.c
    ${SDL_SOURCES_DIR}/src/filesystem/dummy/*.c
    ${SDL_SOURCES_DIR}/src/haptic/dummy/*.c
    ${SDL_SOURCES_DIR}/src/joystick/dummy/*.c
    #${SDL_SOURCES_DIR}/src/main/dummy/*.c
    ${SDL_SOURCES_DIR}/src/video/dummy/*.c
    )

  add_definitions(
    -DUSING_PREMAKE_CONFIG_H=1

    -DSDL_AUDIO_DISABLED=1
    -DSDL_AUDIO_DRIVER_DUMMY=1
    -DSDL_FILESYSTEM_DISABLED=1
    -DSDL_FILESYSTEM_DUMMY=1
    -DSDL_FILE_DISABLED=1
    -DSDL_HAPTIC_DISABLED=1
    -DSDL_JOYSTICK_DISABLED=1

    #-DSDL_THREADS_DISABLED=1
    )

  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    file(GLOB TMP
      ${SDL_SOURCES_DIR}/src/core/linux/*.c
      ${SDL_SOURCES_DIR}/src/loadso/dlopen/*.c
      ${SDL_SOURCES_DIR}/src/render/software/*.c
      ${SDL_SOURCES_DIR}/src/thread/pthread/*.c
      ${SDL_SOURCES_DIR}/src/timer/unix/*.c
      ${SDL_SOURCES_DIR}/src/video/x11/*.c
      )

    list(APPEND SDL_SOURCES ${TMP})

    add_definitions(
      -DSDL_LOADSO_DLOPEN=1
      -DSDL_THREAD_PTHREAD=1
      -DSDL_TIMER_UNIX=1
      -DSDL_POWER_DISABLED=1

      -DSDL_VIDEO_DRIVER_X11=1

      -DSDL_ASSEMBLY_ROUTINES=1
      -DSDL_THREAD_PTHREAD_RECURSIVE_MUTEX=1
      -DSDL_VIDEO_DRIVER_X11_SUPPORTS_GENERIC_EVENTS=1
      -DHAVE_GCC_SYNC_LOCK_TEST_AND_SET=1
      )

    link_libraries(X11 Xext)

    if (NOT CMAKE_SYSTEM_VERSION STREQUAL "Raspberry")
      # Raspberry Pi has no support for OpenGL
      file(GLOB TMP
        ${SDL_SOURCES_DIR}/src/render/opengl/*.c
        ${SDL_SOURCES_DIR}/src/render/opengles2/*.c
        )

      list(APPEND SDL_SOURCES ${TMP})

      add_definitions(
        -DSDL_VIDEO_OPENGL=1
        -DSDL_VIDEO_OPENGL_ES2=1
        -DSDL_VIDEO_RENDER_OGL=1
        -DSDL_VIDEO_RENDER_OGL_ES2=1
        -DSDL_VIDEO_OPENGL_GLX=1
        -DSDL_VIDEO_OPENGL_EGL=1
        )
    endif()

  elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    file(GLOB TMP
      ${SDL_SOURCES_DIR}/src/audio/directsound/*.c
      ${SDL_SOURCES_DIR}/src/audio/disk/*.c
      ${SDL_SOURCES_DIR}/src/audio/winmm/*.c
      ${SDL_SOURCES_DIR}/src/joystick/windows/*.c
      ${SDL_SOURCES_DIR}/src/haptic/windows/*.c
      ${SDL_SOURCES_DIR}/src/power/windows/*.c

      ${SDL_SOURCES_DIR}/src/main/windows/*.c
      ${SDL_SOURCES_DIR}/src/core/windows/*.c
      ${SDL_SOURCES_DIR}/src/loadso/windows/*.c
      ${SDL_SOURCES_DIR}/src/render/direct3d/*.c
      ${SDL_SOURCES_DIR}/src/render/direct3d11/*.c
      ${SDL_SOURCES_DIR}/src/render/opengl/*.c
      ${SDL_SOURCES_DIR}/src/render/psp/*.c
      ${SDL_SOURCES_DIR}/src/render/opengles/*.c
      ${SDL_SOURCES_DIR}/src/render/opengles2/*.c
      ${SDL_SOURCES_DIR}/src/render/software/*.c
      ${SDL_SOURCES_DIR}/src/thread/generic/SDL_syscond.c   # Don't include more files from "thread/generic/*.c"!
      ${SDL_SOURCES_DIR}/src/thread/windows/*.c
      ${SDL_SOURCES_DIR}/src/timer/windows/*.c
      ${SDL_SOURCES_DIR}/src/video/windows/*.c
      ${SDL_SOURCES_DIR}/src/windows/dlopen/*.c
      )

    list(APPEND SDL_SOURCES ${TMP})

    # NB: OpenGL ES headers are not available in MinGW-W64
    add_definitions(
      -DSDL_LOADSO_WINDOWS=1
      -DSDL_THREAD_WINDOWS=1
      -DSDL_TIMER_WINDOWS=1
      -DSDL_POWER_WINDOWS=1

      -DSDL_VIDEO_OPENGL=1
      -DSDL_VIDEO_OPENGL_WGL=1
      -DSDL_VIDEO_RENDER_D3D=1
      -DSDL_VIDEO_RENDER_OGL=1
      -DSDL_VIDEO_DRIVER_WINDOWS=1
      )

    if (MSVC)
      add_definitions(
        -D__FLTUSED__
        -DHAVE_LIBC=1
      )
    else()
      add_definitions(
        -DHAVE_GCC_ATOMICS=1
        -DSDL_ASSEMBLY_ROUTINES=1
        )
    endif()
    
    link_libraries(imm32 winmm version)

  elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    file(GLOB TMP
      ${SDL_SOURCES_DIR}/src/loadso/dlopen/*.c
      ${SDL_SOURCES_DIR}/src/render/opengl/*.c
      ${SDL_SOURCES_DIR}/src/render/opengles2/*.c
      ${SDL_SOURCES_DIR}/src/render/software/*.c
      ${SDL_SOURCES_DIR}/src/thread/pthread/*.c
      ${SDL_SOURCES_DIR}/src/timer/unix/*.c
      ${SDL_SOURCES_DIR}/src/video/cocoa/*.m
      )

    list(APPEND SDL_SOURCES ${TMP})

    add_definitions(
      -DSDL_LOADSO_DLOPEN=1
      -DSDL_THREAD_PTHREAD=1
      -DSDL_TIMER_UNIX=1
      -DSDL_POWER_DISABLED=1

      -DSDL_VIDEO_DRIVER_COCOA=1
      -DSDL_VIDEO_OPENGL=1
      -DSDL_VIDEO_OPENGL_CGL=1
      -DSDL_VIDEO_RENDER_OGL=1
      
      -DSDL_ASSEMBLY_ROUTINES=1
      -DSDL_THREAD_PTHREAD_RECURSIVE_MUTEX=1
      )

    find_library(CARBON_LIBRARY Carbon)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(QUARTZ_LIBRARY QuartzCore)
    link_libraries(${CARBON_LIBRARY} ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${QUARTZ_LIBRARY})

  endif()

else()
  pkg_search_module(SDL2 REQUIRED sdl2)
  include_directories(${SDL2_INCLUDE_DIRS})
  link_libraries(${SDL2_LIBRARIES})
endif()

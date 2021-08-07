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


# ./configure --disable-pdf --disable-svg --disable-xlib --disable-xcb --disable-script --disable-ps --disable-ft --disable-fc --disable-png --disable-trace --disable-interpreter


if (STATIC_BUILD OR NOT USE_SYSTEM_CAIRO)
  SET(CAIRO_SOURCES_DIR ${CMAKE_BINARY_DIR}/cairo-1.14.12)
  SET(CAIRO_URL "http://orthanc.osimis.io/ThirdPartyDownloads/cairo-1.14.12.tar.xz")
  SET(CAIRO_MD5 "9f0db9dbfca0966be8acd682e636d165")

  DownloadPackage(${CAIRO_MD5} ${CAIRO_URL} "${CAIRO_SOURCES_DIR}")

  file(COPY 
    ${CMAKE_CURRENT_LIST_DIR}/cairo-features.h
    DESTINATION ${CAIRO_SOURCES_DIR}/src
    )

  set(CAIRO_SOURCES
    ${CAIRO_SOURCES_DIR}/src/cairo-analysis-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-arc.c
    ${CAIRO_SOURCES_DIR}/src/cairo-array.c
    ${CAIRO_SOURCES_DIR}/src/cairo-atomic.c
    ${CAIRO_SOURCES_DIR}/src/cairo-base64-stream.c
    ${CAIRO_SOURCES_DIR}/src/cairo-base85-stream.c
    ${CAIRO_SOURCES_DIR}/src/cairo-bentley-ottmann.c
    ${CAIRO_SOURCES_DIR}/src/cairo-bentley-ottmann-rectangular.c
    ${CAIRO_SOURCES_DIR}/src/cairo-bentley-ottmann-rectilinear.c
    ${CAIRO_SOURCES_DIR}/src/cairo-botor-scan-converter.c
    ${CAIRO_SOURCES_DIR}/src/cairo-boxes.c
    ${CAIRO_SOURCES_DIR}/src/cairo-boxes-intersect.c
    ${CAIRO_SOURCES_DIR}/src/cairo.c
    ${CAIRO_SOURCES_DIR}/src/cairo-cache.c
    ${CAIRO_SOURCES_DIR}/src/cairo-cff-subset.c
    ${CAIRO_SOURCES_DIR}/src/cairo-clip-boxes.c
    ${CAIRO_SOURCES_DIR}/src/cairo-clip.c
    ${CAIRO_SOURCES_DIR}/src/cairo-clip-polygon.c
    ${CAIRO_SOURCES_DIR}/src/cairo-clip-region.c
    ${CAIRO_SOURCES_DIR}/src/cairo-clip-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-clip-tor-scan-converter.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-cogl-context.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-cogl-gradient.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-cogl-surface.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-cogl-utils.c
    ${CAIRO_SOURCES_DIR}/src/cairo-color.c
    ${CAIRO_SOURCES_DIR}/src/cairo-composite-rectangles.c
    ${CAIRO_SOURCES_DIR}/src/cairo-compositor.c
    ${CAIRO_SOURCES_DIR}/src/cairo-contour.c
    ${CAIRO_SOURCES_DIR}/src/cairo-damage.c
    ${CAIRO_SOURCES_DIR}/src/cairo-debug.c
    ${CAIRO_SOURCES_DIR}/src/cairo-default-context.c
    ${CAIRO_SOURCES_DIR}/src/cairo-deflate-stream.c
    ${CAIRO_SOURCES_DIR}/src/cairo-device.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-directfb-surface.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-egl-context.c
    ${CAIRO_SOURCES_DIR}/src/cairo-error.c
    ${CAIRO_SOURCES_DIR}/src/cairo-fallback-compositor.c
    ${CAIRO_SOURCES_DIR}/src/cairo-fixed.c
    ${CAIRO_SOURCES_DIR}/src/cairo-font-face.c
    ${CAIRO_SOURCES_DIR}/src/cairo-font-face-twin.c
    ${CAIRO_SOURCES_DIR}/src/cairo-font-face-twin-data.c
    ${CAIRO_SOURCES_DIR}/src/cairo-font-options.c
    ${CAIRO_SOURCES_DIR}/src/cairo-freed-pool.c
    ${CAIRO_SOURCES_DIR}/src/cairo-freelist.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-ft-font.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-composite.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-device.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-dispatch.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-glyphs.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-gradient.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-info.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-msaa-compositor.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-operand.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-shaders.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-source.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-spans-compositor.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-surface.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-gl-traps-compositor.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-glx-context.c
    ${CAIRO_SOURCES_DIR}/src/cairo-gstate.c
    ${CAIRO_SOURCES_DIR}/src/cairo-hash.c
    ${CAIRO_SOURCES_DIR}/src/cairo-hull.c
    ${CAIRO_SOURCES_DIR}/src/cairo-image-compositor.c
    ${CAIRO_SOURCES_DIR}/src/cairo-image-info.c
    ${CAIRO_SOURCES_DIR}/src/cairo-image-source.c
    ${CAIRO_SOURCES_DIR}/src/cairo-image-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-line.c
    ${CAIRO_SOURCES_DIR}/src/cairo-lzw.c
    ${CAIRO_SOURCES_DIR}/src/cairo-mask-compositor.c
    ${CAIRO_SOURCES_DIR}/src/cairo-matrix.c
    ${CAIRO_SOURCES_DIR}/src/cairo-mempool.c
    ${CAIRO_SOURCES_DIR}/src/cairo-mesh-pattern-rasterizer.c
    ${CAIRO_SOURCES_DIR}/src/cairo-misc.c
    ${CAIRO_SOURCES_DIR}/src/cairo-mono-scan-converter.c
    ${CAIRO_SOURCES_DIR}/src/cairo-mutex.c
    ${CAIRO_SOURCES_DIR}/src/cairo-no-compositor.c
    ${CAIRO_SOURCES_DIR}/src/cairo-observer.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-os2-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-output-stream.c
    ${CAIRO_SOURCES_DIR}/src/cairo-paginated-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-path-bounds.c
    ${CAIRO_SOURCES_DIR}/src/cairo-path.c
    ${CAIRO_SOURCES_DIR}/src/cairo-path-fill.c
    ${CAIRO_SOURCES_DIR}/src/cairo-path-fixed.c
    ${CAIRO_SOURCES_DIR}/src/cairo-path-in-fill.c
    ${CAIRO_SOURCES_DIR}/src/cairo-path-stroke-boxes.c
    ${CAIRO_SOURCES_DIR}/src/cairo-path-stroke.c
    ${CAIRO_SOURCES_DIR}/src/cairo-path-stroke-polygon.c
    ${CAIRO_SOURCES_DIR}/src/cairo-path-stroke-traps.c
    ${CAIRO_SOURCES_DIR}/src/cairo-path-stroke-tristrip.c
    ${CAIRO_SOURCES_DIR}/src/cairo-pattern.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-pdf-operators.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-pdf-shading.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-pdf-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-pen.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-png.c
    ${CAIRO_SOURCES_DIR}/src/cairo-polygon.c
    ${CAIRO_SOURCES_DIR}/src/cairo-polygon-intersect.c
    ${CAIRO_SOURCES_DIR}/src/cairo-polygon-reduce.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-ps-surface.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-quartz-font.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-quartz-image-surface.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-quartz-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-raster-source-pattern.c
    ${CAIRO_SOURCES_DIR}/src/cairo-recording-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-rectangle.c
    ${CAIRO_SOURCES_DIR}/src/cairo-rectangular-scan-converter.c
    ${CAIRO_SOURCES_DIR}/src/cairo-region.c
    ${CAIRO_SOURCES_DIR}/src/cairo-rtree.c
    ${CAIRO_SOURCES_DIR}/src/cairo-scaled-font.c
    ${CAIRO_SOURCES_DIR}/src/cairo-scaled-font-subsets.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-script-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-shape-mask-compositor.c
    ${CAIRO_SOURCES_DIR}/src/cairo-slope.c
    ${CAIRO_SOURCES_DIR}/src/cairo-spans.c
    ${CAIRO_SOURCES_DIR}/src/cairo-spans-compositor.c
    ${CAIRO_SOURCES_DIR}/src/cairo-spline.c
    ${CAIRO_SOURCES_DIR}/src/cairo-stroke-dash.c
    ${CAIRO_SOURCES_DIR}/src/cairo-stroke-style.c
    ${CAIRO_SOURCES_DIR}/src/cairo-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-surface-clipper.c
    ${CAIRO_SOURCES_DIR}/src/cairo-surface-fallback.c
    ${CAIRO_SOURCES_DIR}/src/cairo-surface-observer.c
    ${CAIRO_SOURCES_DIR}/src/cairo-surface-offset.c
    ${CAIRO_SOURCES_DIR}/src/cairo-surface-snapshot.c
    ${CAIRO_SOURCES_DIR}/src/cairo-surface-subsurface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-surface-wrapper.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-svg-surface.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-tee-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-time.c
    ${CAIRO_SOURCES_DIR}/src/cairo-tor22-scan-converter.c
    ${CAIRO_SOURCES_DIR}/src/cairo-tor-scan-converter.c
    ${CAIRO_SOURCES_DIR}/src/cairo-toy-font-face.c
    ${CAIRO_SOURCES_DIR}/src/cairo-traps.c
    ${CAIRO_SOURCES_DIR}/src/cairo-traps-compositor.c
    ${CAIRO_SOURCES_DIR}/src/cairo-tristrip.c
    ${CAIRO_SOURCES_DIR}/src/cairo-truetype-subset.c
    ${CAIRO_SOURCES_DIR}/src/cairo-type1-fallback.c
    ${CAIRO_SOURCES_DIR}/src/cairo-type1-glyph-names.c
    ${CAIRO_SOURCES_DIR}/src/cairo-type1-subset.c
    ${CAIRO_SOURCES_DIR}/src/cairo-type3-glyph-surface.c
    ${CAIRO_SOURCES_DIR}/src/cairo-unicode.c
    ${CAIRO_SOURCES_DIR}/src/cairo-user-font.c
    ${CAIRO_SOURCES_DIR}/src/cairo-version.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-vg-surface.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-wgl-context.c
    ${CAIRO_SOURCES_DIR}/src/cairo-wideint.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xcb-connection.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xcb-connection-core.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xcb-connection-render.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xcb-connection-shm.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xcb-resources.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xcb-screen.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xcb-shm.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xcb-surface.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xcb-surface-core.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xcb-surface-render.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xlib-core-compositor.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xlib-display.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xlib-fallback-compositor.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xlib-render-compositor.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xlib-screen.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xlib-source.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xlib-surface.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xlib-surface-shm.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xlib-visual.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xlib-xcb-surface.c
    # ${CAIRO_SOURCES_DIR}/src/cairo-xml-surface.c
    )

  include_directories(${CAIRO_SOURCES_DIR}/src)

  set(CAIRO_DEFINITIONS "HAS_PIXMAN_GLYPHS=1")

  if (${CMAKE_SYSTEM_NAME} STREQUAL "PNaCl")
    # Disable vectorized instructions when targeting archicture-independent PNaCl
    set(CAIRO_DEFINITIONS "${CAIRO_DEFINITIONS};HAVE_STDINT_H=1;CAIRO_HAS_PTHREAD=1;HAVE_UINT64_T=1")

  elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten" OR
          ${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    # Disable vectorized instructions and threading if targeting asm.js
    set(CAIRO_DEFINITIONS "${CAIRO_DEFINITIONS};HAVE_STDINT_H=1;CAIRO_HAS_PTHREAD=0;CAIRO_NO_MUTEX=1;HAVE_UINT64_T=1")

  elseif (CMAKE_COMPILER_IS_GNUCXX OR
      CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    set(CAIRO_DEFINITIONS "${CAIRO_DEFINITIONS};HAVE_STDINT_H=1;CAIRO_HAS_PTHREAD=1;HAVE_UINT64_T=1;CAIRO_HAS_REAL_PTHREAD=1;HAVE_GCC_VECTOR_EXTENSIONS;HAVE_FLOAT128")

    if (CMAKE_COMPILER_IS_GNUCXX)
      set_property(
        SOURCE ${CAIRO_SOURCES}
        PROPERTY COMPILE_FLAGS "-Wno-attributes"
        )
    endif()

  elseif (MSVC)
    # The cairo source code comes with built-in support for Visual Studio

  else()
    message(FATAL_ERROR "Support your platform here")

  endif()


  if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    # Explicitly request static building on Windows
    add_definitions(-DCAIRO_WIN32_STATIC_BUILD=1)
  endif()


  set_property(
    SOURCE ${CAIRO_SOURCES}
    PROPERTY COMPILE_DEFINITIONS "${CAIRO_DEFINITIONS}"
    )

else()

  pkg_search_module(CAIRO REQUIRED cairo)
  include_directories(${CAIRO_INCLUDE_DIRS})
  link_libraries(${CAIRO_LIBRARIES})
endif()

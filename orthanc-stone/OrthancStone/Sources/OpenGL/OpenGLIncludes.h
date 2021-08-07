/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 **/


#pragma once

#include "../OrthancStone.h"

#if !defined(ORTHANC_ENABLE_SDL)
#  error The macro ORTHANC_ENABLE_SDL must be defined
#endif

#if !defined(ORTHANC_ENABLE_WASM)
#  error The macro ORTHANC_ENABLE_WASM must be defined
#endif

#if !defined(ORTHANC_ENABLE_OPENGL)
#  error The macro ORTHANC_ENABLE_OPENGL must be defined
#endif

#if ORTHANC_ENABLE_OPENGL != 1
#  error Support for OpenGL is disabled
#endif

#if defined(__APPLE__)
#  include <OpenGL/gl.h>
#  include <OpenGL/glext.h>
#elif defined(QT_VERSION_MAJOR) && (QT_VERSION >= 5)
// Qt5 takes care of the inclusions
#elif defined(_WIN32)
// On Windows, use the compatibility headers provided by glew
#  include <GL/glew.h>
#else
#  include <GL/gl.h>
#  include <GL/glext.h>
#endif



#if ORTHANC_ENABLE_SDL == 1
#  include <SDL_video.h>

#  if !defined(NDEBUG)  // Is build type "debug"?
//   glGetError is very expensive!
#    include <Logging.h>
#    include <OrthancException.h>
#    define ORTHANC_OPENGL_CHECK(name)                                  \
  if(true)                                                              \
  {                                                                     \
    GLenum error = glGetError();                                        \
    if (error != GL_NO_ERROR) {                                         \
      SDL_GLContext ctx = SDL_GL_GetCurrentContext();                   \
      LOG(ERROR) << "Error when calling " << name << " | current context is: 0x" \
                 << std::hex << ctx <<  " | error code is " << error;   \
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,"OpenGL error in " name " | See log."); \
    }                                                                   \
  } else (void)0

#   define ORTHANC_OPENGL_TRACE_CURRENT_CONTEXT(msg)                    \
  if(true)                                                              \
  {                                                                     \
    SDL_GLContext ctx = SDL_GL_GetCurrentContext();                     \
    LOG(TRACE) << msg << " | Current OpenGL context is " << std::hex << ctx; \
  } else (void)0

#  endif /* NDEBUG */
#endif  /* SDL */



#if ORTHANC_ENABLE_WASM == 1
#  include <emscripten/html5.h>

#  if !defined(NDEBUG)  // Is build type "debug"?
#    include <Logging.h>
#    include <OrthancException.h>
#    define ORTHANC_OPENGL_CHECK(name)                                  \
  if(true)                                                              \
  {                                                                     \
    GLenum error = glGetError();                                        \
    if (error != GL_NO_ERROR) {                                         \
      EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context(); \
      EM_BOOL lost = emscripten_is_webgl_context_lost(ctx);             \
      LOG(ERROR) << "Error when calling " << name << " | current context is: 0x" \
                 << std::hex << ctx <<  " | error code is " << error << " | emscripten_is_webgl_context_lost = " << lost; \
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,"OpenGL error in " name " | See log."); \
    }                                                                   \
  } else (void)0

#    define ORTHANC_OPENGL_TRACE_CURRENT_CONTEXT(msg)                   \
  if(true)                                                              \
  {                                                                     \
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context(); \
    LOG(TRACE) << msg << " | Current OpenGL context is " << std::hex << ctx; \
  } else (void)0

#    define ORTHANC_CHECK_CURRENT_CONTEXT(context)                      \
  if(true)                                                              \
  {                                                                     \
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context(); \
    void* actualCtx = reinterpret_cast<void*>(ctx);                     \
    void* expectedCtx = context.DebugGetInternalContext();              \
    if(expectedCtx != actualCtx)                                        \
    {                                                                   \
      LOG(ERROR) << "Expected context was " << std::hex << expectedCtx  \
                 << " while actual context is " << std::hex << actualCtx; \
    }                                                                   \
  } else (void)0

#  endif /* NDEBUG */
#endif /* WASM */





// Define void implementation of debug macros if they were not defined above

#if !defined(ORTHANC_OPENGL_CHECK)
#  define ORTHANC_OPENGL_CHECK(name)
#endif

#if !defined(ORTHANC_OPENGL_TRACE_CURRENT_CONTEXT)
#  define ORTHANC_OPENGL_TRACE_CURRENT_CONTEXT(msg)
#endif

#if !defined(ORTHANC_CHECK_CURRENT_CONTEXT)
#  define ORTHANC_CHECK_CURRENT_CONTEXT(context)
#endif


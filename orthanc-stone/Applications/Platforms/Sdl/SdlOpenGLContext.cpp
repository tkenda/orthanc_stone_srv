/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#include "SdlOpenGLContext.h"
#include "../../../OrthancStone/Sources/StoneException.h"

#if ORTHANC_ENABLE_SDL == 1

#if !defined(ORTHANC_ENABLE_GLEW)
#  error Macro ORTHANC_ENABLE_GLEW must be defined
#endif

#if ORTHANC_ENABLE_GLEW == 1
#  include <GL/glew.h>
#endif

#include <Logging.h>
#include <OrthancException.h>

#include <boost/thread/mutex.hpp>

namespace OrthancStone
{
  SdlOpenGLContext::SdlOpenGLContext(const char* title,
                                     unsigned int width,
                                     unsigned int height,
                                     bool allowDpiScaling) 
    : window_(title, width, height, true /* enable OpenGL */, allowDpiScaling)
    , context_(NULL)
  {
    context_ = SDL_GL_CreateContext(window_.GetObject());
    
    if (context_ == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,
                                      "Cannot initialize OpenGL");
    }

#if ORTHANC_ENABLE_GLEW == 1
    // The initialization function of glew (i.e. "glewInit()") can
    // only be called once an OpenGL is setup.
    // https://stackoverflow.com/a/45033669/881731
    {
      static boost::mutex  mutex_;
      static bool          isGlewInitialized_ = false;
  
      boost::mutex::scoped_lock lock(mutex_);

      if (!isGlewInitialized_)
      {
        LOG(INFO) << "Initializing glew";
        
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
          LOG(ERROR) << glewGetErrorString(err);
          throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,
                                          "Cannot initialize glew");
        }

        isGlewInitialized_ = true;
      }
    }    
#endif
  }

  
  SdlOpenGLContext::~SdlOpenGLContext()
  {
    SDL_GL_DeleteContext(context_);
  }

  void SdlOpenGLContext::MakeCurrent()
  {
    if (SDL_GL_MakeCurrent(window_.GetObject(), context_) != 0)
    {
      const char* errText = SDL_GetError();
      std::stringstream ss;
      ss << "Cannot set current OpenGL context. SDL error text: " << errText;
      std::string errStr = ss.str();
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError, errStr.c_str());
    }

    // This makes our buffer swap synchronized with the monitor's vertical refresh
    SDL_GL_SetSwapInterval(1);
  }

  
  void SdlOpenGLContext::SwapBuffer()
  {
    // Swap our buffer to display the current contents of buffer on screen
    SDL_GL_SwapWindow(window_.GetObject());
  }

  
  unsigned int SdlOpenGLContext::GetCanvasWidth() const
  {
    int w = 0;
    SDL_GL_GetDrawableSize(window_.GetObject(), &w, NULL);
    return static_cast<unsigned int>(w);
  }

  
  unsigned int SdlOpenGLContext::GetCanvasHeight() const
  {
    int h = 0;
    SDL_GL_GetDrawableSize(window_.GetObject(), NULL, &h);
    return static_cast<unsigned int>(h);
  }
}

#endif

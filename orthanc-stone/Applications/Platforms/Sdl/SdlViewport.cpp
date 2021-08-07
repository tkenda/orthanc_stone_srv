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

#include "SdlViewport.h"

#include "../../../OrthancStone/Sources/Scene2DViewport/ViewportController.h"

#include <OrthancException.h>

#include <boost/make_shared.hpp>

namespace OrthancStone
{
  ICompositor& SdlViewport::SdlLock::GetCompositor()
  {
    if (that_.compositor_.get() == NULL)
    {
      // The derived class should have called "AcquireCompositor()"
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
    else
    {
      return *that_.compositor_;
    }
  }


  void SdlViewport::AcquireCompositor(ICompositor* compositor /* takes ownership */)
  {
    if (compositor == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }

    compositor_.reset(compositor);
  }


  SdlViewport::SdlViewport()
  {
    refreshEvent_ = SDL_RegisterEvents(1);

    if (refreshEvent_ == static_cast<uint32_t>(-1))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
  }
  
  void SdlViewport::PostConstructor()
  {
    controller_ = boost::make_shared<ViewportController>(shared_from_this());
  }
    
  void SdlViewport::SendRefreshEvent()
  {
    SDL_Event event;
    SDL_memset(&event, 0, sizeof(event));
    event.type = refreshEvent_;
    SDL_PushEvent(&event);  // This function is thread-safe, and can be called from other threads safely.
  }

  
  void SdlViewport::UpdateSize(unsigned int width, unsigned int height)
  {
    SdlLock lock(*this);
    lock.GetCompositor().SetCanvasSize(width, height);
    lock.Invalidate();
  }


#if ORTHANC_ENABLE_OPENGL == 1
  SdlOpenGLViewport::SdlOpenGLViewport(const std::string& title,
                                       unsigned int width,
                                       unsigned int height,
                                       bool allowDpiScaling) :
    context_(title.c_str(), width, height, allowDpiScaling)
  {
    AcquireCompositor(new OpenGLCompositor(context_));  // (*)
  }
#endif


#if ORTHANC_ENABLE_OPENGL == 1
  void SdlOpenGLViewport::RefreshCanvasSize()
  {
    UpdateSize(context_.GetCanvasWidth(), context_.GetCanvasHeight());
  }
#endif


#if ORTHANC_ENABLE_OPENGL == 1
  boost::shared_ptr<SdlOpenGLViewport> SdlOpenGLViewport::Create(const std::string& title,
                                                                 unsigned int width,
                                                                 unsigned int height,
                                                                 bool allowDpiScaling)
  {
    boost::shared_ptr<SdlOpenGLViewport> that =
      boost::shared_ptr<SdlOpenGLViewport>(new SdlOpenGLViewport(title, width, height, allowDpiScaling));
    that->SdlViewport::PostConstructor();
    return that;
  }
#endif


#if ORTHANC_ENABLE_OPENGL == 1
  uint32_t SdlOpenGLViewport::GetSdlWindowId()
  {
    const SdlWindow& sdlWindowWrapper = context_.GetWindow();
    SDL_Window* sdlWindow = sdlWindowWrapper.GetObject();
    Uint32 sdlWindowId = SDL_GetWindowID(sdlWindow);
    return sdlWindowId;
  }
#endif
  

#if ORTHANC_ENABLE_OPENGL == 1
  SdlOpenGLViewport::~SdlOpenGLViewport()
  {
    // Make sure that the "OpenGLCompositor" is destroyed BEFORE the
    // "OpenGLContext" it references (*)
    ClearCompositor();
  }
#endif


#if ORTHANC_ENABLE_OPENGL == 1
  void SdlOpenGLViewport::Paint()
  {
    SdlLock lock(*this);
    lock.GetCompositor().Refresh(lock.GetController().GetScene());
  }
#endif


#if ORTHANC_ENABLE_OPENGL == 1
  void SdlOpenGLViewport::ToggleMaximize()
  {
    // No need to call "Invalidate()" here, as "UpdateSize()" will
    // be invoked after event "SDL_WINDOWEVENT_SIZE_CHANGED"
    SdlLock lock(*this);
    context_.ToggleMaximize();
  }
#endif


  void SdlCairoViewport::RefreshCanvasSize()
  {
    UpdateSize(window_.GetWidth(), window_.GetHeight());
  }

  SdlCairoViewport::SdlCairoViewport(const std::string& title,
                                     unsigned int width,
                                     unsigned int height,
                                     bool allowDpiScaling) :
    window_(title, width, height, false /* enable OpenGL */, allowDpiScaling),
    sdlSurface_(NULL)
  {
    AcquireCompositor(new CairoCompositor(width, height));
  }

  SdlCairoViewport::~SdlCairoViewport()
  {
    if (sdlSurface_)
    {
      SDL_FreeSurface(sdlSurface_);
    }
  }
  
  uint32_t SdlCairoViewport::GetSdlWindowId()
  {
    SDL_Window* sdlWindow = window_.GetObject();
    Uint32 sdlWindowId = SDL_GetWindowID(sdlWindow);
    return sdlWindowId;
  }

  void SdlCairoViewport::Paint()
  {
    SdlLock lock(*this);

    lock.GetCompositor().Refresh(lock.GetController().GetScene());
    CreateSdlSurfaceFromCompositor(dynamic_cast<CairoCompositor&>(lock.GetCompositor()));
    
    if (sdlSurface_ != NULL)
    {
      window_.Render(sdlSurface_);
    }
  }


  void SdlCairoViewport::ToggleMaximize()
  {
    // No need to call "Invalidate()" here, as "UpdateSize()" will
    // be invoked after event "SDL_WINDOWEVENT_SIZE_CHANGED"
    SdlLock lock(*this);
    window_.ToggleMaximize();
  }

  
  // Assumes that the mutex is locked
  void SdlCairoViewport::CreateSdlSurfaceFromCompositor(const CairoCompositor& compositor)
  {
    static const uint32_t rmask = 0x00ff0000;
    static const uint32_t gmask = 0x0000ff00;
    static const uint32_t bmask = 0x000000ff;

    const unsigned int width = compositor.GetCanvas().GetWidth();
    const unsigned int height = compositor.GetCanvas().GetHeight();

    if (sdlSurface_ != NULL)
    {
      if (sdlSurface_->pixels == compositor.GetCanvas().GetBuffer() &&
          sdlSurface_->w == static_cast<int>(width) &&
          sdlSurface_->h == static_cast<int>(height) &&
          sdlSurface_->pitch == static_cast<int>(compositor.GetCanvas().GetPitch()))
      {
        // The image from the compositor has not changed, no need to update the surface
        return;
      }
      else
      {
        SDL_FreeSurface(sdlSurface_);
      }
    }

    sdlSurface_ = SDL_CreateRGBSurfaceFrom((void*)(compositor.GetCanvas().GetBuffer()), width, height, 32,
                                           compositor.GetCanvas().GetPitch(), rmask, gmask, bmask, 0);
    if (!sdlSurface_)
    {
      LOG(ERROR) << "Cannot create a SDL surface from a Cairo surface";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
  }


  boost::shared_ptr<SdlCairoViewport> SdlCairoViewport::Create(const std::string& title,
                                                               unsigned int width,
                                                               unsigned int height,
                                                               bool allowDpiScaling)
  {
    boost::shared_ptr<SdlCairoViewport> that =
      boost::shared_ptr<SdlCairoViewport>(new SdlCairoViewport(title, width, height, allowDpiScaling));
    that->SdlViewport::PostConstructor();
    return that;
  }
}

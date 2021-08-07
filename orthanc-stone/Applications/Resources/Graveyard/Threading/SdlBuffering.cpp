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


#include "SdlBuffering.h"

#if ORTHANC_ENABLE_SDL == 1

#include "../../Resources/Orthanc/Core/Logging.h"
#include "../../Resources/Orthanc/Core/OrthancException.h"

namespace OrthancStone
{
  SdlBuffering::SdlBuffering() :
    sdlSurface_(NULL),
    pendingFrame_(false)
  {
  }


  SdlBuffering::~SdlBuffering()
  {
    if (sdlSurface_)
    {
      SDL_FreeSurface(sdlSurface_);
    }
  }


  void SdlBuffering::SetSize(unsigned int width,
                             unsigned int height,
                             IViewport& viewport)
  {
    boost::mutex::scoped_lock lock(mutex_);

    viewport.SetSize(width, height);

    if (offscreenSurface_.get() == NULL ||
        offscreenSurface_->GetWidth() != width ||
        offscreenSurface_->GetHeight() != height)
    {
      offscreenSurface_.reset(new CairoSurface(width, height));
    }

    if (onscreenSurface_.get() == NULL ||
        onscreenSurface_->GetWidth() != width ||
        onscreenSurface_->GetHeight() != height)
    {
      onscreenSurface_.reset(new CairoSurface(width, height));

      // TODO Big endian?
      static const uint32_t rmask = 0x00ff0000;
      static const uint32_t gmask = 0x0000ff00;
      static const uint32_t bmask = 0x000000ff;

      if (sdlSurface_)
      {
        SDL_FreeSurface(sdlSurface_);
      }

      sdlSurface_ = SDL_CreateRGBSurfaceFrom(onscreenSurface_->GetBuffer(), width, height, 32,
                                             onscreenSurface_->GetPitch(), rmask, gmask, bmask, 0);
      if (!sdlSurface_)
      {
        LOG(ERROR) << "Cannot create a SDL surface from a Cairo surface";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }    
    }

    pendingFrame_ = false;
  }


  bool SdlBuffering::RenderOffscreen(IViewport& viewport)
  {
    boost::mutex::scoped_lock lock(mutex_);

    if (offscreenSurface_.get() == NULL)
    {
      return false;
    }

    Orthanc::ImageAccessor target = offscreenSurface_->GetAccessor();

    if (viewport.Render(target) &&
        !pendingFrame_)
    {
      pendingFrame_ = true;
      return true;
    }
    else
    {
      return false;
    }
  }


  void SdlBuffering::SwapToScreen(SdlWindow& window)
  {
    if (!pendingFrame_ ||
        offscreenSurface_.get() == NULL ||
        onscreenSurface_.get() == NULL)
    {
      return;
    }

    {
      boost::mutex::scoped_lock lock(mutex_);
      onscreenSurface_->Copy(*offscreenSurface_);
    }
    
    window.Render(sdlSurface_);
    pendingFrame_ = false;
  }
}

#endif

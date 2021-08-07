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


#pragma once

#if ORTHANC_ENABLE_SDL == 1

#include "SdlWindow.h"
#include "../../Framework/Viewport/CairoSurface.h"
#include "../../Framework/Viewport/IViewport.h"

#include <boost/thread/mutex.hpp>

namespace OrthancStone
{
  class SdlBuffering : public boost::noncopyable
  {
  private:
    boost::mutex                 mutex_;
    std::unique_ptr<CairoSurface>  offscreenSurface_;
    std::unique_ptr<CairoSurface>  onscreenSurface_;
    SDL_Surface*                 sdlSurface_;
    bool                         pendingFrame_;

  public:
    SdlBuffering();

    ~SdlBuffering();

    void SetSize(unsigned int width,
                 unsigned int height,
                 IViewport& viewport);

    // Returns "true" if a new refresh of the display should be
    // triggered afterwards
    bool RenderOffscreen(IViewport& viewport);

    void SwapToScreen(SdlWindow& window);
  };
}

#endif

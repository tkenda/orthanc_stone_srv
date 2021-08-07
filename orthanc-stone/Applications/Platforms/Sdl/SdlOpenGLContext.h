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

#include "../../../OrthancStone/Sources/OpenGL/IOpenGLContext.h"
#include "SdlWindow.h"

#include <Compatibility.h>  // For ORTHANC_OVERRIDE

#include <SDL_render.h>

#include <Enumerations.h>

namespace OrthancStone
{
  class SdlOpenGLContext : public OpenGL::IOpenGLContext
  {
  private:
    SdlWindow      window_;
    SDL_GLContext  context_;

  public:
    SdlOpenGLContext(const char* title,
                     unsigned int width,
                     unsigned int height,
                     bool allowDpiScaling = true);

    ~SdlOpenGLContext();

    SdlWindow& GetWindow()
    {
      return window_;
    }

    virtual bool IsContextLost() ORTHANC_OVERRIDE
    {
      // On desktop applications, an OpenGL context should never be lost
      return false;
    }

    virtual void MakeCurrent() ORTHANC_OVERRIDE;

    virtual void SwapBuffer() ORTHANC_OVERRIDE;

    unsigned int GetCanvasWidth() const;

    unsigned int GetCanvasHeight() const;

    void ToggleMaximize()
    {
      window_.ToggleMaximize();
    }
  };
}

#endif

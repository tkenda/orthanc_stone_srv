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

#include "Scene2D.h"
#include "ScenePoint2D.h"

#if ORTHANC_ENABLE_LOCALE == 1
#  include "../Fonts/TextBoundingBox.h"
#endif

namespace OrthancStone
{
  class ICompositor : public boost::noncopyable
  {
  public:
    virtual ~ICompositor() 
    {
    }

    virtual void SetCanvasSize(unsigned int canvasWidth,
                               unsigned int canvasHeight) = 0; 
    
    virtual unsigned int GetCanvasWidth() const = 0;

    virtual unsigned int GetCanvasHeight() const = 0;

    /**
     * WARNING: "Refresh()" must always be called with the same
     * scene. If the scene changes, a call to "ResetScene()" must be
     * done to reset the tracking of the revisions of the layers.
     **/
    virtual void Refresh(const Scene2D& scene) = 0;

    virtual void ResetScene() = 0;

#if ORTHANC_ENABLE_LOCALE == 1
    virtual void SetFont(size_t index,
                         const std::string& ttf,
                         unsigned int fontSize,
                         Orthanc::Encoding codepage) = 0;
#endif

    // Get the center of the given pixel, in canvas coordinates
    ScenePoint2D GetPixelCenterCoordinates(int x, int y) const
    {
      return ScenePoint2D(
        static_cast<double>(x) + 0.5 - static_cast<double>(GetCanvasWidth()) / 2.0,
        static_cast<double>(y) + 0.5 - static_cast<double>(GetCanvasHeight()) / 2.0);
    }

    void FitContent(Scene2D& scene) const
    {
      scene.FitContent(GetCanvasWidth(), GetCanvasHeight());
    }

#if ORTHANC_ENABLE_LOCALE == 1
    virtual TextBoundingBox* ComputeTextBoundingBox(size_t fontIndex,
                                                    const std::string& utf8) = 0;
#endif
  };
}

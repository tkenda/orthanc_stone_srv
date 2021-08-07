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

#include "Color.h"
#include "ScenePoint2D.h"
#include "ISceneLayer.h"

#include <Compatibility.h>  // For ORTHANC_OVERRIDE

namespace OrthancStone
{
  class ArrowSceneLayer : public ISceneLayer
  {
  private:
    ScenePoint2D  a_;
    ScenePoint2D  b_;
    Color         color_;
    double        thickness_;
    uint64_t      revision_;
    double        arrowLength_;   // in pixels
    double        arrowAngle_;    // in radians

  public:
    ArrowSceneLayer(const ScenePoint2D& a,
                    const ScenePoint2D& b);

    virtual uint64_t GetRevision() const ORTHANC_OVERRIDE
    {
      return revision_;
    }

    virtual ISceneLayer* Clone() const ORTHANC_OVERRIDE;

    const ScenePoint2D& GetA() const
    {
      return a_;
    }

    const ScenePoint2D& GetB() const
    {
      return b_;
    }

    void SetThickness(double thickness);

    double GetThickness() const
    {
      return thickness_;
    }

    void SetColor(const Color& color);
    
    void SetColor(uint8_t red,
                  uint8_t green,
                  uint8_t blue)
    {
      SetColor(Color(red, green, blue));
    }
    
    const Color& GetColor() const
    {
      return color_;
    }

    void SetArrowLength(double length);

    double GetArrowLength() const
    {
      return arrowLength_;
    }

    void SetArrowAngle(double angle);

    double GetArrowAngle() const
    {
      return arrowAngle_;
    }

    virtual Type GetType() const ORTHANC_OVERRIDE
    {
      return Type_Arrow;
    }

    virtual void GetBoundingBox(Extent2D& target) const ORTHANC_OVERRIDE;
  };
}

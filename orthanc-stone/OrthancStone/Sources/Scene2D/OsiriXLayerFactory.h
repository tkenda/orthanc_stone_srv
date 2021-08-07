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

#include "../Toolbox/CoordinateSystem3D.h"
#include "../Toolbox/OsiriX/Annotation.h"
#include "Color.h"
#include "ISceneLayer.h"

namespace OrthancStone
{
  class OsiriXLayerFactory : public boost::noncopyable
  {
  private:
    Color   color_;
    double  thickness_;
    double  arrowLength_;
    double  arrowAngle_;
    size_t  fontIndex_;
    
  public:
    OsiriXLayerFactory();
    
    void SetColor(const Color& color)
    {
      color_ = color;
    }

    void SetColor(uint8_t red,
                  uint8_t green,
                  uint8_t blue)
    {
      color_ = Color(red, green, blue);
    }

    const Color& GetColor() const
    {
      return color_;
    }

    void SetThickness(double thickness)
    {
      thickness_ = thickness;
    }

    double GetThickness() const
    {
      return thickness_;
    }

    void SetArrowLength(double arrowLength)
    {
      arrowLength_ = arrowLength;
    }

    double GetArrowLength() const
    {
      return arrowLength_;
    }

    void SetArrowAngle(double arrowAngle)
    {
      arrowAngle_ = arrowAngle;
    }

    double GetArrowAngle() const
    {
      return arrowAngle_;
    }

    void SetFontIndex(double fontIndex)
    {
      fontIndex_ = fontIndex;
    }

    double GetFontIndex() const
    {
      return fontIndex_;
    }

    ISceneLayer* Create(const OsiriX::Annotation& annotation,
                        const CoordinateSystem3D& plane) const;
  };
}

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

#include "ColorSceneLayer.h"
#include "../StoneEnumerations.h"

#include <memory>
#include <string>

namespace OrthancStone
{
  class TextSceneLayer : public ColorSceneLayer
  {
  private:
    double         x_;
    double         y_;
    std::string    utf8_;
    size_t         fontIndex_;
    BitmapAnchor   anchor_;
    unsigned int   border_;
    uint64_t       revision_;
  
  public:
    TextSceneLayer();

    virtual ISceneLayer* Clone() const ORTHANC_OVERRIDE;

    void SetPosition(double x,
                     double y);

    void SetText(const std::string& utf8);

    void SetFontIndex(size_t fontIndex);

    void SetAnchor(BitmapAnchor anchor);

    void SetBorder(unsigned int border);

    double GetX() const
    {
      return x_;
    }
    
    double GetY() const
    {
      return y_;
    }

    unsigned int GetBorder() const
    {
      return border_;
    }
  
    const std::string& GetText() const
    {
      return utf8_;
    }

    size_t GetFontIndex() const
    {
      return fontIndex_;
    }

    BitmapAnchor GetAnchor() const
    {
      return anchor_;
    }

    virtual Type GetType() const ORTHANC_OVERRIDE
    {
      return Type_Text;
    }

    virtual void GetBoundingBox(Extent2D& target) const ORTHANC_OVERRIDE
    {
      target.Clear();  // Not implemented
    }

    virtual uint64_t GetRevision() const ORTHANC_OVERRIDE
    {
      return revision_;
    }
  };
}

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

#include "ISceneLayer.h"
#include "Color.h"

#include <Compatibility.h>  // For ORTHANC_OVERRIDE

namespace OrthancStone
{
  // TODO - Is this needed?
  class ColorSceneLayer : public ISceneLayer
  {
  private:
    Color    color_;
    uint64_t revision_;

  protected:
    void BumpRevision()
    {
      // this is *not* thread-safe!!!  => (SJO) no problem, Stone assumes mono-threading
      revision_++;
    }

  public:
    ColorSceneLayer() :
      revision_(0)
    {
    }

    virtual uint64_t GetRevision() const ORTHANC_OVERRIDE
    {
      return revision_;
    }

    void SetColor(uint8_t red,
                  uint8_t green,
                  uint8_t blue)
    {
      color_ = Color(red, green, blue);
      BumpRevision();
    }

    void SetColor(const Color& color)
    {
      color_ = color;
      BumpRevision();
    }

    const Color& GetColor() const
    {
      return color_;
    }
  };
}

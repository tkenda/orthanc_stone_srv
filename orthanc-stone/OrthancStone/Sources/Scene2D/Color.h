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

#include <stdint.h>

namespace OrthancStone
{
  class Color
  {
  private:
    uint8_t  red_;
    uint8_t  green_;
    uint8_t  blue_;

  public:
    Color() :
      red_(255),
      green_(255),
      blue_(255)
    {
    }

    Color(uint8_t red,
          uint8_t green,
          uint8_t blue) :
      red_(red),
      green_(green),
      blue_(blue)
    {
    }

    uint8_t GetRed() const
    {
      return red_;
    }

    uint8_t GetGreen() const
    {
      return green_;
    }

    uint8_t GetBlue() const
    {
      return blue_;
    }

    float GetRedAsFloat() const
    {
      return static_cast<float>(red_) / 255.0f;
    }

    float GetGreenAsFloat() const
    {
      return static_cast<float>(green_) / 255.0f;
    }

    float GetBlueAsFloat() const
    {
      return static_cast<float>(blue_) / 255.0f;
    }
  };
}

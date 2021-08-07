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

// PixelTestPatterns.h

#pragma once

#include "../StoneException.h"

#include <Images/ImageAccessor.h>

#include <string>
#include <stdint.h>
#include <math.h>

namespace OrthancStone
{
  namespace PixelTestPatterns
  {
    template<typename T, typename U>
    inline uint8_t byteAddClip(T v1, U v2)
    {
      double tmp = static_cast<double>(v1) + static_cast<double>(v2);
      if (tmp > 255.0)
        tmp = 255;
      if (tmp < 0.0)
        tmp = 0;
      return static_cast<uint8_t>(tmp+0.5);
    }

    // fills the area with a horizontal gradient.
    // leftmost pixels are filled with r0 g0 b0
    // rightmost pixels are filled with r1 g1 b1
    // linear interpolation in-between
    inline void fillWithHGradient(Orthanc::ImageAccessor& target,
      uint8_t r0, uint8_t g0, uint8_t b0,
      uint8_t r1, uint8_t g1, uint8_t b1)
    {
      if (target.GetFormat() != Orthanc::PixelFormat_RGBA32) {
        ORTHANC_ASSERT(false, "Wrong pixel format");
      }
      const unsigned int width = target.GetWidth();
      const unsigned int height = target.GetHeight();

      ORTHANC_ASSERT(width > 0);
      ORTHANC_ASSERT(height > 0);

      double invWidth = 1.0 / static_cast<double>(target.GetWidth());
      double rIncr = (static_cast<double>(r1) - static_cast<double>(r0))* invWidth;
      double gIncr = (static_cast<double>(g1) - static_cast<double>(g0))* invWidth;
      double bIncr = (static_cast<double>(b1) - static_cast<double>(b0))* invWidth;

      for (unsigned int y = 0; y < height; y++)
      {
        uint8_t r = r0;
        uint8_t g = g0;
        uint8_t b = b0;
        uint8_t* q = reinterpret_cast<uint8_t*>(target.GetRow(y));
        for (unsigned int x = 0; x < width; x++)
        {
          q[0] = r;
          q[1] = g;
          q[2] = b;
          q[3] = 255;
          r = byteAddClip(r, rIncr);
          g = byteAddClip(g, gIncr);
          b = byteAddClip(b, bIncr);
          q += 4;
        }
      }
    }

    inline void fillWithVGradient(Orthanc::ImageAccessor& target,
      uint8_t r0, uint8_t g0, uint8_t b0,
      uint8_t r1, uint8_t g1, uint8_t b1)
    {
      if (target.GetFormat() != Orthanc::PixelFormat_RGBA32) {
        ORTHANC_ASSERT(false, "Wrong pixel format");
      }
      const unsigned int width = target.GetWidth();
      const unsigned int height = target.GetHeight();

      ORTHANC_ASSERT(width > 0);
      ORTHANC_ASSERT(height > 0);

      double invHeight = 1.0 / static_cast<double>(target.GetHeight());
      double rIncr = (static_cast<double>(r1) - static_cast<double>(r0))* invHeight;
      double gIncr = (static_cast<double>(g1) - static_cast<double>(g0))* invHeight;
      double bIncr = (static_cast<double>(b1) - static_cast<double>(b0))* invHeight;

      uint8_t r = r0;
      uint8_t g = g0;
      uint8_t b = b0;
      for (unsigned int y = 0; y < height; y++)
      {
        uint8_t* q = reinterpret_cast<uint8_t*>(target.GetRow(y));
        for (unsigned int x = 0; x < width; x++)
        {
          q[0] = r;
          q[1] = g;
          q[2] = b;
          q[3] = 255;
          q += 4;
        }
        r = byteAddClip(r, rIncr);
        g = byteAddClip(g, gIncr);
        b = byteAddClip(b, bIncr);
      }
    }

  }
}


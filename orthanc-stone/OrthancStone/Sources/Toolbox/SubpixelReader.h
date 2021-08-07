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

#include "../StoneEnumerations.h"
#include "GeometryToolbox.h"

#include <Images/ImageTraits.h>

#include <boost/noncopyable.hpp>
#include <cmath>

namespace OrthancStone
{
  namespace Internals
  {
    class SubpixelReaderBase : public boost::noncopyable
    {
    private:
      const Orthanc::ImageAccessor& source_;
      unsigned int                  width_;
      unsigned int                  height_;

    public:
      explicit SubpixelReaderBase(const Orthanc::ImageAccessor& source) :
        source_(source),
        width_(source.GetWidth()),
        height_(source.GetHeight())
      {
      }

      ORTHANC_FORCE_INLINE
      const Orthanc::ImageAccessor& GetSource() const
      {
        return source_;
      }
      
      ORTHANC_FORCE_INLINE
      unsigned int GetWidth() const
      {
        return width_;
      }
      
      ORTHANC_FORCE_INLINE
      unsigned int GetHeight() const
      {
        return height_;
      }
    };
  }

    
  template <Orthanc::PixelFormat Format,
            ImageInterpolation Interpolation>
  class SubpixelReader;

    
  template <Orthanc::PixelFormat Format>
  class SubpixelReader<Format, ImageInterpolation_Nearest> : 
    public Internals::SubpixelReaderBase
  {
  public:
    typedef Orthanc::PixelTraits<Format>  Traits;
    typedef typename Traits::PixelType    PixelType;

    explicit SubpixelReader(const Orthanc::ImageAccessor& source) :
      SubpixelReaderBase(source)
    {
    }

    inline bool GetValue(PixelType& target,
                         float x,
                         float y) const;

    inline bool GetFloatValue(float& target,
                              float x,
                              float y) const;
  };

    
    
  template <Orthanc::PixelFormat Format>
  class SubpixelReader<Format, ImageInterpolation_Bilinear> : 
    public Internals::SubpixelReaderBase
  {
  public:
    typedef Orthanc::PixelTraits<Format>  Traits;
    typedef typename Traits::PixelType    PixelType;

    explicit SubpixelReader(const Orthanc::ImageAccessor& source) :
      SubpixelReaderBase(source)
    {
    }

    inline bool GetFloatValue(float& target,
                              float x,
                              float y) const;

    inline bool GetValue(PixelType& target,
                         float x,
                         float y) const;
  };



  template <Orthanc::PixelFormat Format>
  bool SubpixelReader<Format, ImageInterpolation_Nearest>::GetValue(PixelType& target,
                                                                    float x,
                                                                    float y) const
  {
    if (x < 0 ||
        y < 0)
    {
      return false;
    }
    else
    {
      unsigned int ux = static_cast<unsigned int>(std::floor(x));
      unsigned int uy = static_cast<unsigned int>(std::floor(y));

      if (ux < GetWidth() &&
          uy < GetHeight())
      {
        Orthanc::ImageTraits<Format>::GetPixel(target, GetSource(), ux, uy);
        return true;
      }
      else
      {
        return false;
      }
    }
  }



  template <Orthanc::PixelFormat Format>
  bool SubpixelReader<Format, ImageInterpolation_Nearest>::GetFloatValue(float& target,
                                                                         float x,
                                                                         float y) const
  {
    PixelType value;
    
    if (GetValue(value, x, y))
    {
      target = Traits::PixelToFloat(value);
      return true;
    }
    else
    {
      return false;
    }
  }



  template <Orthanc::PixelFormat Format>
  bool SubpixelReader<Format, ImageInterpolation_Bilinear>::GetValue(PixelType& target,
                                                                     float x,
                                                                     float y) const
  {
    float value;

    if (GetFloatValue(value, x, y))
    {
      Traits::FloatToPixel(target, value);
      return true;
    }
    else
    {
      return false;
    }
  }



  template <Orthanc::PixelFormat Format>
  bool SubpixelReader<Format, ImageInterpolation_Bilinear>::GetFloatValue(float& target,
                                                                          float x,
                                                                          float y) const
  {
    x -= 0.5f;
    y -= 0.5f;
        
    if (x < 0 ||
        y < 0)
    {
      return false;
    }
    else
    {
      unsigned int ux = static_cast<unsigned int>(std::floor(x));
      unsigned int uy = static_cast<unsigned int>(std::floor(y));

      float f00, f01, f10, f11;

      if (ux < GetWidth() &&
          uy < GetHeight())
      {
        f00 = Orthanc::ImageTraits<Format>::GetFloatPixel(GetSource(), ux, uy);
      }
      else
      {
        return false;
      }

      if (ux + 1 < GetWidth())
      {
        f01 = Orthanc::ImageTraits<Format>::GetFloatPixel(GetSource(), ux + 1, uy);
      }
      else
      {
        f01 = f00;
      }

      if (uy + 1 < GetHeight())
      {
        f10 = Orthanc::ImageTraits<Format>::GetFloatPixel(GetSource(), ux, uy + 1);
      }
      else
      {
        f10 = f00;
      }

      if (ux + 1 < GetWidth() &&
          uy + 1 < GetHeight())
      {
        f11 = Orthanc::ImageTraits<Format>::GetFloatPixel(GetSource(), ux + 1, uy + 1);
      }
      else
      {
        f11 = f00;
      }

      float ax = x - static_cast<float>(ux);
      float ay = y - static_cast<float>(uy);
      target = GeometryToolbox::ComputeBilinearInterpolationUnitSquare(ax, ay, f00, f01, f10, f11);
          
      return true;
    }
  }
}

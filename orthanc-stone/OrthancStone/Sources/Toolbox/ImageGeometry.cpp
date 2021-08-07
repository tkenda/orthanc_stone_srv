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


#include "ImageGeometry.h"

#include "Extent2D.h"
#include "SubpixelReader.h"

#include <Images/ImageProcessing.h>
#include <Logging.h>
#include <OrthancException.h>


namespace OrthancStone
{
  static void AddTransformedPoint(Extent2D& extent,
                                  const Matrix& a,
                                  double x,
                                  double y)
  {
    assert(a.size1() == 3 &&
           a.size2() == 3);

    Vector p = LinearAlgebra::Product(a, LinearAlgebra::CreateVector(x, y, 1));

    if (!LinearAlgebra::IsCloseToZero(p[2]))
    {
      extent.AddPoint(p[0] / p[2], p[1] / p[2]);
    }
  }
  
  
  bool GetProjectiveTransformExtent(unsigned int& x1,
                                    unsigned int& y1,
                                    unsigned int& x2,
                                    unsigned int& y2,
                                    const Matrix& a,
                                    unsigned int sourceWidth,
                                    unsigned int sourceHeight,
                                    unsigned int targetWidth,
                                    unsigned int targetHeight)
  {
    if (targetWidth == 0 ||
        targetHeight == 0)
    {
      return false;
    }
    
    Extent2D extent;
    AddTransformedPoint(extent, a, 0, 0);
    AddTransformedPoint(extent, a, sourceWidth, 0);
    AddTransformedPoint(extent, a, 0, sourceHeight);
    AddTransformedPoint(extent, a, sourceWidth, sourceHeight);

    if (extent.IsEmpty())
    {
      return false;
    }
    else
    {
      int tmp = static_cast<int>(std::floor(extent.GetX1()));
      if (tmp < 0)
      {
        x1 = 0;
      }
      else
      {
        x1 = static_cast<unsigned int>(tmp);
      }

      tmp = static_cast<int>(std::floor(extent.GetY1()));
      if (tmp < 0)
      {
        y1 = 0;
      }
      else
      {
        y1 = static_cast<unsigned int>(tmp);
      }

      tmp = static_cast<int>(std::ceil(extent.GetX2()));
      if (tmp < 0)
      {
        return false;
      }
      else if (static_cast<unsigned int>(tmp) >= targetWidth)
      {
        x2 = targetWidth - 1;
      }
      else
      {
        x2 = static_cast<unsigned int>(tmp);
      }

      tmp = static_cast<int>(std::ceil(extent.GetY2()));
      if (tmp < 0)
      {
        return false;
      }
      else if (static_cast<unsigned int>(tmp) >= targetHeight)
      {
        y2 = targetHeight - 1;
      }
      else
      {
        y2 = static_cast<unsigned int>(tmp);
      }

      return (x1 <= x2 &&
              y1 <= y2);
    }
  }


  template <typename Reader,
            bool HasOffsetX,
            bool HasOffsetY>
  static void ApplyAffineTransformToRow(typename Reader::PixelType* p,
                                        Reader& reader,
                                        unsigned int x1,
                                        unsigned int x2,
                                        float positionX,
                                        float positionY,
                                        float offsetX,
                                        float offsetY)
  {
    typename Reader::PixelType value;

    for (unsigned int x = x1; x <= x2; x++, p++)
    {     
      if (reader.GetValue(value, positionX, positionY))
      {
        *p = value;
      }

      if (HasOffsetX)
      {
        positionX += offsetX;
      }

      if (HasOffsetY)
      {
        positionY += offsetY;
      }
    }
  }
  

  template <Orthanc::PixelFormat Format,
            ImageInterpolation Interpolation>
  static void ApplyAffineInternal(Orthanc::ImageAccessor& target,
                                  const Orthanc::ImageAccessor& source,
                                  const Matrix& a,
                                  bool clear)
  {
    assert(target.GetFormat() == Format &&
           source.GetFormat() == Format);
    
    typedef SubpixelReader<Format, Interpolation>  Reader;
    typedef typename Reader::PixelType             PixelType;

    if (clear)
    {
      if (Format == Orthanc::PixelFormat_RGB24)
      {
        Orthanc::ImageProcessing::Set(target, 0, 0, 0, 255);
      }
      else
      {
        Orthanc::ImageProcessing::Set(target, 0);
      }
    }

    Matrix inva;
    if (!LinearAlgebra::InvertMatrixUnsafe(inva, a))
    {
      // Singular matrix
      return;
    }

    Reader reader(source);

    unsigned int x1, y1, x2, y2;

    if (GetProjectiveTransformExtent(x1, y1, x2, y2, a,
                                     source.GetWidth(), source.GetHeight(),
                                     target.GetWidth(), target.GetHeight()))
    {
      const size_t targetPitch = target.GetPitch();
      uint8_t *targetRow = reinterpret_cast<uint8_t*>
        (reinterpret_cast<PixelType*>(target.GetRow(y1)) + x1);

      for (unsigned int y = y1; y <= y2; y++)
      {
        Vector start;
        LinearAlgebra::AssignVector(start, static_cast<double>(x1) + 0.5,
                                    static_cast<double>(y) + 0.5, 1);
        start = boost::numeric::ublas::prod(inva, start);
        assert(LinearAlgebra::IsNear(1.0, start(2)));

        Vector offset;
        LinearAlgebra::AssignVector(offset, static_cast<double>(x1) + 1.5,
                                    static_cast<double>(y) + 0.5, 1);
        offset = boost::numeric::ublas::prod(inva, offset) - start;
        assert(LinearAlgebra::IsNear(0.0, offset(2)));

        float startX = static_cast<float>(start[0]);
        float startY = static_cast<float>(start[1]);
        float offsetX = static_cast<float>(offset[0]);
        float offsetY = static_cast<float>(offset[1]);

        PixelType* pixel = reinterpret_cast<PixelType*>(targetRow);
        if (LinearAlgebra::IsCloseToZero(offsetX))
        {
          ApplyAffineTransformToRow<Reader, false, true>
            (pixel, reader, x1, x2, startX, startY, offsetX, offsetY);
        }
        else if (LinearAlgebra::IsCloseToZero(offsetY))
        {
          ApplyAffineTransformToRow<Reader, true, false>
            (pixel, reader, x1, x2, startX, startY, offsetX, offsetY);
        }
        else
        {
          ApplyAffineTransformToRow<Reader, true, true>
            (pixel, reader, x1, x2, startX, startY, offsetX, offsetY);
        }

        targetRow += targetPitch;
      }
    }    
  }


  void ApplyAffineTransform(Orthanc::ImageAccessor& target,
                            const Orthanc::ImageAccessor& source,
                            double a11,
                            double a12,
                            double b1,
                            double a21,
                            double a22,
                            double b2,
                            ImageInterpolation interpolation,
                            bool clear)
  {
    if (source.GetFormat() != target.GetFormat())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }

    if (interpolation != ImageInterpolation_Nearest &&
        interpolation != ImageInterpolation_Bilinear)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    Matrix a;
    a.resize(3, 3);
    a(0, 0) = a11;
    a(0, 1) = a12;
    a(0, 2) = b1;
    a(1, 0) = a21;
    a(1, 1) = a22;
    a(1, 2) = b2;
    a(2, 0) = 0;
    a(2, 1) = 0;
    a(2, 2) = 1;

    switch (source.GetFormat())
    {
      case Orthanc::PixelFormat_Grayscale8:
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ApplyAffineInternal<Orthanc::PixelFormat_Grayscale8, 
                                ImageInterpolation_Nearest>(target, source, a, clear);
            break;

          case ImageInterpolation_Bilinear:
            ApplyAffineInternal<Orthanc::PixelFormat_Grayscale8, 
                                ImageInterpolation_Bilinear>(target, source, a, clear);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
        break;

      case Orthanc::PixelFormat_Grayscale16:
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ApplyAffineInternal<Orthanc::PixelFormat_Grayscale16, 
                                ImageInterpolation_Nearest>(target, source, a, clear);
            break;

          case ImageInterpolation_Bilinear:
            ApplyAffineInternal<Orthanc::PixelFormat_Grayscale16, 
                                ImageInterpolation_Bilinear>(target, source, a, clear);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
        break;

      case Orthanc::PixelFormat_SignedGrayscale16:
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ApplyAffineInternal<Orthanc::PixelFormat_SignedGrayscale16, 
                                ImageInterpolation_Nearest>(target, source, a, clear);
            break;

          case ImageInterpolation_Bilinear:
            ApplyAffineInternal<Orthanc::PixelFormat_SignedGrayscale16, 
                                ImageInterpolation_Bilinear>(target, source, a, clear);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
        break;

      case Orthanc::PixelFormat_Float32:
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ApplyAffineInternal<Orthanc::PixelFormat_Float32, 
                                ImageInterpolation_Nearest>(target, source, a, clear);
            break;

          case ImageInterpolation_Bilinear:
            ApplyAffineInternal<Orthanc::PixelFormat_Float32, 
                                ImageInterpolation_Bilinear>(target, source, a, clear);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
        break;

      case Orthanc::PixelFormat_RGB24:
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ApplyAffineInternal<Orthanc::PixelFormat_RGB24, 
                                ImageInterpolation_Nearest>(target, source, a, clear);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }
  }


  template <Orthanc::PixelFormat Format,
            ImageInterpolation Interpolation>
  static void ApplyProjectiveInternal(Orthanc::ImageAccessor& target,
                                      const Orthanc::ImageAccessor& source,
                                      const Matrix& a,
                                      const Matrix& inva)
  {
    assert(target.GetFormat() == Format &&
           source.GetFormat() == Format);
    
    typedef SubpixelReader<Format, Interpolation> Reader;
    typedef typename Reader::PixelType            PixelType;

    Reader reader(source);
    unsigned int x1, y1, x2, y2;

    const float floatWidth = static_cast<float>(source.GetWidth());
    const float floatHeight = static_cast<float>(source.GetHeight());

    if (GetProjectiveTransformExtent(x1, y1, x2, y2, a,
                                     source.GetWidth(), source.GetHeight(),
                                     target.GetWidth(), target.GetHeight()))
    {
      const size_t targetPitch = target.GetPitch();
      uint8_t *targetRow = reinterpret_cast<uint8_t*>
        (reinterpret_cast<PixelType*>(target.GetRow(y1)) + x1);

      for (unsigned int y = y1; y <= y2; y++)
      {
        PixelType *p = reinterpret_cast<PixelType*>(targetRow);

        for (unsigned int x = x1; x <= x2; x++)
        {
          Vector v;
          LinearAlgebra::AssignVector(v, static_cast<double>(x) + 0.5, 
                                      static_cast<double>(y) + 0.5, 1);

          Vector vv = LinearAlgebra::Product(inva, v);

          assert(!LinearAlgebra::IsCloseToZero(vv[2]));
          const double w = 1.0 / vv[2];
          const float sourceX = static_cast<float>(vv[0] * w);
          const float sourceY = static_cast<float>(vv[1] * w);
          
          // Make sure no integer overflow will occur after truncation
          // (the static_cast<unsigned int> could otherwise throw an
          // exception in WebAssembly if strong projective effects)
          if (sourceX < floatWidth &&
              sourceY < floatHeight)
          { 
            reader.GetValue(*p, sourceX, sourceY);
          }

          p++;
        }

        targetRow += targetPitch;
      }
    }
  }

    
  void ApplyProjectiveTransform(Orthanc::ImageAccessor& target,
                                const Orthanc::ImageAccessor& source,
                                const Matrix& a,
                                ImageInterpolation interpolation,
                                bool clear)
  {
    if (source.GetFormat() != target.GetFormat())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }

    if (a.size1() != 3 ||
        a.size2() != 3)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageSize);
    }

    if (interpolation != ImageInterpolation_Nearest &&
        interpolation != ImageInterpolation_Bilinear)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    // Check whether we are dealing with an affine transform
    if (LinearAlgebra::IsCloseToZero(a(2, 0)) &&
        LinearAlgebra::IsCloseToZero(a(2, 1)))
    {
      double w = a(2, 2);
      if (LinearAlgebra::IsCloseToZero(w))
      {
        LOG(ERROR) << "Singular projective matrix";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }
      else
      {
        ApplyAffineTransform(target, source, 
                             a(0, 0) / w, a(0, 1) / w, a(0, 2) / w,
                             a(1, 0) / w, a(1, 1) / w, a(1, 2) / w,
                             interpolation, clear);
        return;
      }
    }

    if (clear)
    {
      if (target.GetFormat() == Orthanc::PixelFormat_RGB24)
      {
        Orthanc::ImageProcessing::Set(target, 0, 0, 0, 255);
      }
      else
      {
        Orthanc::ImageProcessing::Set(target, 0);
      }
    }

    Matrix inva;
    if (!LinearAlgebra::InvertMatrixUnsafe(inva, a))
    {
      return;
    }

    switch (source.GetFormat())
    {
      case Orthanc::PixelFormat_Grayscale8:
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ApplyProjectiveInternal<Orthanc::PixelFormat_Grayscale8, 
                                    ImageInterpolation_Nearest>(target, source, a, inva);
            break;

          case ImageInterpolation_Bilinear:
            ApplyProjectiveInternal<Orthanc::PixelFormat_Grayscale8, 
                                    ImageInterpolation_Bilinear>(target, source, a, inva);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
        break;

      case Orthanc::PixelFormat_Grayscale16:
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ApplyProjectiveInternal<Orthanc::PixelFormat_Grayscale16, 
                                    ImageInterpolation_Nearest>(target, source, a, inva);
            break;

          case ImageInterpolation_Bilinear:
            ApplyProjectiveInternal<Orthanc::PixelFormat_Grayscale16, 
                                    ImageInterpolation_Bilinear>(target, source, a, inva);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
        break;

      case Orthanc::PixelFormat_SignedGrayscale16:
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ApplyProjectiveInternal<Orthanc::PixelFormat_SignedGrayscale16, 
                                    ImageInterpolation_Nearest>(target, source, a, inva);
            break;

          case ImageInterpolation_Bilinear:
            ApplyProjectiveInternal<Orthanc::PixelFormat_SignedGrayscale16, 
                                    ImageInterpolation_Bilinear>(target, source, a, inva);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
        break;

      case Orthanc::PixelFormat_Float32:
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ApplyProjectiveInternal<Orthanc::PixelFormat_Float32, 
                                    ImageInterpolation_Nearest>(target, source, a, inva);
            break;

          case ImageInterpolation_Bilinear:
            ApplyProjectiveInternal<Orthanc::PixelFormat_Float32, 
                                    ImageInterpolation_Bilinear>(target, source, a, inva);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
        break;

      case Orthanc::PixelFormat_RGB24:
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ApplyProjectiveInternal<Orthanc::PixelFormat_RGB24, 
                                    ImageInterpolation_Nearest>(target, source, a, inva);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }
  }
}

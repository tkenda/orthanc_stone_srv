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


#include "VolumeReslicer.h"

#include "../Toolbox/GeometryToolbox.h"
#include "../Toolbox/SubvoxelReader.h"

#include <Images/ImageTraits.h>
#include <Logging.h>
#include <OrthancException.h>

#include <boost/math/special_functions/round.hpp>

namespace OrthancStone
{
  // Anonymous namespace to avoid clashes between compilation modules
  namespace
  {
    enum TransferFunction
    {
      TransferFunction_Copy,
      TransferFunction_Float,
      TransferFunction_Linear
    };


    template <Orthanc::PixelFormat InputFormat,
              Orthanc::PixelFormat OutputFormat,
              ImageInterpolation Interpolation,
              TransferFunction Function>
    class PixelShader;

    
    template <Orthanc::PixelFormat Format>
    class PixelShader<Format, 
                      Format, 
                      ImageInterpolation_Nearest, 
                      TransferFunction_Copy>
    {
    private:
      typedef SubvoxelReader<Format, ImageInterpolation_Nearest>  VoxelReader;
      typedef Orthanc::PixelTraits<Format>                        PixelWriter;

      VoxelReader  reader_;
      
    public:
      PixelShader(const ImageBuffer3D& image,
                  float /* scaling */,
                  float /* offset */) :
        reader_(image)
      {
      }
      
      ORTHANC_FORCE_INLINE
      void Apply(typename PixelWriter::PixelType* pixel,
                 float volumeX,
                 float volumeY,
                 float volumeZ)
      {
        typename VoxelReader::PixelType value;

        if (!reader_.GetValue(value, volumeX, volumeY, volumeZ))
        {
          VoxelReader::Traits::SetMinValue(value);
        }

        *pixel = value;
      }        
    };

    
    template <Orthanc::PixelFormat InputFormat,
              Orthanc::PixelFormat OutputFormat>
    class PixelShader<InputFormat, 
                      OutputFormat, 
                      ImageInterpolation_Nearest, 
                      TransferFunction_Copy>
    {
    private:
      typedef SubvoxelReader<InputFormat, ImageInterpolation_Nearest>  VoxelReader;
      typedef Orthanc::PixelTraits<OutputFormat>                       PixelWriter;

      VoxelReader  reader_;
      
    public:
      PixelShader(const ImageBuffer3D& image,
                  float /* scaling */,
                  float /* offset */) :
        reader_(image)
      {
      }
      
      ORTHANC_FORCE_INLINE
      void Apply(typename PixelWriter::PixelType* pixel,
                 float volumeX,
                 float volumeY,
                 float volumeZ)
      {
        typename VoxelReader::PixelType value;

        if (!reader_.GetValue(value, volumeX, volumeY, volumeZ))
        {
          VoxelReader::Traits::SetMinValue(value);
        }

        PixelWriter::FloatToPixel(*pixel, VoxelReader::Traits::PixelToFloat(value));
      }        
    };

    
    template <Orthanc::PixelFormat InputFormat,
              Orthanc::PixelFormat OutputFormat,
              ImageInterpolation Interpolation>
    class PixelShader<InputFormat,
                      OutputFormat,
                      Interpolation,
                      TransferFunction_Float>
    {
    private:
      typedef SubvoxelReader<InputFormat, Interpolation>  VoxelReader;
      typedef Orthanc::PixelTraits<OutputFormat>          PixelWriter;

      VoxelReader  reader_;
      float        outOfVolume_;
      
    public:
      PixelShader(const ImageBuffer3D& image,
                  float /* scaling */,
                  float /* offset */) :
        reader_(image),
        outOfVolume_(static_cast<float>(std::numeric_limits<typename VoxelReader::PixelType>::min()))
      {
      }
      
      ORTHANC_FORCE_INLINE
      void Apply(typename PixelWriter::PixelType* pixel,
                 float volumeX,
                 float volumeY,
                 float volumeZ)
      {
        float value;

        if (!reader_.GetFloatValue(value, volumeX, volumeY, volumeZ))
        {
          value = outOfVolume_;
        }

        PixelWriter::FloatToPixel(*pixel, value);
      }
    };

    
   template <Orthanc::PixelFormat InputFormat,
             Orthanc::PixelFormat OutputFormat,
             ImageInterpolation Interpolation>
    class PixelShader<InputFormat,
                      OutputFormat,
                      Interpolation,
                      TransferFunction_Linear>
    {
    private:
      typedef SubvoxelReader<InputFormat, Interpolation>  VoxelReader;
      typedef Orthanc::PixelTraits<OutputFormat>          PixelWriter;

      VoxelReader  reader_;
      float        scaling_;
      float        offset_;
      float        outOfVolume_;
      
    public:
      PixelShader(const ImageBuffer3D& image,
                  float scaling,
                  float offset) :
        reader_(image),
        scaling_(scaling),
        offset_(offset),
        outOfVolume_(static_cast<float>(std::numeric_limits<typename VoxelReader::PixelType>::min()))
      {
      }
      
      ORTHANC_FORCE_INLINE
      void Apply(typename PixelWriter::PixelType* pixel,
                 float volumeX,
                 float volumeY,
                 float volumeZ)
      {
        float value;

        if (reader_.GetFloatValue(value, volumeX, volumeY, volumeZ))
        {
          value = scaling_ * value + offset_;
        }
        else
        {
          value = outOfVolume_;
        }

        PixelWriter::FloatToPixel(*pixel, value);
      }        
    };



    class FastRowIterator : public boost::noncopyable
    {
    private:
      float  position_[3];
      float  offset_[3];
      
    public:
      FastRowIterator(const Orthanc::ImageAccessor& slice,
                      const Extent2D& extent,
                      const CoordinateSystem3D& plane,
                      const OrientedVolumeBoundingBox& box,
                      unsigned int y)
      {
        const double width = static_cast<double>(slice.GetWidth());
        const double height = static_cast<double>(slice.GetHeight());
        assert(y < height);

        Vector q1 = plane.MapSliceToWorldCoordinates
          (extent.GetX1() + extent.GetWidth() * 0.5 / width,
           extent.GetY1() + extent.GetHeight() * (static_cast<double>(y) + 0.5) / height);

        Vector q2 = plane.MapSliceToWorldCoordinates
          (extent.GetX1() + extent.GetWidth() * (static_cast<double>(width - 1) + 0.5) / width,
           extent.GetY1() + extent.GetHeight() * (static_cast<double>(y) + 0.5) / height);

        Vector r1, r2;
        box.ToInternalCoordinates(r1, q1);
        box.ToInternalCoordinates(r2, q2);

        position_[0] = static_cast<float>(r1[0]);
        position_[1] = static_cast<float>(r1[1]);
        position_[2] = static_cast<float>(r1[2]);
        
        Vector tmp = (r2 - r1) / static_cast<double>(width - 1);
        offset_[0] = static_cast<float>(tmp[0]);
        offset_[1] = static_cast<float>(tmp[1]);
        offset_[2] = static_cast<float>(tmp[2]);
      }

      ORTHANC_FORCE_INLINE
      void Next()
      {
        position_[0] += offset_[0];
        position_[1] += offset_[1];
        position_[2] += offset_[2];
      }

      ORTHANC_FORCE_INLINE
      void GetVolumeCoordinates(float& x,
                                float& y,
                                float& z) const
      {
        x = position_[0];
        y = position_[1];
        z = position_[2];
      }
    };


    class SlowRowIterator : public boost::noncopyable
    {
    private:
      const Orthanc::ImageAccessor&  slice_;
      const Extent2D&                extent_;
      const CoordinateSystem3D&      plane_;
      const OrientedVolumeBoundingBox&     box_;
      unsigned int                   x_;
      unsigned int                   y_;
      
    public:
      SlowRowIterator(const Orthanc::ImageAccessor& slice,
                      const Extent2D& extent,
                      const CoordinateSystem3D& plane,
                      const OrientedVolumeBoundingBox& box,
                      unsigned int y) :
        slice_(slice),
        extent_(extent),
        plane_(plane),
        box_(box),
        x_(0),
        y_(y)
      {
        assert(y_ < slice_.GetHeight());
      }

      void Next()
      {
        x_++;
      }

      void GetVolumeCoordinates(float& x,
                                float& y,
                                float& z) const
      {
        assert(x_ < slice_.GetWidth());
        
        const double width = static_cast<double>(slice_.GetWidth());
        const double height = static_cast<double>(slice_.GetHeight());
        
        Vector q = plane_.MapSliceToWorldCoordinates
          (extent_.GetX1() + extent_.GetWidth() * (static_cast<double>(x_) + 0.5) / width,
           extent_.GetY1() + extent_.GetHeight() * (static_cast<double>(y_) + 0.5) / height);

        Vector r;
        box_.ToInternalCoordinates(r, q);

        x = static_cast<float>(r[0]);
        y = static_cast<float>(r[1]);
        z = static_cast<float>(r[2]);
      }
    };


    template <typename RowIterator,
              Orthanc::PixelFormat InputFormat,
              Orthanc::PixelFormat OutputFormat,
              ImageInterpolation Interpolation,
              TransferFunction Function>
    static void ProcessImage(Orthanc::ImageAccessor& slice,
                             const Extent2D& extent,
                             const ImageBuffer3D& source,
                             const CoordinateSystem3D& plane,
                             const OrientedVolumeBoundingBox& box,
                             float scaling,
                             float offset)
    {
      typedef PixelShader<InputFormat, OutputFormat, Interpolation, Function>   Shader;

      const unsigned int outputWidth = slice.GetWidth();
      const unsigned int outputHeight = slice.GetHeight();

      const float sourceWidth = static_cast<float>(source.GetWidth());
      const float sourceHeight = static_cast<float>(source.GetHeight());
      const float sourceDepth = static_cast<float>(source.GetDepth());

      Shader shader(source, scaling, offset);

      for (unsigned int y = 0; y < outputHeight; y++)
      {
        typedef typename Orthanc::ImageTraits<OutputFormat>::PixelType PixelType;
        PixelType* p = reinterpret_cast<PixelType*>(slice.GetRow(y));

        RowIterator it(slice, extent, plane, box, y);

        for (unsigned int x = 0; x < outputWidth; x++, p++)
        {
          float volumeX, volumeY, volumeZ;
          it.GetVolumeCoordinates(volumeX, volumeY, volumeZ);

          shader.Apply(p, 
                       volumeX * sourceWidth, 
                       volumeY * sourceHeight, 
                       volumeZ * sourceDepth);
          it.Next();
        }
      }
    }


    template <typename RowIterator,
              Orthanc::PixelFormat InputFormat,
              Orthanc::PixelFormat OutputFormat>
    static void ProcessImage(Orthanc::ImageAccessor& slice,
                             const Extent2D& extent,
                             const ImageBuffer3D& source,
                             const CoordinateSystem3D& plane,
                             const OrientedVolumeBoundingBox& box,
                             ImageInterpolation interpolation,
                             bool hasLinearFunction,
                             float scaling,
                             float offset)
    {
      if (hasLinearFunction)
      {
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ProcessImage<RowIterator, InputFormat, OutputFormat,
                         ImageInterpolation_Nearest, TransferFunction_Linear>
              (slice, extent, source, plane, box, scaling, offset);
            break;

          case ImageInterpolation_Bilinear:
            ProcessImage<RowIterator, InputFormat, OutputFormat,
                         ImageInterpolation_Bilinear, TransferFunction_Linear>
              (slice, extent, source, plane, box, scaling, offset);
            break;

          case ImageInterpolation_Trilinear:
            ProcessImage<RowIterator, InputFormat, OutputFormat,
                         ImageInterpolation_Trilinear, TransferFunction_Linear>
              (slice, extent, source, plane, box, scaling, offset);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }        
      }
      else
      {
        switch (interpolation)
        {
          case ImageInterpolation_Nearest:
            ProcessImage<RowIterator, InputFormat, OutputFormat,
                         ImageInterpolation_Nearest, TransferFunction_Copy>
              (slice, extent, source, plane, box, 0, 0);
            break;

          case ImageInterpolation_Bilinear:
            ProcessImage<RowIterator, InputFormat, OutputFormat,
                         ImageInterpolation_Bilinear, TransferFunction_Float>
              (slice, extent, source, plane, box, 0, 0);
            break;

          case ImageInterpolation_Trilinear:
            ProcessImage<RowIterator, InputFormat, OutputFormat,
                         ImageInterpolation_Trilinear, TransferFunction_Float>
              (slice, extent, source, plane, box, 0, 0);
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }        
      }
    }
    
    
    template <typename RowIterator>
    static void ProcessImage(Orthanc::ImageAccessor& slice,
                             const Extent2D& extent,
                             const ImageBuffer3D& source,
                             const CoordinateSystem3D& plane,
                             const OrientedVolumeBoundingBox& box,
                             ImageInterpolation interpolation,
                             bool hasLinearFunction,
                             float scaling,
                             float offset)
    {
      if (source.GetFormat() == Orthanc::PixelFormat_Grayscale8 &&
          slice.GetFormat() == Orthanc::PixelFormat_Grayscale8)
      {
        ProcessImage<RowIterator,
                     Orthanc::PixelFormat_Grayscale8,
                     Orthanc::PixelFormat_Grayscale8>
          (slice, extent, source, plane, box, interpolation, hasLinearFunction, scaling, offset);
      }
      else if (source.GetFormat() == Orthanc::PixelFormat_Grayscale16 &&
               slice.GetFormat() == Orthanc::PixelFormat_Grayscale8)
      {
        ProcessImage<RowIterator,
                     Orthanc::PixelFormat_Grayscale16,
                     Orthanc::PixelFormat_Grayscale8>
          (slice, extent, source, plane, box, interpolation, hasLinearFunction, scaling, offset);
      }
      else if (source.GetFormat() == Orthanc::PixelFormat_Grayscale16 &&
               slice.GetFormat() == Orthanc::PixelFormat_Grayscale16)
      {
        ProcessImage<RowIterator,
                     Orthanc::PixelFormat_Grayscale16,
                     Orthanc::PixelFormat_Grayscale16>
          (slice, extent, source, plane, box, interpolation, hasLinearFunction, scaling, offset);
      }
      else if (source.GetFormat() == Orthanc::PixelFormat_SignedGrayscale16 &&
               slice.GetFormat() == Orthanc::PixelFormat_BGRA32)
      {
        ProcessImage<RowIterator,
                     Orthanc::PixelFormat_SignedGrayscale16,
                     Orthanc::PixelFormat_BGRA32>
          (slice, extent, source, plane, box, interpolation, hasLinearFunction, scaling, offset);
      }
      else if (source.GetFormat() == Orthanc::PixelFormat_Grayscale16 &&
               slice.GetFormat() == Orthanc::PixelFormat_BGRA32)
      {
        ProcessImage<RowIterator,
                     Orthanc::PixelFormat_Grayscale16,
                     Orthanc::PixelFormat_BGRA32>
          (slice, extent, source, plane, box, interpolation, hasLinearFunction, scaling, offset);
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
      }
    }
  }
    
    

  void VolumeReslicer::CheckIterators(const ImageBuffer3D& source,
                                      const CoordinateSystem3D& plane,
                                      const OrientedVolumeBoundingBox& box) const
  {
    const unsigned int width = slice_->GetWidth();
    const unsigned int height = slice_->GetHeight();
    
    for (unsigned int y = 0; y < height; y++)
    {
      FastRowIterator fast(*slice_, extent_, plane, box, y);
      SlowRowIterator slow(*slice_, extent_, plane, box, y);

      for (unsigned int x = 0; x < width; x++)
      {
        float px, py, pz;
        fast.GetVolumeCoordinates(px, py, pz);

        float qx, qy, qz;
        slow.GetVolumeCoordinates(qx, qy, qz);

        Vector d;
        LinearAlgebra::AssignVector(d, px - qx, py - qy, pz - qz);
        double norm = boost::numeric::ublas::norm_2(d);
        if (norm > 0.0001)
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
        }
          
        fast.Next();
        slow.Next();
      }
    }
  }

    
  void VolumeReslicer::Reset()
  {
    success_ = false;
    extent_.Clear();
    slice_.reset(NULL);
  }


  float VolumeReslicer::GetMinOutputValue() const
  {
    switch (outputFormat_)
    {
      case Orthanc::PixelFormat_Grayscale8:
      case Orthanc::PixelFormat_Grayscale16:
      case Orthanc::PixelFormat_BGRA32:
        return 0.0f;
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }
  }

    
  float VolumeReslicer::GetMaxOutputValue() const
  {
    switch (outputFormat_)
    {
      case Orthanc::PixelFormat_Grayscale8:
      case Orthanc::PixelFormat_BGRA32:
        return static_cast<float>(std::numeric_limits<uint8_t>::max());
        break;

      case Orthanc::PixelFormat_Grayscale16:
        return static_cast<float>(std::numeric_limits<uint16_t>::max());
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }
  }
    

  VolumeReslicer::VolumeReslicer() :
    outputFormat_(Orthanc::PixelFormat_Grayscale8),
    interpolation_(ImageInterpolation_Nearest),
    fastMode_(true),
    success_(false)
  {
    ResetLinearFunction();
  }


  void VolumeReslicer::GetLinearFunction(float& scaling,
                                         float& offset) const
  {
    if (hasLinearFunction_)
    {
      scaling = scaling_;
      offset = offset_;
    }
    else
    {
      scaling = 1.0f;
      offset = 0.0f;
    }
  }

  
  void VolumeReslicer::ResetLinearFunction()
  {
    Reset();
    hasLinearFunction_ = false;
    scaling_ = 1.0f;
    offset_ = 0.0f;
  }
    

  void VolumeReslicer::SetLinearFunction(float scaling,
                                         float offset)
  {
    Reset();
    hasLinearFunction_ = true;
    scaling_ = scaling;
    offset_ = offset;
  }

  
  void VolumeReslicer::SetWindow(float low,
                                 float high)
  {
    //printf("Range in pixel values: %f->%f\n", low, high);
    float scaling = (GetMaxOutputValue() - GetMinOutputValue()) / (high - low);
    float offset = GetMinOutputValue() - scaling * low;
    
    SetLinearFunction(scaling, offset);

    /*float x = scaling_ * low + offset_;
    float y = scaling_ * high + offset_;
    printf("%f %f (should be %f->%f)\n", x, y, GetMinOutputValue(), GetMaxOutputValue());*/
  }
  

  void VolumeReslicer::FitRange(const ImageBuffer3D& image)
  {
    float minInputValue, maxInputValue;

    if (!image.GetRange(minInputValue, maxInputValue) ||
        maxInputValue < 1)
    {
      ResetLinearFunction();
    }
    else
    {
      SetWindow(minInputValue, maxInputValue);
    }
  }  

  
  void VolumeReslicer::SetWindowing(ImageWindowing windowing,
                                    const ImageBuffer3D& image,
                                    float rescaleSlope,
                                    float rescaleIntercept)
  {
    if (windowing == ImageWindowing_Custom)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    float center, width;
    ComputeWindowing(center, width, windowing, 0, 0);

    float a = (center - width / 2.0f - rescaleIntercept) / rescaleSlope;
    float b = (center + width / 2.0f - rescaleIntercept) / rescaleSlope;
    SetWindow(a, b);
  }


  void VolumeReslicer::SetOutputFormat(Orthanc::PixelFormat format)
  {
    if (format != Orthanc::PixelFormat_Grayscale8 &&
        format != Orthanc::PixelFormat_Grayscale16 &&
        format != Orthanc::PixelFormat_BGRA32)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    if (hasLinearFunction_)
    {
      LOG(WARNING) << "Calls to VolumeReslicer::SetOutputFormat() should be done before VolumeReslicer::FitRange()";
    }
    
    outputFormat_ = format;
    Reset();
  }


  void VolumeReslicer::SetInterpolation(ImageInterpolation interpolation)
  {
    if (interpolation != ImageInterpolation_Nearest &&
        interpolation != ImageInterpolation_Bilinear &&
        interpolation != ImageInterpolation_Trilinear)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    interpolation_ = interpolation;
    Reset();
  }


  const Extent2D& VolumeReslicer::GetOutputExtent() const
  {
    if (success_)
    {
      return extent_;
    }
    else
    {
      LOG(ERROR) << "VolumeReslicer::GetOutputExtent(): (!success_)";

      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }

  
  const Orthanc::ImageAccessor& VolumeReslicer::GetOutputSlice() const
  {
    if (success_)
    {
      assert(slice_.get() != NULL);
      return *slice_;
    }
    else
    {
      LOG(ERROR) << "VolumeReslicer::GetOutputSlice(): (!success_)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }

  
  Orthanc::ImageAccessor* VolumeReslicer::ReleaseOutputSlice()
  {
    if (success_)
    {
      assert(slice_.get() != NULL);
      success_ = false;
      return slice_.release();
    }
    else
    {
      LOG(ERROR) << "VolumeReslicer::ReleaseOutputSlice(): (!success_)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }

  
  void VolumeReslicer::Apply(const ImageBuffer3D& source,
                             const VolumeImageGeometry& geometry,
                             const CoordinateSystem3D& plane)
  {
    // Choose the default voxel size as the finest voxel dimension
    // of the source volumetric image
    const Vector dim = geometry.GetVoxelDimensions(VolumeProjection_Axial);
    double voxelSize = dim[0];
    
    if (dim[1] < voxelSize)
    {
      voxelSize = dim[1];
    }
    
    if (dim[2] < voxelSize)
    {
      voxelSize = dim[2];
    }

    if (voxelSize <= 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    Apply(source, geometry, plane, voxelSize);
  }

  
  void VolumeReslicer::Apply(const ImageBuffer3D& source,
                             const VolumeImageGeometry& geometry,
                             const CoordinateSystem3D& plane,
                             double voxelSize)
  {
    Reset();
    pixelSpacing_ = voxelSize;

    // Firstly, compute the intersection of the source volumetric
    // image with the reslicing plane. This leads to a polygon with 3
    // to 6 vertices. We compute the extent of the intersection
    // polygon, with respect to the coordinate system of the reslicing
    // plane.
    OrientedVolumeBoundingBox box(geometry);

    if (!box.ComputeExtent(extent_, plane))
    {
      // The plane does not intersect with the bounding box of the volume
      slice_.reset(new Orthanc::Image(outputFormat_, 0, 0, false));
      success_ = true;
      return;
    }

    // Secondly, the extent together with the voxel size gives the
    // size of the output image
    unsigned int width = boost::math::iround(extent_.GetWidth() / voxelSize);
    unsigned int height = boost::math::iround(extent_.GetHeight() / voxelSize);

    slice_.reset(new Orthanc::Image(outputFormat_, width, height, false));

    //CheckIterators(source, plane, box);
      
    if (fastMode_)
    {
      ProcessImage<FastRowIterator>(*slice_, extent_, source, plane, box,
                                    interpolation_, hasLinearFunction_, scaling_, offset_);
    }
    else
    {
      ProcessImage<SlowRowIterator>(*slice_, extent_, source, plane, box,
                                    interpolation_, hasLinearFunction_, scaling_, offset_);
    }

    success_ = true;
  }


  double VolumeReslicer::GetPixelSpacing() const
  {
    if (success_)
    {
      return pixelSpacing_;
    }
    else
    {
      LOG(ERROR) << "VolumeReslicer::GetPixelSpacing(): (!success_)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }
}

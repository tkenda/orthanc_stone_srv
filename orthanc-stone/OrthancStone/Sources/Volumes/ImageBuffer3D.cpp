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


#include "ImageBuffer3D.h"

#include <Images/ImageProcessing.h>
#include <Logging.h>
#include <OrthancException.h>

#include <string.h>

namespace OrthancStone
{
  void ImageBuffer3D::GetAxialSliceAccessor(Orthanc::ImageAccessor& target,
                                            unsigned int slice,
                                            bool readOnly)
  {
    if (slice >= depth_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    if (readOnly)
    {
      target.AssignReadOnly(format_, width_, height_, image_.GetPitch(),
                            image_.GetConstRow(height_ * (depth_ - 1 - slice)));
    }
    else
    {
      target.AssignWritable(format_, width_, height_, image_.GetPitch(),
                            image_.GetRow(height_ * (depth_ - 1 - slice)));
    }
  }


  void ImageBuffer3D::GetCoronalSliceAccessor(Orthanc::ImageAccessor& target,
                                              unsigned int slice,
                                              bool readOnly)
  {
    if (slice >= height_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    if (readOnly)
    {
      target.AssignReadOnly(format_, width_, depth_, image_.GetPitch() * height_,
                            image_.GetConstRow(slice));
    }
    else
    {
      target.AssignWritable(format_, width_, depth_, image_.GetPitch() * height_,
                            image_.GetRow(slice));
    }
  }


  Orthanc::Image*  ImageBuffer3D::ExtractSagittalSlice(unsigned int slice) const
  {
    if (slice >= width_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    
    std::unique_ptr<Orthanc::Image> result(new Orthanc::Image(format_, height_, depth_, false));

    const unsigned int bytesPerPixel = Orthanc::GetBytesPerPixel(format_);

    for (unsigned int z = 0; z < depth_; z++)
    {
      uint8_t* q = reinterpret_cast<uint8_t*>(result->GetRow(z));

      for (unsigned int y = 0; y < height_; y++)
      {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(image_.GetConstRow(y + z * height_)) + bytesPerPixel * slice;
        
        for (size_t x = 0; x < bytesPerPixel; ++x)
        {
          q[x] = p[x];
        }
        
        q += bytesPerPixel;
      }
    }

    return result.release();
  }


  void ImageBuffer3D::CommitSagittalSlice(unsigned int slice,
                                          const Orthanc::ImageAccessor& source)
  {
    if (slice >= width_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    
    const unsigned int bytesPerPixel = Orthanc::GetBytesPerPixel(format_);

    for (unsigned int z = 0; z < depth_; z++)
    {
      const uint8_t* p = reinterpret_cast<const uint8_t*>(source.GetConstRow(z));

      for (unsigned int y = 0; y < height_; y++)
      {
        uint8_t* q = reinterpret_cast<uint8_t*>(image_.GetRow(y + z * height_)) + bytesPerPixel * slice;

        for (size_t x = 0; x < bytesPerPixel; ++x)
        {
          q[x] = p[x];
        }
        
        p += bytesPerPixel;
      }
    }
  }    


  ImageBuffer3D::ImageBuffer3D(Orthanc::PixelFormat format,
                               unsigned int width,
                               unsigned int height,
                               unsigned int depth,
                               bool computeRange) :
    image_(format, width, height * depth, false),
    format_(format),
    width_(width),
    height_(height),
    depth_(depth),
    computeRange_(computeRange),
    hasRange_(false)
  {
    LOG(TRACE) << "Created a 3D image of size " << width << "x" << height
              << "x" << depth << " in " << Orthanc::EnumerationToString(format)
              << " (" << (GetEstimatedMemorySize() / (1024ll * 1024ll)) << "MB)";
  }


  void ImageBuffer3D::Clear()
  {
    memset(image_.GetBuffer(), 0, image_.GetHeight() * image_.GetPitch());
  }


  uint64_t ImageBuffer3D::GetEstimatedMemorySize() const
  {
    return image_.GetPitch() * image_.GetHeight() * Orthanc::GetBytesPerPixel(format_);
  }


  void ImageBuffer3D::ExtendImageRange(const Orthanc::ImageAccessor& slice)
  {
    if (!computeRange_ ||
        slice.GetWidth() == 0 ||
        slice.GetHeight() == 0)
    {
      return;
    }

    float sliceMin, sliceMax;

    switch (slice.GetFormat())
    {
      case Orthanc::PixelFormat_Grayscale8:
      case Orthanc::PixelFormat_Grayscale16:
      case Orthanc::PixelFormat_Grayscale32:
      case Orthanc::PixelFormat_SignedGrayscale16:
      {
        int64_t a, b;
        Orthanc::ImageProcessing::GetMinMaxIntegerValue(a, b, slice);
        sliceMin = static_cast<float>(a);
        sliceMax = static_cast<float>(b);
        break;
      }

      case Orthanc::PixelFormat_Float32:
        Orthanc::ImageProcessing::GetMinMaxFloatValue(sliceMin, sliceMax, slice);
        break;

      default:
        return;
    }

    if (hasRange_)
    {
      minValue_ = std::min(minValue_, sliceMin);
      maxValue_ = std::max(maxValue_, sliceMax);
    }
    else
    {
      hasRange_ = true;
      minValue_ = sliceMin;
      maxValue_ = sliceMax;
    }
  }


  bool ImageBuffer3D::GetRange(float& minValue,
                               float& maxValue) const
  {
    if (hasRange_)
    {
      minValue = minValue_;
      maxValue = maxValue_;
      return true;
    }
    else
    {
      return false;
    }
  }


  ImageBuffer3D::SliceReader::SliceReader(const ImageBuffer3D& that,
                                          VolumeProjection projection,
                                          unsigned int slice)
  {
    /**
     * NB: The "const_cast" below are OK, as the "readonly" argument
     * to "GetAxialSliceAccessor()" and "GetCoronalSliceAccessor()"
     * are set to "true", which implies read-only access.
     **/
    
    switch (projection)
    {
      case VolumeProjection_Axial:
        const_cast<ImageBuffer3D&>(that).GetAxialSliceAccessor(accessor_, slice, true);
        break;

      case VolumeProjection_Coronal:
        const_cast<ImageBuffer3D&>(that).GetCoronalSliceAccessor(accessor_, slice, true);
        break;

      case VolumeProjection_Sagittal:
        sagittal_.reset(that.ExtractSagittalSlice(slice));
        sagittal_->GetReadOnlyAccessor(accessor_);
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  void ImageBuffer3D::SliceWriter::Flush()
  {
    if (modified_)
    {
      if (sagittal_.get() != NULL)
      {
        assert(sagittal_->GetWidth() == that_.height_ &&
               sagittal_->GetHeight() == that_.depth_ &&
               sagittal_->GetFormat() == that_.format_);

        that_.CommitSagittalSlice(slice_, *sagittal_);
      }

      // Update the dynamic range of the underlying image, if
      // "computeRange_" is set to true
      that_.ExtendImageRange(accessor_);
    }
  }


  ImageBuffer3D::SliceWriter::SliceWriter(ImageBuffer3D& that,
                                          VolumeProjection projection,
                                          unsigned int slice) :
    that_(that),
    modified_(false),
    slice_(slice)
  {
    switch (projection)
    {
      case VolumeProjection_Axial:
        that.GetAxialSliceAccessor(accessor_, slice, false);
        break;

      case VolumeProjection_Coronal:
        that.GetCoronalSliceAccessor(accessor_, slice, false);
        break;

      case VolumeProjection_Sagittal:
        sagittal_.reset(that.ExtractSagittalSlice(slice));
        sagittal_->GetWriteableAccessor(accessor_);
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  uint8_t ImageBuffer3D::GetVoxelGrayscale8(unsigned int x,
                                            unsigned int y,
                                            unsigned int z) const
  {
    if (format_ != Orthanc::PixelFormat_Grayscale8)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }

    if (x >= width_ ||
        y >= height_ ||
        z >= depth_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    const void* p = image_.GetConstRow(y + height_ * (depth_ - 1 - z));
    return reinterpret_cast<const uint8_t*>(p) [x];
  }


  uint16_t ImageBuffer3D::GetVoxelGrayscale16(unsigned int x,
                                              unsigned int y,
                                              unsigned int z) const
  {
    if (format_ != Orthanc::PixelFormat_Grayscale16)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }

    if (x >= width_ ||
        y >= height_ ||
        z >= depth_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    const void* p = image_.GetConstRow(y + height_ * (depth_ - 1 - z));
    return reinterpret_cast<const uint16_t*>(p) [x];
  }
}

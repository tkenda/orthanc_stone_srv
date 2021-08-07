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

#include "../Toolbox/Extent2D.h"
#include "OrientedVolumeBoundingBox.h"
#include "ImageBuffer3D.h"

namespace OrthancStone
{
  // Hypothesis: The output voxels always have square size
  class VolumeReslicer : public boost::noncopyable
  {
  private:
    // Input parameters
    Orthanc::PixelFormat           outputFormat_;
    bool                           hasLinearFunction_;
    float                          scaling_;  // "a" in "f(x) = a * x + b"
    float                          offset_;   // "b" in "f(x) = a * x + b"
    ImageInterpolation             interpolation_;
    bool                           fastMode_;

    // Output of reslicing
    bool                           success_;
    Extent2D                       extent_;
    std::unique_ptr<Orthanc::Image>  slice_;
    double                         pixelSpacing_;

    void CheckIterators(const ImageBuffer3D& source,
                        const CoordinateSystem3D& plane,
                        const OrientedVolumeBoundingBox& box) const;

    void Reset();

    float GetMinOutputValue() const;

    float GetMaxOutputValue() const;

    void SetWindow(float low,
                   float high);
    
  public:
    VolumeReslicer();

    void GetLinearFunction(float& scaling,
                           float& offset) const;

    void ResetLinearFunction();
    
    void SetLinearFunction(float scaling,
                           float offset);

    void FitRange(const ImageBuffer3D& image);

    void SetWindowing(ImageWindowing windowing,
                      const ImageBuffer3D& image,
                      float rescaleSlope,
                      float rescaleIntercept);

    Orthanc::PixelFormat GetOutputFormat() const
    {
      return outputFormat_;
    }

    void SetOutputFormat(Orthanc::PixelFormat format);

    ImageInterpolation GetInterpolation() const
    {
      return interpolation_;
    }

    void SetInterpolation(ImageInterpolation interpolation);

    bool IsFastMode() const
    {
      return fastMode_;
    }

    void EnableFastMode(bool enabled)
    {
      fastMode_ = enabled;
    }

    bool IsSuccess() const
    {
      return success_;
    }

    const Extent2D& GetOutputExtent() const;

    const Orthanc::ImageAccessor& GetOutputSlice() const;

    Orthanc::ImageAccessor* ReleaseOutputSlice();

    void Apply(const ImageBuffer3D& source,
               const VolumeImageGeometry& geometry,
               const CoordinateSystem3D& plane);

    void Apply(const ImageBuffer3D& source,
               const VolumeImageGeometry& geometry,
               const CoordinateSystem3D& plane,
               double voxelSize);

    double GetPixelSpacing() const;
  };
}

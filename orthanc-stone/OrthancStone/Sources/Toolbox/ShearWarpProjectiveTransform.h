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

#include "FiniteProjectiveCamera.h"

namespace OrthancStone
{
  class ShearWarpProjectiveTransform : public boost::noncopyable
  {
  private:
    Matrix        k_;
    Matrix        M_shear;
    Matrix        M_warp;
    Vector        eye_o;
    unsigned int  intermediateWidth_;
    unsigned int  intermediateHeight_;

  public:
    ShearWarpProjectiveTransform(const Matrix& M_view,
                                 //const Matrix& P,           // Permutation applied to the volume
                                 unsigned int volumeWidth,
                                 unsigned int volumeHeight,
                                 unsigned int volumeDepth,
                                 double pixelSpacingX,
                                 double pixelSpacingY,
                                 unsigned int imageWidth,
                                 unsigned int imageHeight);

    const Matrix& GetIntrinsicParameters() const
    {
      return k_;
    }

    const Matrix& GetShear() const
    {
      return M_shear;
    }

    const Matrix& GetWarp() const
    {
      return M_warp;
    }

    const Vector& GetCameraCenter() const
    {
      return eye_o;
    }

    unsigned int GetIntermediateWidth() const
    {
      return intermediateWidth_;
    }

    unsigned int GetIntermediateHeight() const
    {
      return intermediateHeight_;
    }

    FiniteProjectiveCamera *CreateCamera() const;

    void ComputeShearOnSlice(double& a11,
                             double& b1,
                             double& a22,
                             double& b2,
                             double& shearedZ,
                             const double sourceZ);

    static Matrix CalibrateView(const Vector& camera,
                                const Vector& principalPoint,
                                double angle);

    static Orthanc::ImageAccessor* ApplyAxial(float& maxValue,
                                              const Matrix& M_view,  // cf. "CalibrateView()"
                                              const ImageBuffer3D& source,
                                              const VolumeImageGeometry& geometry,
                                              Orthanc::PixelFormat targetFormat,
                                              unsigned int targetWidth,
                                              unsigned int targetHeight,
                                              bool mip,
                                              double pixelSpacing,
                                              unsigned int countSlices,
                                              ImageInterpolation shearInterpolation,
                                              ImageInterpolation warpInterpolation);
  };
}

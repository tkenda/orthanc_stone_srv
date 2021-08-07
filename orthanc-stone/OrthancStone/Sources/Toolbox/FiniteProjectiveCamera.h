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

#include "LinearAlgebra.h"
#include "../Volumes/ImageBuffer3D.h"
#include "../Volumes/VolumeImageGeometry.h"

namespace OrthancStone
{
  // Reference: "Multiple View Geometry in Computer Vision (2nd Edition)"
  class FiniteProjectiveCamera : public boost::noncopyable
  {
  private:
    Matrix  p_;     // 3x4 matrix - Equation (6.11) - page 157
    Matrix  k_;     // 3x3 matrix of intrinsic parameters - Equation (6.10) - page 157
    Matrix  r_;     // 3x3 rotation matrix in 3D space
    Vector  c_;     // 3x1 vector in 3D space corresponding to camera center
    Matrix  minv_;  // Inverse of the M = P(1:3,1:3) submatrix

    void ComputeMInverse();

    void Setup(const Matrix& k,
               const Matrix& r,
               const Vector& c);

    void Setup(const Matrix& p);

  public:
    FiniteProjectiveCamera(const Matrix& k,
                           const Matrix& r,
                           const Vector& c)
    {
      Setup(k, r, c);
    }

    explicit FiniteProjectiveCamera(const Matrix& p)
    {
      Setup(p);
    }

    FiniteProjectiveCamera(const double k[9],
                           const double r[9],
                           const double c[3]);

    explicit FiniteProjectiveCamera(const double p[12]);

    // Constructor that implements camera calibration
    FiniteProjectiveCamera(const Vector& camera,
                           const Vector& principalPoint,
                           double angle,
                           unsigned int imageWidth,
                           unsigned int imageHeight,
                           double pixelSpacingX,
                           double pixelSpacingY);

    const Matrix& GetMatrix() const
    {
      return p_;
    }

    const Matrix& GetRotation() const
    {
      return r_;
    }

    const Vector& GetCenter() const
    {
      return c_;
    }

    const Matrix& GetIntrinsicParameters() const
    {
      return k_;
    }

    // Computes the 3D vector that represents the direction from the
    // camera center to the (x,y) imaged point
    Vector GetRayDirection(double x,
                           double y) const;

    // Apply the camera to a 3D point "v" that is not at infinity. "v"
    // can be encoded either as a non-homogeneous vector (3
    // components), or as a homogeneous vector (4 components).
    void ApplyFinite(double& x,
                     double& y,
                     const Vector& v) const;

    // Apply the camera to a 3D point "v" that is possibly at
    // infinity. The result is a 2D point in homogeneous coordinates.
    Vector ApplyGeneral(const Vector& v) const;

    Orthanc::ImageAccessor* ApplyRaytracer(const ImageBuffer3D& source,
                                           const VolumeImageGeometry& geometry,
                                           Orthanc::PixelFormat targetFormat,
                                           unsigned int targetWidth,
                                           unsigned int targetHeight,
                                           bool mip) const;
  };
}

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

namespace OrthancStone
{
  namespace GeometryToolbox
  {
    void ProjectPointOntoPlane(Vector& result,
                               const Vector& point,
                               const Vector& planeNormal,
                               const Vector& planeOrigin);

    /*
    Alternated faster implementation (untested yet)
    */
    void ProjectPointOntoPlane2(double& resultX,
                                double& resultY,
                                double& resultZ,
                                const Vector& point,
                                const Vector& planeNormal,
                                const Vector& planeOrigin);

    bool IsParallel(const Vector& u,
                    const Vector& v);

    bool IsParallelOrOpposite(bool& isOpposite,
                              const Vector& u,
                              const Vector& v);

    bool IntersectTwoPlanes(Vector& p,
                            Vector& direction,
                            const Vector& origin1,
                            const Vector& normal1,
                            const Vector& origin2,
                            const Vector& normal2);

    bool ClipLineToRectangle(double& x1,  // Coordinates of the clipped line (out)
                             double& y1,
                             double& x2,
                             double& y2,
                             const double ax,  // Two points defining the line (in)
                             const double ay,
                             const double bx,
                             const double by,
                             const double& xmin,   // Coordinates of the rectangle (in)
                             const double& ymin,
                             const double& xmax,
                             const double& ymax);

    bool GetPixelSpacing(double& spacingX, 
                         double& spacingY,
                         const Orthanc::DicomMap& dicom);

    inline double ProjectAlongNormal(const Vector& point,
                                     const Vector& normal)
    {
      return boost::numeric::ublas::inner_prod(point, normal);
    }

    Matrix CreateRotationMatrixAlongX(double a);

    Matrix CreateRotationMatrixAlongY(double a);

    Matrix CreateRotationMatrixAlongZ(double a);

    Matrix CreateTranslationMatrix(double dx,
                                   double dy,
                                   double dz);

    Matrix CreateScalingMatrix(double sx,
                               double sy,
                               double sz);
    
    bool IntersectPlaneAndSegment(Vector& p,
                                  const Vector& normal,
                                  double d,
                                  const Vector& edgeFrom,
                                  const Vector& edgeTo);

    bool IntersectPlaneAndLine(Vector& p,
                               const Vector& normal,
                               double d,
                               const Vector& origin,
                               const Vector& direction);

    void AlignVectorsWithRotation(Matrix& r,
                                  const Vector& a,
                                  const Vector& b);

    void ComputeNormalFromCosines(Vector& normal,
                                  const Vector& cosines);

    bool ComputeNormal(Vector& normal,
                       const Orthanc::DicomMap& dicom);

    inline float ComputeBilinearInterpolationUnitSquare(float x,
                                                        float y,
                                                        float f00,    // source(0, 0)
                                                        float f01,    // source(1, 0)
                                                        float f10,    // source(0, 1)
                                                        float f11);   // source(1, 1)

    inline float ComputeTrilinearInterpolationUnitSquare(float x,
                                                         float y,
                                                         float z,
                                                         float f000,   // source(0, 0, 0)
                                                         float f001,   // source(1, 0, 0)
                                                         float f010,   // source(0, 1, 0)
                                                         float f011,   // source(1, 1, 0)
                                                         float f100,   // source(0, 0, 1)
                                                         float f101,   // source(1, 0, 1)
                                                         float f110,   // source(0, 1, 1)
                                                         float f111);  // source(1, 1, 1)
  };
}


float OrthancStone::GeometryToolbox::ComputeBilinearInterpolationUnitSquare(float x,
                                                                            float y,
                                                                            float f00,
                                                                            float f01,
                                                                            float f10,
                                                                            float f11)
{
  // This function only works within the unit square
  assert(x >= 0 && y >= 0 && x <= 1 && y <= 1);

  // https://en.wikipedia.org/wiki/Bilinear_interpolation#Unit_square
  return (f00 * (1.0f - x) * (1.0f - y) +
          f01 * x * (1.0f - y) +
          f10 * (1.0f - x) * y +
          f11 * x * y);
}


float OrthancStone::GeometryToolbox::ComputeTrilinearInterpolationUnitSquare(float x,
                                                                             float y,
                                                                             float z,
                                                                             float f000,
                                                                             float f001,
                                                                             float f010,
                                                                             float f011,
                                                                             float f100,
                                                                             float f101,
                                                                             float f110,
                                                                             float f111)
{
  // "In practice, a trilinear interpolation is identical to two
  // bilinear interpolation combined with a linear interpolation"
  // https://en.wikipedia.org/wiki/Trilinear_interpolation#Method
  float a = ComputeBilinearInterpolationUnitSquare(x, y, f000, f001, f010, f011);
  float b = ComputeBilinearInterpolationUnitSquare(x, y, f100, f101, f110, f111);

  return (1.0f - z) * a + z * b;
}

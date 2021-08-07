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


#include "GeometryToolbox.h"

#include <Logging.h>
#include <OrthancException.h>

#include <cassert>

namespace OrthancStone
{
  namespace GeometryToolbox
  {
    void ProjectPointOntoPlane(Vector& result,
                               const Vector& point,
                               const Vector& planeNormal,
                               const Vector& planeOrigin)
    {
      double norm =  boost::numeric::ublas::norm_2(planeNormal);
      if (LinearAlgebra::IsCloseToZero(norm))
      {
        // Division by zero
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }

      // Make sure the norm of the normal is 1
      Vector n;
      n = planeNormal / norm;

      // Algebraic form of line–plane intersection, where the line passes
      // through "point" along the direction "normal" (thus, l == n)
      // https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection#Algebraic_form
      result = boost::numeric::ublas::inner_prod(planeOrigin - point, n) * n + point;
    }

    /*
    undefined results if vector are not 3D
    */
    void ProjectPointOntoPlane2(
      double& resultX,
      double& resultY,
      double& resultZ,
      const Vector& point,
      const Vector& planeNormal,
      const Vector& planeOrigin)
    {
      double pointX = point[0];
      double pointY = point[1];
      double pointZ = point[2];

      double planeNormalX = planeNormal[0];
      double planeNormalY = planeNormal[1];
      double planeNormalZ = planeNormal[2];

      double planeOriginX = planeOrigin[0];
      double planeOriginY = planeOrigin[1];
      double planeOriginZ = planeOrigin[2];

      double normSq = (planeNormalX * planeNormalX) + (planeNormalY * planeNormalY) + (planeNormalZ * planeNormalZ);

      // Algebraic form of line–plane intersection, where the line passes
      // through "point" along the direction "normal" (thus, l == n)
      // https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection#Algebraic_form

      if (LinearAlgebra::IsNear(1.0, normSq))
      {
        double nX = planeNormalX;
        double nY = planeNormalY;
        double nZ = planeNormalZ;
        
        double prod = (planeOriginX - pointX) * nX + (planeOriginY - pointY) * nY + (planeOriginZ - pointZ) * nZ;

        resultX = prod * nX + pointX; 
        resultY = prod * nY + pointY; 
        resultZ = prod * nZ + pointZ;
      }
      else
      {
        double norm = sqrt(normSq);
        if (LinearAlgebra::IsCloseToZero(norm))
        {
          // Division by zero
          throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
        }
        double invNorm = 1.0 / norm;
        double nX = planeNormalX * invNorm;
        double nY = planeNormalY * invNorm;
        double nZ = planeNormalZ * invNorm;

        double prod = (planeOriginX - pointX) * nX + (planeOriginY - pointY) * nY + (planeOriginZ - pointZ) * nZ;

        resultX = prod * nX + pointX;
        resultY = prod * nY + pointY;
        resultZ = prod * nZ + pointZ;
      }
    }

    bool IsParallelOrOpposite(bool& isOpposite,
                              const Vector& u,
                              const Vector& v)
    {
      // The dot product of the two vectors gives the cosine of the angle
      // between the vectors
      // https://en.wikipedia.org/wiki/Dot_product

      double normU = boost::numeric::ublas::norm_2(u);
      double normV = boost::numeric::ublas::norm_2(v);

      if (LinearAlgebra::IsCloseToZero(normU) ||
          LinearAlgebra::IsCloseToZero(normV))
      {
        return false;
      }

      double cosAngle = boost::numeric::ublas::inner_prod(u, v) / (normU * normV);

      // The angle must be zero, so the cosine must be almost equal to
      // cos(0) == 1 (or cos(180) == -1 if allowOppositeDirection == true)

      if (LinearAlgebra::IsCloseToZero(cosAngle - 1.0))
      {
        isOpposite = false;
        return true;
      }
      else if (LinearAlgebra::IsCloseToZero(fabs(cosAngle) - 1.0))
      {
        isOpposite = true;
        return true;
      }
      else
      {
        return false;
      }
    }


    bool IsParallel(const Vector& u,
                    const Vector& v)
    {
      bool isOpposite;
      return (IsParallelOrOpposite(isOpposite, u, v) &&
              !isOpposite);
    }


    bool IntersectTwoPlanes(Vector& p,
                            Vector& direction,
                            const Vector& origin1,
                            const Vector& normal1,
                            const Vector& origin2,
                            const Vector& normal2)
    {
      // This is "Intersection of 2 Planes", possibility "(C) 3 Plane
      // Intersect Point" of:
      // http://geomalgorithms.com/a05-_intersect-1.html

      // The direction of the line of intersection is orthogonal to the
      // normal of both planes
      LinearAlgebra::CrossProduct(direction, normal1, normal2);

      double norm = boost::numeric::ublas::norm_2(direction);
      if (LinearAlgebra::IsCloseToZero(norm))
      {
        // The two planes are parallel or coincident
        return false;
      }

      double d1 = -boost::numeric::ublas::inner_prod(normal1, origin1);
      double d2 = -boost::numeric::ublas::inner_prod(normal2, origin2);
      Vector tmp = d2 * normal1 - d1 * normal2;

      LinearAlgebra::CrossProduct(p, tmp, direction);
      p /= norm;

      return true;
    }


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
                             const double& ymax)
    {
      // This is Skala algorithm for rectangles, "A new approach to line
      // and line segment clipping in homogeneous coordinates"
      // (2005). This is a direct, non-optimized translation of Algorithm
      // 2 in the paper.

      static const uint8_t tab1[16] = { 255 /* none */,
                                        0,
                                        0,
                                        1,
                                        1,
                                        255 /* na */,
                                        0,
                                        2,
                                        2,
                                        0,
                                        255 /* na */,
                                        1,
                                        1,
                                        0,
                                        0,
                                        255 /* none */ };


      static const uint8_t tab2[16] = { 255 /* none */,
                                        3,
                                        1,
                                        3,
                                        2,
                                        255 /* na */,
                                        2,
                                        3,
                                        3,
                                        2,
                                        255 /* na */,
                                        2,
                                        3,
                                        1,
                                        3,
                                        255 /* none */ };

      // Create the coordinates of the rectangle
      Vector x[4];
      LinearAlgebra::AssignVector(x[0], xmin, ymin, 1.0);
      LinearAlgebra::AssignVector(x[1], xmax, ymin, 1.0);
      LinearAlgebra::AssignVector(x[2], xmax, ymax, 1.0);
      LinearAlgebra::AssignVector(x[3], xmin, ymax, 1.0);

      // Move to homogoneous coordinates in 2D
      Vector p;

      {
        Vector a, b;
        LinearAlgebra::AssignVector(a, ax, ay, 1.0);
        LinearAlgebra::AssignVector(b, bx, by, 1.0);
        LinearAlgebra::CrossProduct(p, a, b);
      }

      uint8_t c = 0;

      for (unsigned int k = 0; k < 4; k++)
      {
        if (boost::numeric::ublas::inner_prod(p, x[k]) >= 0)
        {
          c |= (1 << k);
        }
      }

      assert(c < 16);

      uint8_t i = tab1[c];
      uint8_t j = tab2[c];

      if (i == 255 || j == 255)
      {
        return false;   // No intersection
      }
      else
      {
        Vector a, b, e;
        LinearAlgebra::CrossProduct(e, x[i], x[(i + 1) % 4]);
        LinearAlgebra::CrossProduct(a, p, e);
        LinearAlgebra::CrossProduct(e, x[j], x[(j + 1) % 4]);
        LinearAlgebra::CrossProduct(b, p, e);

        // Go back to non-homogeneous coordinates
        x1 = a[0] / a[2];
        y1 = a[1] / a[2];
        x2 = b[0] / b[2];
        y2 = b[1] / b[2];

        return true;
      }
    }


    bool GetPixelSpacing(double& spacingX, 
                         double& spacingY,
                         const Orthanc::DicomMap& dicom)
    {
      Vector v;

      if (LinearAlgebra::ParseVector(v, dicom, Orthanc::DICOM_TAG_PIXEL_SPACING))
      {
        if (v.size() != 2 ||
            v[0] <= 0 ||
            v[1] <= 0)
        {
          LOG(ERROR) << "Bad value for PixelSpacing tag";
          throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
        }
        else
        {
          // WARNING: X/Y are swapped (Y comes first)
          spacingX = v[1];
          spacingY = v[0];
          return true;
        }
      }
      else
      {
        // The "PixelSpacing" is of type 1C: It could be absent, use
        // default value in such a case
        spacingX = 1;
        spacingY = 1;
        return false;
      }
    }

    
    Matrix CreateRotationMatrixAlongX(double a)
    {
      // Rotate along X axis (R_x)
      // https://en.wikipedia.org/wiki/Rotation_matrix#Basic_rotations
      Matrix r(3, 3);
      r(0,0) = 1;
      r(0,1) = 0;
      r(0,2) = 0;
      r(1,0) = 0;
      r(1,1) = cos(a);
      r(1,2) = -sin(a);
      r(2,0) = 0;
      r(2,1) = sin(a);
      r(2,2) = cos(a);
      return r;
    }
    

    Matrix CreateRotationMatrixAlongY(double a)
    {
      // Rotate along Y axis (R_y)
      // https://en.wikipedia.org/wiki/Rotation_matrix#Basic_rotations
      Matrix r(3, 3);
      r(0,0) = cos(a);
      r(0,1) = 0;
      r(0,2) = sin(a);
      r(1,0) = 0;
      r(1,1) = 1;
      r(1,2) = 0;
      r(2,0) = -sin(a);
      r(2,1) = 0;
      r(2,2) = cos(a);
      return r;
    }


    Matrix CreateRotationMatrixAlongZ(double a)
    {
      // Rotate along Z axis (R_z)
      // https://en.wikipedia.org/wiki/Rotation_matrix#Basic_rotations
      Matrix r(3, 3);
      r(0,0) = cos(a);
      r(0,1) = -sin(a);
      r(0,2) = 0;
      r(1,0) = sin(a);
      r(1,1) = cos(a);
      r(1,2) = 0;
      r(2,0) = 0;
      r(2,1) = 0;
      r(2,2) = 1;
      return r;
    }    


    Matrix CreateTranslationMatrix(double dx,
                                   double dy,
                                   double dz)
    {
      Matrix m = LinearAlgebra::IdentityMatrix(4);
      m(0,3) = dx;
      m(1,3) = dy;
      m(2,3) = dz;
      return m;    
    }


    Matrix CreateScalingMatrix(double sx,
                               double sy,
                               double sz)
    {
      Matrix m = LinearAlgebra::IdentityMatrix(4);
      m(0,0) = sx;
      m(1,1) = sy;
      m(2,2) = sz;
      return m;    
    }


    bool IntersectPlaneAndSegment(Vector& p,
                                  const Vector& normal,
                                  double d,
                                  const Vector& edgeFrom,
                                  const Vector& edgeTo)
    {
      // http://geomalgorithms.com/a05-_intersect-1.html#Line-Plane-Intersection

      // Check for parallel line and plane
      Vector direction = edgeTo - edgeFrom;
      double denominator = boost::numeric::ublas::inner_prod(direction, normal);

      if (fabs(denominator) < 100.0 * std::numeric_limits<double>::epsilon())
      {
        return false;
      }
      else
      {
        // Compute intersection
        double t = -(normal[0] * edgeFrom[0] + 
                     normal[1] * edgeFrom[1] + 
                     normal[2] * edgeFrom[2] + d) / denominator;

        if (t >= 0 && t <= 1)
        {
          // The intersection lies inside edge segment
          p = edgeFrom + t * direction;
          return true;
        }
        else
        {
          return false;
        }
      }
    }


    bool IntersectPlaneAndLine(Vector& p,
                               const Vector& normal,
                               double d,
                               const Vector& origin,
                               const Vector& direction)
    {
      // http://geomalgorithms.com/a05-_intersect-1.html#Line-Plane-Intersection

      // Check for parallel line and plane
      double denominator = boost::numeric::ublas::inner_prod(direction, normal);

      if (fabs(denominator) < 100.0 * std::numeric_limits<double>::epsilon())
      {
        return false;
      }
      else
      {
        // Compute intersection
        double t = -(normal[0] * origin[0] + 
                     normal[1] * origin[1] + 
                     normal[2] * origin[2] + d) / denominator;

        p = origin + t * direction;
        return true;
      }
    }


    void AlignVectorsWithRotation(Matrix& r,
                                  const Vector& a,
                                  const Vector& b)
    {
      // This is Rodrigues' rotation formula:
      // https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula#Matrix_notation

      // Check also result A4.6 from "Multiple View Geometry in Computer
      // Vision - 2nd edition" (p. 584)
  
      if (a.size() != 3 ||
          b.size() != 3)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }

      double aNorm = boost::numeric::ublas::norm_2(a);
      double bNorm = boost::numeric::ublas::norm_2(b);

      if (LinearAlgebra::IsCloseToZero(aNorm) ||
          LinearAlgebra::IsCloseToZero(bNorm))
      {
        LOG(ERROR) << "Vector with zero norm";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);      
      }

      Vector aUnit, bUnit;
      aUnit = a / aNorm;
      bUnit = b / bNorm;

      Vector v;
      LinearAlgebra::CrossProduct(v, aUnit, bUnit);

      double cosine = boost::numeric::ublas::inner_prod(aUnit, bUnit);

      if (LinearAlgebra::IsCloseToZero(1 + cosine))
      {
        // "a == -b": TODO
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
      }

      Matrix k;
      LinearAlgebra::CreateSkewSymmetric(k, v);

#if 0
      double sine = boost::numeric::ublas::norm_2(v);

      r = (boost::numeric::ublas::identity_matrix<double>(3) +
           sine * k + 
           (1 - cosine) * boost::numeric::ublas::prod(k, k));
#else
      r = (boost::numeric::ublas::identity_matrix<double>(3) +
           k + 
           boost::numeric::ublas::prod(k, k) / (1 + cosine));
#endif
    }

    
    void ComputeNormalFromCosines(Vector& normal,
                                  const Vector& cosines)
    {
      if (cosines.size() != 6)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }
      else
      {
        normal.resize(3);
        normal[0] = cosines[1] * cosines[5] - cosines[2] * cosines[4];
        normal[1] = cosines[2] * cosines[3] - cosines[0] * cosines[5];
        normal[2] = cosines[0] * cosines[4] - cosines[1] * cosines[3];
      }
    }

    
    bool ComputeNormal(Vector& normal,
                       const Orthanc::DicomMap& dicom)
    {
      Vector cosines;
      if (LinearAlgebra::ParseVector(cosines, dicom, Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT) &&
          cosines.size() == 6)
      {
        ComputeNormalFromCosines(normal, cosines);
        return true;
      }
      else
      {
        return false;
      }
    }
  }
}

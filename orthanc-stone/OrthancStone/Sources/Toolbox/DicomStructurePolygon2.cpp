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

#ifdef BGO_ENABLE_DICOMSTRUCTURESETLOADER2

#include "DicomStructurePolygon2.h"

#include "../Toolbox/LinearAlgebra.h"

#include <Logging.h>

namespace OrthancStone
{
  void DicomStructurePolygon2::ComputeDependentProperties()
  {
    ORTHANC_ASSERT(state_ == Building);

    for (size_t j = 0; j < points_.size(); ++j)
    {
      // TODO: move to AddPoint!
      const Point3D& p = points_[j];
      if (p[0] < minX_)
        minX_ = p[0];
      if (p[0] > maxX_)
        maxX_ = p[0];

      if (p[1] < minY_)
        minY_ = p[1];
      if (p[1] > maxY_)
        maxY_ = p[1];

      if (p[2] < minZ_)
        minZ_ = p[2];
      if (p[2] > maxZ_)
        maxZ_ = p[2];
    }

    if (LinearAlgebra::IsNear(minX_, maxX_))
    {
      LinearAlgebra::AssignVector(normal_, 1, 0, 0);
      //ORTHANC_ASSERT(!LinearAlgebra::IsNear(minX, maxX));
      ORTHANC_ASSERT(!LinearAlgebra::IsNear(minY_, maxY_));
      ORTHANC_ASSERT(!LinearAlgebra::IsNear(minZ_, maxZ_));
    }
    else if (LinearAlgebra::IsNear(minY_, maxY_))
    {
      LinearAlgebra::AssignVector(normal_, 0, 1, 0);
      ORTHANC_ASSERT(!LinearAlgebra::IsNear(minX_, maxX_));
      ORTHANC_ASSERT(!LinearAlgebra::IsNear(minZ_, maxZ_));
    }
    else if (LinearAlgebra::IsNear(minZ_, maxZ_))
    {
      LinearAlgebra::AssignVector(normal_, 0, 0, 1);
      ORTHANC_ASSERT(!LinearAlgebra::IsNear(minX_, maxX_));
      ORTHANC_ASSERT(!LinearAlgebra::IsNear(minY_, maxY_));
    }
    else
    {
      LOG(ERROR) << "The contour is not coplanar and not parallel to any axis.";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    state_ = Valid;
  }


  void DicomStructurePolygon2::ProjectOnConstantPlane(
    std::vector<Point2D>& intersections, const CoordinateSystem3D& plane) const
  {
    // the plane can either have constant X, or constant Y.
    // - for constant Z planes, use the ProjectOnParallelPlane method
    // - other type of planes are not supported

    // V is the coordinate that is constant in the plane
    double planeV = 0.0;

    // if true, then "u" in the code is "x" and "v" is "y". 
    // (v is constant in the plane)
    bool uvxy = false;

    size_t uindex = static_cast<size_t>(-1);
    size_t vindex = static_cast<size_t>(-1);

    ORTHANC_ASSERT(LinearAlgebra::IsNear(plane.GetNormal()[2], 0.0));

    if (LinearAlgebra::IsNear(plane.GetNormal()[1], 0.0))
    {
      // normal is 1,0,0 (or -1,0,0). 
      // plane is constant X
      uindex = 1;
      vindex = 0;

      uvxy = false;
      planeV = plane.GetOrigin()[0];
      if (planeV < minX_)
        return;
      if (planeV > maxX_)
        return;
    }
    else if (LinearAlgebra::IsNear(plane.GetNormal()[0], 0.0))
    {
      // normal is 0,1,0 (or 0,-1,0). 
      // plane is constant Y
      uindex = 0;
      vindex = 1;

      uvxy = true;
      planeV = plane.GetOrigin()[1];
      if (planeV < minY_)
        return;
      if (planeV > maxY_)
        return;
    }
    else
    {
      // if the following assertion(s) fail(s), it means the plane is NOT a constant-X or constant-Y plane
      LOG(ERROR) << "Plane normal must be (a,0,0) or (0,a,0), with a == -1 or a == 1";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    size_t pointCount = GetPointCount();
    if (pointCount >= 3)
    {
      // this vector will contain the coordinates of the intersection points
      // between the plane and the polygon.
      // these are expressed in the U coordinate, that is either X or Y, 
      // depending upon the plane orientation
      std::vector<double> uIntersections;

      // we loop on the segments of the polygon (TODO: optimize)
      // and we compute the intersection between each segment and the cut
      // cutting plane (slice) has a constant X

      for (size_t iPoint = 0; iPoint < pointCount; ++iPoint)
      {
        double u1 = points_[iPoint][uindex];
        double v1 = points_[iPoint][vindex];

        double u2 = 0;
        double v2 = 0;

        if (iPoint < pointCount - 1)
        {
          u2 = points_[iPoint + 1][uindex];
          v2 = points_[iPoint + 1][vindex];
        }
        else
        {
          u2 = points_[0][uindex];
          v2 = points_[0][vindex];
        }

        // Check if the segment intersects the plane
        if ((std::min(v1, v2) <= planeV) && (std::max(v1, v2) >= planeV))
        {
          // special case: the segment is parallel to the plane but close to it
          if (LinearAlgebra::IsNear(v1, v2))
          {
            // in that case, we choose to label both points as an intersection
            double x, y;
            plane.ProjectPoint(x, y, points_[iPoint]);
            intersections.push_back(Point2D(x, y));

            plane.ProjectPoint(x, y, points_[iPoint + 1]);
            intersections.push_back(Point2D(x, y));
          }
          else
          {
            // we are looking for u so that (u,planeV) belongs to the segment
            // let's define alpha = (u-u2)/(u1-u2) --> u = alpha*(u1-u2) + u2
            // alpha = (v2-planeV)/(v2-v1)
            // because the following two triangles are similar
            // [ (planeY,x)  , (y2,x2), (planeY,x2) ] or
            // [ (planeX,y)  , (x2,y2), (planeX,y2) ]
            // and
            // [ (y1    ,x1) , (y2,x2), (y1    ,x2) ] or
            // [ (x1    ,y1) , (x2,y2), (x1    ,y2) ]

            /*
              void CoordinateSystem3D::ProjectPoint(double& offsetX,
              double& offsetY,
              const Vector& point) const
            */
            double alpha = (v2 - planeV) / (v2 - v1);

            // get rid of numerical oddities
            if (alpha < 0.0)
              alpha = 0.0;
            if (alpha > 1.0)
              alpha = 1.0;
            double u = alpha * (u1 - u2) + u2;

            // here is the intersection in world coordinates
            Vector intersection;
            if(uvxy)
              LinearAlgebra::AssignVector(intersection, u, planeV, minZ_);
            else
              LinearAlgebra::AssignVector(intersection, planeV, u, minZ_);

            // and we convert it to plane coordinates
            {
              double xi, yi;
              plane.ProjectPoint(xi, yi, intersection);

              // we consider that the x axis is always parallel to the polygons
              // TODO: is this hypothesis safe??????
              uIntersections.insert(std::lower_bound(uIntersections.begin(), uIntersections.end(), xi), xi);
            }
          }
        }
      } // end of for (size_t iPoint = 0; iPoint < pointCount; ++iPoint)
    
      // now we convert the intersections to plane points
      // we consider that the x axis is always parallel to the polygons
      // TODO: same hypothesis as above: plane is perpendicular to polygons, 
      // plane is parallel to the XZ (constant Y) or YZ (constant X) 3D planes
      for (size_t i = 0; i < uIntersections.size(); ++i)
      {
        double x = uIntersections[i];
        intersections.push_back(Point2D(x, minZ_));
      }
    } // end of if (pointCount >= 3)
    else
    {
      LOG(ERROR) << "This polygon has " << pointCount << " vertices, which is less than 3 --> skipping";
    }
  } 

  void DicomStructurePolygon2::ProjectOnParallelPlane(
    std::vector< std::pair<Point2D, Point2D> >& segments, 
    const CoordinateSystem3D& plane) const
  {
    if (points_.size() < 3)
      return;

    // the plane is horizontal
    ORTHANC_ASSERT(LinearAlgebra::IsNear(plane.GetNormal()[0], 0.0));
    ORTHANC_ASSERT(LinearAlgebra::IsNear(plane.GetNormal()[1], 0.0));

    segments.clear();
    segments.reserve(points_.size());
    // since the returned values need to be expressed in the supplied coordinate
    // system, we need to subtract origin_ from the returned points

    double planeOriginX = plane.GetOrigin()[0];
    double planeOriginY = plane.GetOrigin()[1];

    // precondition: points_.size() >= 3
    for (size_t j = 0; j < points_.size()-1; ++j)
    {
      // segment between point j and j+1

      const Point3D& point0 = GetPoint(j);
      // subtract plane origin x and y
      Point2D p0(point0[0] - planeOriginX, point0[1] - planeOriginY);
    
      const Point3D& point1 = GetPoint(j+1);
      // subtract plane origin x and y
      Point2D p1(point1[0] - planeOriginX, point1[1] - planeOriginY);

      segments.push_back(std::pair<Point2D, Point2D>(p0,p1));
    }


    // final segment 

    const Point3D& point0 = GetPoint(points_.size() - 1);
    // subtract plane origin x and y
    Point2D p0(point0[0] - planeOriginX, point0[1] - planeOriginY);

    const Point3D& point1 = GetPoint(0);
    // subtract plane origin x and y
    Point2D p1(point1[0] - planeOriginX, point1[1] - planeOriginY);

    segments.push_back(std::pair<Point2D, Point2D>(p0, p1));
  }

  double DicomStructurePolygon2::GetZ() const
  {
    ORTHANC_ASSERT(LinearAlgebra::IsNear(normal_[0], 0.0));
    ORTHANC_ASSERT(LinearAlgebra::IsNear(normal_[1], 0.0));
    ORTHANC_ASSERT(LinearAlgebra::IsNear(minZ_, maxZ_));
    return minZ_;
  }
}

#endif 
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2


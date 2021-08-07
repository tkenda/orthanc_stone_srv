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


#include "OrientedVolumeBoundingBox.h"

#include "../Toolbox/GeometryToolbox.h"
#include "ImageBuffer3D.h"

#include <OrthancException.h>

#include <cassert>

namespace OrthancStone
{
  OrientedVolumeBoundingBox::OrientedVolumeBoundingBox(const VolumeImageGeometry& geometry)
  {
    unsigned int n = geometry.GetDepth();
    if (n < 1)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageSize);      
    }

    Vector dim = geometry.GetVoxelDimensions(VolumeProjection_Axial);

    u_ = geometry.GetAxialGeometry().GetAxisX();
    v_ = geometry.GetAxialGeometry().GetAxisY();
    w_ = geometry.GetAxialGeometry().GetNormal();

    hu_ = static_cast<double>(geometry.GetWidth() * dim[0] / 2.0);
    hv_ = static_cast<double>(geometry.GetHeight() * dim[1] / 2.0);
    hw_ = static_cast<double>(geometry.GetDepth() * dim[2] / 2.0);
      
    c_ = (geometry.GetAxialGeometry().GetOrigin() + 
          (hu_ - dim[0] / 2.0) * u_ +
          (hv_ - dim[1] / 2.0) * v_ +
          (hw_ - dim[2] / 2.0) * w_);
  }


  bool OrientedVolumeBoundingBox::HasIntersectionWithPlane(std::vector<Vector>& points,
                                                           const Vector& normal,
                                                           double d) const
  {
    assert(normal.size() == 3);

    double r = (hu_ * fabs(boost::numeric::ublas::inner_prod(normal, u_)) +
                hv_ * fabs(boost::numeric::ublas::inner_prod(normal, v_)) +
                hw_ * fabs(boost::numeric::ublas::inner_prod(normal, w_)));

    double s = boost::numeric::ublas::inner_prod(normal, c_) + d;

    if (fabs(s) >= r)
    {
      // No intersection, or intersection is reduced to a single point
      return false;
    }
    else
    {
      Vector p;

      // Loop over all the 12 edges (segments) of the oriented
      // bounding box, and check whether they intersect the plane
        
      // X-aligned edges
      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ - u_ * hu_ - v_ * hv_ - w_ * hw_,
           c_ + u_ * hu_ - v_ * hv_ - w_ * hw_))
      {
        points.push_back(p);
      }

      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ - u_ * hu_ + v_ * hv_ - w_ * hw_,
           c_ + u_ * hu_ + v_ * hv_ - w_ * hw_))
      {
        points.push_back(p);
      }

      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ - u_ * hu_ - v_ * hv_ + w_ * hw_,
           c_ + u_ * hu_ - v_ * hv_ + w_ * hw_))
      {
        points.push_back(p);
      }

      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ - u_ * hu_ + v_ * hv_ + w_ * hw_,
           c_ + u_ * hu_ + v_ * hv_ + w_ * hw_))
      {
        points.push_back(p);
      }

      // Y-aligned edges
      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ - u_ * hu_ - v_ * hv_ - w_ * hw_,
           c_ - u_ * hu_ + v_ * hv_ - w_ * hw_))
      {
        points.push_back(p);
      }

      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ + u_ * hu_ - v_ * hv_ - w_ * hw_,
           c_ + u_ * hu_ + v_ * hv_ - w_ * hw_))
      {
        points.push_back(p);
      }

      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ - u_ * hu_ - v_ * hv_ + w_ * hw_,
           c_ - u_ * hu_ + v_ * hv_ + w_ * hw_))
      {
        points.push_back(p);
      }

      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ + u_ * hu_ - v_ * hv_ + w_ * hw_,
           c_ + u_ * hu_ + v_ * hv_ + w_ * hw_))
      {
        points.push_back(p);
      }

      // Z-aligned edges
      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ - u_ * hu_ - v_ * hv_ - w_ * hw_,
           c_ - u_ * hu_ - v_ * hv_ + w_ * hw_))
      {
        points.push_back(p);
      }

      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ + u_ * hu_ - v_ * hv_ - w_ * hw_,
           c_ + u_ * hu_ - v_ * hv_ + w_ * hw_))
      {
        points.push_back(p);
      }

      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ - u_ * hu_ + v_ * hv_ - w_ * hw_,
           c_ - u_ * hu_ + v_ * hv_ + w_ * hw_))
      {
        points.push_back(p);
      }

      if (GeometryToolbox::IntersectPlaneAndSegment
          (p, normal, d,
           c_ + u_ * hu_ + v_ * hv_ - w_ * hw_,
           c_ + u_ * hu_ + v_ * hv_ + w_ * hw_))
      {
        points.push_back(p);
      }

      return true;
    }
  }


  bool OrientedVolumeBoundingBox::HasIntersection(std::vector<Vector>& points,
                                                  const CoordinateSystem3D& plane) const
  {
    // From the vector equation of a 3D plane (specified by origin
    // and normal), to the general equation of a 3D plane (which
    // looses information about the origin of the coordinate system)
    const Vector& normal = plane.GetNormal();
    const Vector& origin = plane.GetOrigin();
    double d = -(normal[0] * origin[0] + normal[1] * origin[1] + normal[2] * origin[2]);

    return HasIntersectionWithPlane(points, normal, d);
  }
  

  bool OrientedVolumeBoundingBox::Contains(const Vector& p) const
  {
    assert(p.size() == 3);

    const Vector q = p - c_;

    return (fabs(boost::numeric::ublas::inner_prod(q, u_)) <= hu_ &&
            fabs(boost::numeric::ublas::inner_prod(q, v_)) <= hv_ &&
            fabs(boost::numeric::ublas::inner_prod(q, w_)) <= hw_);
  }

  
  void OrientedVolumeBoundingBox::FromInternalCoordinates(Vector& target,
                                                          double x,
                                                          double y,
                                                          double z) const
  {
    target = (c_ +
              u_ * 2.0 * hu_ * (x - 0.5) +
              v_ * 2.0 * hv_ * (y - 0.5) +
              w_ * 2.0 * hw_ * (z - 0.5));
  }

  
  void OrientedVolumeBoundingBox::FromInternalCoordinates(Vector& target,
                                                          const Vector& source) const
  {
    assert(source.size() == 3);
    FromInternalCoordinates(target, source[0], source[1], source[2]);
  }

  
  void OrientedVolumeBoundingBox::ToInternalCoordinates(Vector& target,
                                                        const Vector& source) const
  {
    assert(source.size() == 3);
    const Vector q = source - c_;

    double x = boost::numeric::ublas::inner_prod(q, u_) / (2.0 * hu_) + 0.5;
    double y = boost::numeric::ublas::inner_prod(q, v_) / (2.0 * hv_) + 0.5;
    double z = boost::numeric::ublas::inner_prod(q, w_) / (2.0 * hw_) + 0.5;

    LinearAlgebra::AssignVector(target, x, y, z);
  }


  bool OrientedVolumeBoundingBox::ComputeExtent(Extent2D& extent,
                                                const CoordinateSystem3D& plane) const
  {
    extent.Clear();
    
    std::vector<Vector> points;
    if (HasIntersection(points, plane))
    {
      for (size_t i = 0; i < points.size(); i++)
      {
        double x, y;
        plane.ProjectPoint(x, y, points[i]);
        extent.AddPoint(x, y);
      }
      
      return true;
    }
    else
    {
      return false;
    }
  }
}

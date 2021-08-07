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

#ifdef BGO_ENABLE_DICOMSTRUCTURESETLOADER2

#include "CoordinateSystem3D.h"
#include "DicomStructureSetUtils.h"
#include "Extent2D.h"

#include "../Scene2D/Color.h"
#include "../StoneException.h"

#include "OrthancDatasets/FullOrthancDataset.h"

#include <list>
#include <string>

namespace OrthancStone
{

  /**
  Only polygons that are planar and parallel to either the X,Y or Z plane
  ("X plane" == plane where X is equal to a constant for each point) are
  supported.
  */
  class DicomStructurePolygon2
  {
  public:
    enum Type
    {
      ClosedPlanar,
      Unsupported
    };

    DicomStructurePolygon2(std::string referencedSopInstanceUid, const std::string& type)
      : referencedSopInstanceUid_(referencedSopInstanceUid)
      , state_(Building)
      , minX_(std::numeric_limits<double>::max())
      , maxX_(-std::numeric_limits<double>::max())
      , minY_(std::numeric_limits<double>::max())
      , maxY_(-std::numeric_limits<double>::max())
      , minZ_(std::numeric_limits<double>::max())
      , maxZ_(-std::numeric_limits<double>::max())
      , type_(TypeFromString(type))
    {
      ORTHANC_ASSERT(type_ == ClosedPlanar);
    }

    void ComputeDependentProperties();

    size_t GetPointCount() const
    {
      ORTHANC_ASSERT(state_ == Valid);
      return points_.size();
    }

    const Point3D& GetPoint(size_t i) const
    {
      ORTHANC_ASSERT(state_ == Valid);
      return points_.at(i);
    }

    void AddPoint(const Point3D& v)
    {
      ORTHANC_ASSERT(state_ == Building);
      points_.push_back(v);
    }

    void Reserve(size_t n)
    {
      ORTHANC_ASSERT(state_ == Building);
      points_.reserve(n);
    }

    /**
    This method takes a plane+coord system  that is parallel to the polygon
    and adds to polygons a new vector with the ordered set of points projected
    on the plane, in the plane coordinate system.
    */
    void ProjectOnParallelPlane(
      std::vector< std::pair<Point2D,Point2D> >& segments,
      const CoordinateSystem3D& plane) const;

    /**
    Returns the coordinates of the intersection of the polygon and a plane 
    that is perpendicular to the polygons (plane has either constant X or 
    constant Y)
    */
    void ProjectOnConstantPlane(
      std::vector<Point2D>& intersections,
      const CoordinateSystem3D& plane) const;

    /**
    This method assumes polygon has a normal equal to 0,0,-1 and 0,0,1 (thus,
    the polygon is parallel to the XY plane) and returns the Z coordinate of 
    all the polygon points
    */
    double GetZ() const;

    /**
    The normal sign is left undefined for now
    */
    Vector3D GetNormal() const
    {
      return normal_;
    }

    /**
    This method will compute the intersection between a polygon and
    a plane where either X, Y or Z is constant.
    The plane is given with an origin and a normal. If the normal is
    not parallel to an axis, an error is raised.
    */
    void ComputeIntersectionWithPlane(const CoordinateSystem3D& plane);

  private:
    static Type TypeFromString(const std::string& s)
    {
      if (s == "CLOSED_PLANAR")
        return ClosedPlanar;
      else
        return Unsupported;
    }
    enum State
    {
      Building,
      Valid
    };
    std::string           referencedSopInstanceUid_;
    CoordinateSystem3D    geometry_;
    std::vector<Point3D>  points_;
    Vector3D              normal_; // sign is irrelevant for now
    State                 state_;
    double                minX_, maxX_, minY_, maxY_, minZ_, maxZ_;
    Type                  type_;
  };
}

#endif
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2



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

#include "../Toolbox/CoordinateSystem3D.h"
#include "../Toolbox/Extent2D.h"
#include "../Toolbox/LinearAlgebra.h"
#include "VolumeImageGeometry.h"

namespace OrthancStone
{
  class OrientedVolumeBoundingBox : public boost::noncopyable
  {
  private:
    Vector  c_;   // center
    Vector  u_;   // normalized width vector
    Vector  v_;   // normalized height vector
    Vector  w_;   // normalized depth vector
    double  hu_;  // half width
    double  hv_;  // half height
    double  hw_;  // half depth

  public:
    explicit OrientedVolumeBoundingBox(const VolumeImageGeometry& geometry);

    const Vector& GetCenter() const
    {
      return c_;
    }

    bool HasIntersectionWithPlane(std::vector<Vector>& points,
                                  const Vector& normal,
                                  double d) const;

    bool HasIntersection(std::vector<Vector>& points,
                         const CoordinateSystem3D& plane) const;

    bool Contains(const Vector& p) const;

    void FromInternalCoordinates(Vector& target,
                                 double x,
                                 double y,
                                 double z) const;

    void FromInternalCoordinates(Vector& target,
                                 const Vector& source) const;

    void ToInternalCoordinates(Vector& target,
                               const Vector& source) const;

    bool ComputeExtent(Extent2D& extent,
                       const CoordinateSystem3D& plane) const;
  };
}


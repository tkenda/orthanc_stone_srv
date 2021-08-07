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

#include "../Toolbox/AffineTransform2D.h"

namespace OrthancStone
{
  class ScenePoint2D
  {
  private:
    double  x_;
    double  y_;

  public:
    ScenePoint2D() :
      x_(0),
      y_(0)
    {
    }

    ScenePoint2D(double x,
                 double y) :
      x_(x),
      y_(y)
    {
    }

    double GetX() const
    {
      return x_;
    }

    double GetY() const
    {
      return y_;
    }

    ScenePoint2D Apply(const AffineTransform2D& t) const;

    const ScenePoint2D operator-(const ScenePoint2D& a) const;

    const ScenePoint2D operator+(const ScenePoint2D& a) const;

    const ScenePoint2D operator*(double a) const;

    const ScenePoint2D operator/(double a) const;

    static void MidPoint(ScenePoint2D& result,
                         const ScenePoint2D& a,
                         const ScenePoint2D& b);

    static double Dot(const ScenePoint2D& a,
                      const ScenePoint2D& b);

    static double SquaredMagnitude(const ScenePoint2D& v);

    static double Magnitude(const ScenePoint2D& v);

    static double SquaredDistancePtPt(const ScenePoint2D& a,
                                      const ScenePoint2D& b);

    static double DistancePtPt(const ScenePoint2D& a,
                               const ScenePoint2D& b);

    // Distance from point p to [a,b] segment
    static double SquaredDistancePtSegment(const ScenePoint2D& a,
                                           const ScenePoint2D& b,
                                           const ScenePoint2D& p);
  };
}

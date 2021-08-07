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


#include "ScenePoint2D.h"


namespace OrthancStone
{
  ScenePoint2D ScenePoint2D::Apply(const AffineTransform2D& t) const
  {
    double x = x_;
    double y = y_;
    t.Apply(x, y);
    return ScenePoint2D(x, y);
  }

  
  const ScenePoint2D ScenePoint2D::operator-(const ScenePoint2D& a) const
  {
    ScenePoint2D v;
    v.x_ = x_ - a.x_;
    v.y_ = y_ - a.y_;
    return v;
  }

  
  const ScenePoint2D ScenePoint2D::operator+(const ScenePoint2D& a) const
  {
    ScenePoint2D v;
    v.x_ = x_ + a.x_;
    v.y_ = y_ + a.y_;
    return v;
  }

  
  const ScenePoint2D ScenePoint2D::operator*(double a) const
  {
    ScenePoint2D v;
    v.x_ = x_ * a;
    v.y_ = y_ * a;
    return v;
  }

  
  const ScenePoint2D ScenePoint2D::operator/(double a) const
  {
    ScenePoint2D v;
    v.x_ = x_ / a;
    v.y_ = y_ / a;
    return v;
  }


  void ScenePoint2D::MidPoint(ScenePoint2D& result,
                              const ScenePoint2D& a,
                              const ScenePoint2D& b)
  {
    result.x_ = 0.5 * (a.x_ + b.x_);
    result.y_ = 0.5 * (a.y_ + b.y_);
  }

  
  double ScenePoint2D::Dot(const ScenePoint2D& a, const ScenePoint2D& b)
  {
    return a.x_ * b.x_ + a.y_ * b.y_;
  }

  
  double ScenePoint2D::SquaredMagnitude(const ScenePoint2D& v)
  {
    return v.x_ * v.x_ + v.y_ * v.y_;
  }


  double ScenePoint2D::Magnitude(const ScenePoint2D& v)
  {
    double squaredMagnitude = SquaredMagnitude(v);
      
    if (LinearAlgebra::IsCloseToZero(squaredMagnitude))
    {
      return 0.0;
    }
    else
    {
      return sqrt(squaredMagnitude);
    }
  }


  double ScenePoint2D::SquaredDistancePtPt(const ScenePoint2D& a, const ScenePoint2D& b)
  {
    ScenePoint2D n = b - a;
    return Dot(n, n);
  }

  
  double ScenePoint2D::DistancePtPt(const ScenePoint2D& a, const ScenePoint2D& b)
  {
    double squaredDist = SquaredDistancePtPt(a, b);
    return sqrt(squaredDist);
  }


  double ScenePoint2D::SquaredDistancePtSegment(const ScenePoint2D& a, const ScenePoint2D& b, const ScenePoint2D& p)
  {
    // Rewritten from https://www.randygaul.net/2014/07/23/distance-point-to-line-segment/

    ScenePoint2D n = b - a;
    ScenePoint2D pa = a - p;

    double c = Dot(n, pa);

    // Closest point is a
    if (c > 0.0)
    {
      return Dot(pa, pa);
    }

    ScenePoint2D bp = p - b;

    // Closest point is b
    if (Dot(n, bp) > 0.0)
    {
      return Dot(bp, bp);
    }

    // if segment length is very short, we approximate distance to the
    // distance with a
    double nq = Dot(n, n);
    if (LinearAlgebra::IsCloseToZero(nq))
    {
      // segment is very small: approximate distance from point to segment
      // with distance from p to a
      return Dot(pa, pa);
    }
    else
    {
      // Closest point is between a and b
      ScenePoint2D e = pa - n * (c / nq);
      return Dot(e, e);
    }
  }
}

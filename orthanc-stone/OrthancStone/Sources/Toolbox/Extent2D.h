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

namespace OrthancStone
{
  class Extent2D
  {
  private:
    bool    empty_;
    double  x1_;
    double  y1_;
    double  x2_;
    double  y2_;

  public:
    Extent2D()
    {
      Clear();
    }

    Extent2D(double x1,
             double y1,
             double x2,
             double y2);

    void Clear();

    void AddPoint(double x,
                  double y);

    void Union(const Extent2D& other);

    bool IsEmpty() const;

    double GetX1() const
    {
      return x1_;
    }

    double GetY1() const
    {
      return y1_;
    }

    double GetX2() const
    {
      return x2_;
    }

    double GetY2() const
    {
      return y2_;
    }

    double GetWidth() const
    {
      return x2_ - x1_;
    }

    double GetHeight() const
    {
      return y2_ - y1_;
    }

    double GetCenterX() const
    {
      return (x1_ + x2_) / 2.0;
    }

    double GetCenterY() const
    {
      return (y1_ + y2_) / 2.0;
    }

    bool Contains(double x,
                  double y) const;
  };
}

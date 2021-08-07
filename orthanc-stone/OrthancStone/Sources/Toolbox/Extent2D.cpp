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


#include "Extent2D.h"

#include <algorithm>
#include <cassert>
#include <limits>

namespace OrthancStone
{
  Extent2D::Extent2D(double x1,
                     double y1,
                     double x2,
                     double y2) :
    empty_(false),
    x1_(x1),
    y1_(y1),
    x2_(x2),
    y2_(y2)
  {
    if (x1_ > x2_)
    {
      std::swap(x1_, x2_);
    }

    if (y1_ > y2_)
    {
      std::swap(y1_, y2_);
    }
  }


  void Extent2D::Clear()
  {
    empty_ = true;
    x1_ = 0;
    y1_ = 0;
    x2_ = 0;
    y2_ = 0;      
  }

  void Extent2D::AddPoint(double x,
                          double y)
  {
    if (empty_)
    {
      x1_ = x;
      y1_ = y;
      x2_ = x;
      y2_ = y;
      empty_ = false;
    }
    else
    {
      x1_ = std::min(x1_, x);
      y1_ = std::min(y1_, y);
      x2_ = std::max(x2_, x);
      y2_ = std::max(y2_, y);
    }

    assert(x1_ <= x2_ &&
           y1_ <= y2_);    // This is the invariant of the structure
  }


  void Extent2D::Union(const Extent2D& other)
  {
    if (other.empty_)
    {
      return;
    }

    if (empty_)
    {
      *this = other;
      return;
    }

    assert(!empty_);

    x1_ = std::min(x1_, other.x1_);
    y1_ = std::min(y1_, other.y1_);
    x2_ = std::max(x2_, other.x2_);
    y2_ = std::max(y2_, other.y2_);

    assert(x1_ <= x2_ &&
           y1_ <= y2_);    // This is the invariant of the structure
  }


  bool Extent2D::IsEmpty() const
  {
    if (empty_)
    {
      return true;
    }
    else
    {
      assert(x1_ <= x2_ &&
             y1_ <= y2_);
      return (x2_ <= x1_ + 10 * std::numeric_limits<double>::epsilon() ||
              y2_ <= y1_ + 10 * std::numeric_limits<double>::epsilon());
    }
  }


  bool Extent2D::Contains(double x,
                          double y) const
  {
    if (empty_)
    {
      return false;
    }
    else
    {
      return (x >= x1_ && x <= x2_ &&
              y >= y1_ && y <= y2_);
    }
  }
}

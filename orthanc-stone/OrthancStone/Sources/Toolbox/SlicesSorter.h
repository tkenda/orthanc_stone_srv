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

#include "CoordinateSystem3D.h"

#include <IDynamicObject.h>

namespace OrthancStone
{
  // TODO - Replace this with "SortedFrames"
  class SlicesSorter : public boost::noncopyable
  {
  private:
    class SliceWithDepth;
    struct Comparator;

    typedef std::vector<SliceWithDepth*>  Slices;

    Slices  slices_;
    bool    hasNormal_;
    
    const SliceWithDepth& GetSlice(size_t i) const;
    
    void SetNormal(const Vector& normal);
    
    void SortInternal();

    void FilterNormal(const Vector& normal);
    
    bool SelectNormal(Vector& normal) const;

  public:
    SlicesSorter() : hasNormal_(false)
    {
    }

    ~SlicesSorter();
    
    void Reserve(size_t count)
    {
      slices_.reserve(count);
    }

    void AddSlice(const CoordinateSystem3D& plane)
    {
      AddSlice(plane, NULL);
    }

    void AddSlice(const CoordinateSystem3D& plane,
                  Orthanc::IDynamicObject* payload);  // Takes ownership

    size_t GetSlicesCount() const
    {
      return slices_.size();
    }

    const CoordinateSystem3D& GetSliceGeometry(size_t i) const;

    bool HasSlicePayload(size_t i) const;
    
    const Orthanc::IDynamicObject& GetSlicePayload(size_t i) const;

    // WARNING - Apply the sorting algorithm can reduce the number of
    // slices. This is notably the case if all the slices are not
    // parallel to the reference normal that will be selected.
    bool Sort();

    // WARNING - The slices must have been sorted before calling this method
    bool ComputeSpacingBetweenSlices(double& spacing /* out */) const;

    // WARNING - The slices must have been sorted before calling this method
    bool AreAllSlicesDistinct() const;
  };
}

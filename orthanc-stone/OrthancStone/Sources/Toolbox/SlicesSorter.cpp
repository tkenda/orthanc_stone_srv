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


#include "SlicesSorter.h"

#include "GeometryToolbox.h"

#include <Logging.h>
#include <OrthancException.h>

namespace OrthancStone
{
  class SlicesSorter::SliceWithDepth : public boost::noncopyable
  {
  private:
    CoordinateSystem3D  geometry_;
    double              depth_;

    std::unique_ptr<Orthanc::IDynamicObject>   payload_;

  public:
    SliceWithDepth(const CoordinateSystem3D& geometry,
                   Orthanc::IDynamicObject* payload) :
      geometry_(geometry),
      depth_(0),
      payload_(payload)
    {
    }

    void SetNormal(const Vector& normal)
    {
      depth_ = boost::numeric::ublas::inner_prod(geometry_.GetOrigin(), normal);
    }

    double GetDepth() const
    {
      return depth_;
    }

    const CoordinateSystem3D& GetGeometry() const
    {
      return geometry_;
    }

    bool HasPayload() const
    {
      return (payload_.get() != NULL);
    }

    const Orthanc::IDynamicObject& GetPayload() const
    {
      if (HasPayload())
      {
        return *payload_;
      }
      else
      {
        LOG(ERROR) << "SlicesSorter::SliceWithDepth::GetPayload(): (!HasPayload())";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
    }
  };


  struct SlicesSorter::Comparator
  {
    bool operator() (const SliceWithDepth* const& a,
                     const SliceWithDepth* const& b) const
    {
      return a->GetDepth() < b->GetDepth();
    }
  };


  SlicesSorter::~SlicesSorter()
  {
    for (size_t i = 0; i < slices_.size(); i++)
    {
      assert(slices_[i] != NULL);
      delete slices_[i];
    }
  }


  void SlicesSorter::AddSlice(const CoordinateSystem3D& slice,
                              Orthanc::IDynamicObject* payload)
  {
    slices_.push_back(new SliceWithDepth(slice, payload));
  }

  
  const SlicesSorter::SliceWithDepth& SlicesSorter::GetSlice(size_t i) const
  {
    if (i >= slices_.size())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      assert(slices_[i] != NULL);
      return *slices_[i];
    }
  }


  const CoordinateSystem3D& SlicesSorter::GetSliceGeometry(size_t i) const
  {
    return GetSlice(i).GetGeometry();
  }
  
  
  bool SlicesSorter::HasSlicePayload(size_t i) const
  {
    return GetSlice(i).HasPayload();
  }
  
    
  const Orthanc::IDynamicObject& SlicesSorter::GetSlicePayload(size_t i) const
  {
    return GetSlice(i).GetPayload();
  }

  
  void SlicesSorter::SetNormal(const Vector& normal)
  {
    for (size_t i = 0; i < slices_.size(); i++)
    {
      slices_[i]->SetNormal(normal);
    }

    hasNormal_ = true;
  }
  
    
  void SlicesSorter::SortInternal()
  {
    if (!hasNormal_)
    {
      LOG(ERROR) << "SlicesSorter::SortInternal(): (!hasNormal_)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }

    Comparator comparator;
    std::sort(slices_.begin(), slices_.end(), comparator);
  }
  

  void SlicesSorter::FilterNormal(const Vector& normal)
  {
    size_t pos = 0;

    for (size_t i = 0; i < slices_.size(); i++)
    {
      if (GeometryToolbox::IsParallel(normal, slices_[i]->GetGeometry().GetNormal()))
      {
        // This slice is compatible with the selected normal
        slices_[pos] = slices_[i];
        pos += 1;
      }
      else
      {
        delete slices_[i];
        slices_[i] = NULL;
      }
    }

    slices_.resize(pos);
  }
  
    
  bool SlicesSorter::SelectNormal(Vector& normal) const
  {
    std::vector<Vector>  normalCandidates;
    std::vector<unsigned int>  normalCount;

    bool found = false;

    for (size_t i = 0; !found && i < GetSlicesCount(); i++)
    {
      const Vector& normal = GetSlice(i).GetGeometry().GetNormal();

      bool add = true;
      for (size_t j = 0; add && j < normalCandidates.size(); j++)  // (*)
      {
        if (GeometryToolbox::IsParallel(normal, normalCandidates[j]))
        {
          normalCount[j] += 1;
          add = false;
        }
      }

      if (add)
      {
        if (normalCount.size() > 2)
        {
          // To get linear-time complexity in (*). This heuristics
          // allows the series to have one single frame that is
          // not parallel to the others (such a frame could be a
          // generated preview)
          found = false;
        }
        else
        {
          normalCandidates.push_back(normal);
          normalCount.push_back(1);
        }
      }
    }

    for (size_t i = 0; !found && i < normalCandidates.size(); i++)
    {
      unsigned int count = normalCount[i];
      if (count == GetSlicesCount() ||
          count + 1 == GetSlicesCount())
      {
        normal = normalCandidates[i];
        found = true;
      }
    }

    return found;
  }


  bool SlicesSorter::Sort()
  {
    if (GetSlicesCount() > 0)
    {
      Vector normal;
      if (SelectNormal(normal))
      {
        FilterNormal(normal);
        SetNormal(normal);
        SortInternal();
        return true;
      }
    }

    return false;
  }


  bool SlicesSorter::ComputeSpacingBetweenSlices(double& spacing /* out */) const
  {
    if (GetSlicesCount() <= 1)
    {
      // This is a volume that is empty or that contains one single
      // slice: Choose a dummy z-dimension for voxels
      spacing = 1.0;
      return true;
    }
    
    const CoordinateSystem3D& reference = GetSliceGeometry(0);

    double referencePosition = reference.ProjectAlongNormal(reference.GetOrigin());
        
    double p = reference.ProjectAlongNormal(GetSliceGeometry(1).GetOrigin());
    spacing = p - referencePosition;

    if (spacing <= 0)
    {
      LOG(ERROR) << "SlicesSorter::ComputeSpacingBetweenSlices(): (spacing <= 0)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls,
                                      "Please call the Sort() method before");
    }

    for (size_t i = 1; i < GetSlicesCount(); i++)
    {
      Vector q = reference.GetOrigin() + spacing * static_cast<double>(i) * reference.GetNormal();
      double d = boost::numeric::ublas::norm_2(q - GetSliceGeometry(i).GetOrigin());

      if (!LinearAlgebra::IsNear(d, 0, 0.001 /* tolerance expressed in mm */))
      {
        return false;
      }
    }

    return true;
  }


  bool SlicesSorter::AreAllSlicesDistinct() const
  {
    if (GetSlicesCount() <= 1)
    {
      return true;
    }
    else
    {
      const CoordinateSystem3D& reference = GetSliceGeometry(0);
      double previousPosition = reference.ProjectAlongNormal(GetSliceGeometry(0).GetOrigin());
     
      for (size_t i = 1; i < GetSlicesCount(); i++)
      {
        double position = reference.ProjectAlongNormal(GetSliceGeometry(i).GetOrigin());

        if (LinearAlgebra::IsNear(position, previousPosition, 0.001 /* tolerance expressed in mm */))
        {
          return false;
        }

        previousPosition = position;
      }

      return true;
    }
  }
}

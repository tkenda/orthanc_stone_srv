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


#include "../Toolbox/SlicesSorter.h"
#include "SeriesOrderedFrames.h"

#include <Logging.h>
#include <OrthancException.h>

namespace OrthancStone
{
  class SeriesOrderedFrames::Instance : public boost::noncopyable
  {
  private:
    std::unique_ptr<Orthanc::DicomMap>  dicom_;
    DicomInstanceParameters           parameters_;

  public:
    explicit Instance(const Orthanc::DicomMap& dicom) :
      dicom_(dicom.Clone()),
      parameters_(dicom)
    {
    }
    
    const Orthanc::DicomMap& GetInstance() const
    {
      return *dicom_;
    }
    
    const DicomInstanceParameters& GetInstanceParameters() const
    {
      return parameters_;
    }

    bool Lookup3DGeometry(CoordinateSystem3D& target) const
    {
      try
      {
        std::string imagePositionPatient, imageOrientationPatient;
        if (dicom_->LookupStringValue(imagePositionPatient, Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, false) &&
            dicom_->LookupStringValue(imageOrientationPatient, Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, false))
        {
          target = CoordinateSystem3D(imagePositionPatient, imageOrientationPatient);
          return true;
        }
      }
      catch (Orthanc::OrthancException&)
      {
      }

      return false;
    }

    bool LookupIndexInSeries(int& target) const
    {
      std::string value;

      if (dicom_->LookupStringValue(value, Orthanc::DICOM_TAG_INSTANCE_NUMBER, false) ||
          dicom_->LookupStringValue(value, Orthanc::DICOM_TAG_IMAGE_INDEX, false))
      {
        try
        {
          target = boost::lexical_cast<int>(value);
          return true;
        }
        catch (boost::bad_lexical_cast&)
        {
        }
      }

      return false;
    }
  };


  class SeriesOrderedFrames::Frame : public boost::noncopyable
  {
  private:
    const Instance*  instance_;
    unsigned int     frameIndex_;

  public:
    Frame(const Instance& instance,
          unsigned int frameIndex) :
      instance_(&instance),
      frameIndex_(frameIndex)
    {
      if (frameIndex_ >= instance.GetInstanceParameters().GetImageInformation().GetNumberOfFrames())
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }
    }

    const Orthanc::DicomMap& GetInstance() const
    {
      assert(instance_ != NULL);
      return instance_->GetInstance();
    }
    
    const DicomInstanceParameters& GetInstanceParameters() const
    {
      assert(instance_ != NULL);
      return instance_->GetInstanceParameters();
    }

    unsigned int GetFrameIndex() const
    {
      return frameIndex_;
    }
  };


  class SeriesOrderedFrames::InstanceWithIndexInSeries
  {
  private:
    const Instance* instance_;  // Don't use a reference to make "std::sort()" happy
    int             index_;

  public:
    explicit InstanceWithIndexInSeries(const Instance& instance) :
      instance_(&instance)
    {
      if (!instance_->LookupIndexInSeries(index_))
      {
        index_ = std::numeric_limits<int>::max();
      }
    }

    const Instance& GetInstance() const
    {
      return *instance_;
    }

    int GetIndexInSeries() const
    {
      return index_;
    }

    bool operator< (const InstanceWithIndexInSeries& other) const
    {
      return (index_ < other.index_);
    }
  };


  void SeriesOrderedFrames::Clear()
  {
    for (size_t i = 0; i < instances_.size(); i++)
    {
      assert(instances_[i] != NULL);
      delete instances_[i];
    }

    for (size_t i = 0; i < orderedFrames_.size(); i++)
    {
      assert(orderedFrames_[i] != NULL);
      delete orderedFrames_[i];
    }

    instances_.clear();
    orderedFrames_.clear();
  }


  bool SeriesOrderedFrames::Sort3DVolume()
  {
    SlicesSorter sorter;
    sorter.Reserve(instances_.size());

    for (size_t i = 0; i < instances_.size(); i++)
    {
      CoordinateSystem3D geometry;
      if (instances_[i]->Lookup3DGeometry(geometry))
      {
        sorter.AddSlice(geometry, new Orthanc::SingleValueObject<Instance*>(instances_[i]));
      }
      else
      {
        return false;   // Not a 3D volume
      }
    }

    if (!sorter.Sort() ||
        sorter.GetSlicesCount() != instances_.size() ||
        !sorter.AreAllSlicesDistinct())
    {
      return false;
    }
    else
    {
      for (size_t i = 0; i < sorter.GetSlicesCount(); i++)
      {
        assert(sorter.HasSlicePayload(i));

        const Orthanc::SingleValueObject<Instance*>& payload =
          dynamic_cast<const Orthanc::SingleValueObject<Instance*>&>(sorter.GetSlicePayload(i));
              
        assert(payload.GetValue() != NULL);
              
        for (size_t j = 0; j < payload.GetValue()->GetInstanceParameters().GetImageInformation().GetNumberOfFrames(); j++)
        {
          orderedFrames_.push_back(new Frame(*payload.GetValue(), 
                                             static_cast<unsigned int>(j)));
        }
      }

      isRegular_ = sorter.ComputeSpacingBetweenSlices(spacingBetweenSlices_);
      return true;
    }
  }


  void SeriesOrderedFrames::SortIndexInSeries()
  {
    std::vector<InstanceWithIndexInSeries> tmp;
    tmp.reserve(instances_.size());      
        
    for (size_t i = 0; i < instances_.size(); i++)
    {
      assert(instances_[i] != NULL);
      tmp.push_back(InstanceWithIndexInSeries(*instances_[i]));
    }

    std::sort(tmp.begin(), tmp.end());

    for (size_t i = 0; i < tmp.size(); i++)
    {
      for (size_t j = 0; j < tmp[i].GetInstance().GetInstanceParameters().GetImageInformation().GetNumberOfFrames(); j++)
      {
        orderedFrames_.push_back(new Frame(tmp[i].GetInstance(), 
                                           static_cast<unsigned int>(j)));
      }
    }
  }


  const SeriesOrderedFrames::Frame& SeriesOrderedFrames::GetFrame(size_t seriesIndex) const
  {
    if (seriesIndex >= orderedFrames_.size())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      assert(orderedFrames_[seriesIndex] != NULL);
      return *(orderedFrames_[seriesIndex]);
    }
  }
  

  SeriesOrderedFrames::SeriesOrderedFrames(LoadedDicomResources& instances) :
    isVolume_(false),
    isRegular_(false),
    spacingBetweenSlices_(0)
  {
    instances_.reserve(instances.GetSize());

    size_t numberOfFrames = 0;
      
    for (size_t i = 0; i < instances.GetSize(); i++)
    {
      try
      {
        std::unique_ptr<Instance> instance(new Instance(instances.GetResource(i)));
        numberOfFrames += instance->GetInstanceParameters().GetImageInformation().GetNumberOfFrames();
        instances_.push_back(instance.release());
      }
      catch (Orthanc::OrthancException&)
      {
        // The instance has not all the required DICOM tags, skip it
      }
    }

    orderedFrames_.reserve(numberOfFrames);
      
    if (Sort3DVolume())
    {
      isVolume_ = true;

      if (isRegular_)
      {
        LOG(INFO) << "Regular 3D volume detected";
      }
      else
      {
        LOG(INFO) << "Non-regular 3D volume detected";
      }
    }
    else
    {
      LOG(INFO) << "Series is not a 3D volume, sorting by index";
      SortIndexInSeries();
    }

    LOG(INFO) << "Number of frames: " << orderedFrames_.size();
  }


  unsigned int SeriesOrderedFrames::GetFrameIndex(size_t seriesIndex) const
  {
    return GetFrame(seriesIndex).GetFrameIndex();
  }


  const Orthanc::DicomMap& SeriesOrderedFrames::GetInstance(size_t seriesIndex) const
  {
    return GetFrame(seriesIndex).GetInstance();
  }


  const DicomInstanceParameters& SeriesOrderedFrames::GetInstanceParameters(size_t seriesIndex) const
  {
    return GetFrame(seriesIndex).GetInstanceParameters();
  }


  double SeriesOrderedFrames::GetSpacingBetweenSlices() const
  {
    if (IsRegular3DVolume())
    {
      return spacingBetweenSlices_;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }
}

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


#include "SeriesMetadataLoader.h"

#include <DicomFormat/DicomInstanceHasher.h>

namespace OrthancStone
{
  SeriesMetadataLoader::SeriesMetadataLoader(boost::shared_ptr<DicomResourcesLoader>& loader) :
    loader_(loader),
    state_(State_Setup)
  {
  }


  bool SeriesMetadataLoader::IsScheduledWithHigherPriority(const std::string& seriesInstanceUid,
                                                           int priority) const
  {
    if (series_.find(seriesInstanceUid) != series_.end())
    {
      // This series is readily available
      return true;
    }
    else
    {
      std::map<std::string, int>::const_iterator found = scheduled_.find(seriesInstanceUid);

      return (found != scheduled_.end() &&
              found->second < priority);
    }
  }


  void SeriesMetadataLoader::Handle(const DicomResourcesLoader::SuccessMessage& message)
  {
    assert(message.GetResources());

    switch (state_)
    {
      case State_Setup:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);

      case State_Default:
      {
        std::string studyInstanceUid;
        std::string seriesInstanceUid;

        if (message.GetResources()->LookupTagValueConsensus(studyInstanceUid, Orthanc::DICOM_TAG_STUDY_INSTANCE_UID) &&
            message.GetResources()->LookupTagValueConsensus(seriesInstanceUid, Orthanc::DICOM_TAG_SERIES_INSTANCE_UID))
        {
          series_[seriesInstanceUid] = message.GetResources();

          SuccessMessage loadedMessage(*this, message.GetDicomSource(), studyInstanceUid,
                                       seriesInstanceUid, *message.GetResources());
          BroadcastMessage(loadedMessage);
        }

        break;
      }

      case State_DicomDir:
      {
        assert(!dicomDir_);
        assert(seriesSize_.empty());

        dicomDir_ = message.GetResources();
            
        for (size_t i = 0; i < message.GetResources()->GetSize(); i++)
        {
          std::string seriesInstanceUid;
          if (message.GetResources()->GetResource(i).LookupStringValue
              (seriesInstanceUid, Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, false))
          {
            boost::shared_ptr<LoadedDicomResources> target
              (new LoadedDicomResources(Orthanc::DICOM_TAG_SOP_INSTANCE_UID));

            if (loader_->ScheduleLoadDicomFile(target, message.GetPriority(), message.GetDicomSource(), dicomDirPath_, 
                                               message.GetResources()->GetResource(i), false /* no need for pixel data */,
                                               NULL /* TODO PAYLOAD */))
            {
              std::map<std::string, unsigned int>::iterator found = seriesSize_.find(seriesInstanceUid);
              if (found == seriesSize_.end())
              {
                series_[seriesInstanceUid].reset
                  (new LoadedDicomResources(Orthanc::DICOM_TAG_SOP_INSTANCE_UID));
                seriesSize_[seriesInstanceUid] = 1;
              }
              else
              {
                found->second ++;
              }
            }
          }
        }

        LOG(INFO) << "Read a DICOMDIR containing " << seriesSize_.size() << " series";            

        state_ = State_DicomFile;
        break;
      }

      case State_DicomFile:
      {
        assert(dicomDir_);
        assert(message.GetResources()->GetSize() <= 1);  // Could be zero if corrupted DICOM instance

        if (message.GetResources()->GetSize() == 1)
        {
          const Orthanc::DicomMap& instance = message.GetResources()->GetResource(0);

          std::string studyInstanceUid;
          std::string seriesInstanceUid;
          if (instance.LookupStringValue(studyInstanceUid, Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, false) &&
              instance.LookupStringValue(seriesInstanceUid, Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, false))
          {
            Series::const_iterator series = series_.find(seriesInstanceUid);
            std::map<std::string, unsigned int>::const_iterator size = seriesSize_.find(seriesInstanceUid);

            if (series == series_.end() ||
                size == seriesSize_.end())
            {
              throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
            }
            else
            {
              series->second->AddResource(instance);

              if (series->second->GetSize() > size->second)
              {
                throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
              }
              else if (series->second->GetSize() == size->second)
              {
                // The series is complete
                SuccessMessage loadedMessage(
                  *this, message.GetDicomSource(),
                  studyInstanceUid, seriesInstanceUid, *series->second);
                loadedMessage.SetDicomDir(dicomDirPath_, dicomDir_);
                BroadcastMessage(loadedMessage);
              }
            }
          }
        }

        break;
      }

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
  }


  SeriesMetadataLoader::SuccessMessage::SuccessMessage(
    const SeriesMetadataLoader& loader,
    const DicomSource& source,
    const std::string& studyInstanceUid,
    const std::string& seriesInstanceUid,
    LoadedDicomResources& instances) :
    OriginMessage(loader),
    source_(source),
    studyInstanceUid_(studyInstanceUid),
    seriesInstanceUid_(seriesInstanceUid),
    instances_(instances)
  {
    LOG(INFO) << "Loaded series " << seriesInstanceUid
              << ", number of instances: " << instances_.GetSize();
  }


  boost::shared_ptr<SeriesMetadataLoader> SeriesMetadataLoader::Create(const ILoadersContext::ILock& context)
  {
    boost::shared_ptr<DicomResourcesLoader> loader(DicomResourcesLoader::Create(context));
      
    boost::shared_ptr<SeriesMetadataLoader> obj(new SeriesMetadataLoader(loader));
    obj->Register<DicomResourcesLoader::SuccessMessage>(*loader, &SeriesMetadataLoader::Handle);
    return obj;
  }


  SeriesMetadataLoader::Accessor::Accessor(SeriesMetadataLoader& that,
                                           const std::string& seriesInstanceUid)
  {
    Series::const_iterator found = that.series_.find(seriesInstanceUid);
    if (found != that.series_.end())
    {
      assert(found->second != NULL);
      series_ = found->second;
    }
  }


  size_t SeriesMetadataLoader::Accessor::GetInstancesCount() const
  {
    if (IsComplete())
    {
      return series_->GetSize();
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  const Orthanc::DicomMap& SeriesMetadataLoader::Accessor::GetInstance(size_t index) const
  {
    if (IsComplete())
    {
      return series_->GetResource(index);
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }     
  }


  bool SeriesMetadataLoader::Accessor::LookupInstance(Orthanc::DicomMap& target,
                                                      const std::string& sopInstanceUid) const
  {
    if (IsComplete())
    {     
      if (series_->HasResource(sopInstanceUid))
      {
        target.Clear();
        series_->MergeResource(target, sopInstanceUid);
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }    
  }


  void SeriesMetadataLoader::ScheduleLoadSeries(int priority,
                                                const DicomSource& source,
                                                const std::string& studyInstanceUid,
                                                const std::string& seriesInstanceUid)
  {
    if (state_ != State_Setup &&
        state_ != State_Default)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls,
                                      "The loader is working in DICOMDIR state");
    }

    state_ = State_Default;

    // Only re-schedule the loading if the previous loading was with lower priority
    if (!IsScheduledWithHigherPriority(seriesInstanceUid, priority))
    {
      if (source.IsDicomWeb())
      {
        boost::shared_ptr<LoadedDicomResources> target
          (new LoadedDicomResources(Orthanc::DICOM_TAG_SOP_INSTANCE_UID));
        loader_->ScheduleGetDicomWeb(
          target, priority, source,
          "/studies/" + studyInstanceUid + "/series/" + seriesInstanceUid + "/metadata",
          NULL /* TODO PAYLOAD */);

        scheduled_[seriesInstanceUid] = priority;
      }
      else if (source.IsOrthanc())
      {
        // This flavor of the method is only available with DICOMweb, as
        // Orthanc requires the "PatientID" to be known
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls,
                                        "The PatientID must be provided on Orthanc sources");
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
      }
    }
  }


  void SeriesMetadataLoader::ScheduleLoadSeries(int priority,
                                                const DicomSource& source,
                                                const std::string& patientId,
                                                const std::string& studyInstanceUid,
                                                const std::string& seriesInstanceUid)
  {
    if (state_ != State_Setup &&
        state_ != State_Default)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls,
                                      "The loader is working in DICOMDIR state");
    }

    state_ = State_Default;

    if (source.IsDicomWeb())
    {
      ScheduleLoadSeries(priority, source, studyInstanceUid, seriesInstanceUid);
    }
    else if (!IsScheduledWithHigherPriority(seriesInstanceUid, priority))
    {
      if (source.IsOrthanc())
      {
        // Dummy SOP Instance UID, as we are working at the "series" level
        Orthanc::DicomInstanceHasher hasher(patientId, studyInstanceUid, seriesInstanceUid, "dummy");

        boost::shared_ptr<LoadedDicomResources> target
          (new LoadedDicomResources(Orthanc::DICOM_TAG_SOP_INSTANCE_UID));
        
        loader_->ScheduleLoadOrthancResources(target, priority, source, Orthanc::ResourceType_Series,
                                              hasher.HashSeries(), Orthanc::ResourceType_Instance,
                                              NULL /* TODO PAYLOAD */);

        scheduled_[seriesInstanceUid] = priority;
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
      }
    }
  }


  void SeriesMetadataLoader::ScheduleLoadDicomDir(int priority,
                                                  const DicomSource& source,
                                                  const std::string& path)
  {
    if (!source.IsDicomDir())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }

    if (state_ != State_Setup)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls,
                                      "The loader cannot load two different DICOMDIR");
    }

    state_ = State_DicomDir;
    dicomDirPath_ = path;
    boost::shared_ptr<LoadedDicomResources> dicomDir
      (new LoadedDicomResources(Orthanc::DICOM_TAG_REFERENCED_SOP_INSTANCE_UID_IN_FILE));
    loader_->ScheduleLoadDicomDir(dicomDir, priority, source, path,
                                  NULL /* TODO PAYLOAD */);
  }
}

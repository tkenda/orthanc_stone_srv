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

#include "DicomResourcesLoader.h"

namespace OrthancStone
{
  class SeriesMetadataLoader :
    public ObserverBase<SeriesMetadataLoader>,
    public IObservable
  {
  private:
    enum State
    {
      State_Setup,
      State_Default,
      State_DicomDir,
      State_DicomFile
    };

    typedef std::map<std::string, boost::shared_ptr<LoadedDicomResources> >  Series;

    boost::shared_ptr<DicomResourcesLoader>  loader_;
    State                                    state_;
    std::map<std::string, int>               scheduled_;   // Maps a "SeriesInstanceUID" to a priority
    Series                                   series_;
    boost::shared_ptr<LoadedDicomResources>  dicomDir_;
    std::string                              dicomDirPath_;
    std::map<std::string, unsigned int>      seriesSize_;

    explicit SeriesMetadataLoader(boost::shared_ptr<DicomResourcesLoader>& loader);

    bool IsScheduledWithHigherPriority(const std::string& seriesInstanceUid,
                                       int priority) const;

    void Handle(const DicomResourcesLoader::SuccessMessage& message);

  public:
    class SuccessMessage : public OriginMessage<SeriesMetadataLoader>
    {
      ORTHANC_STONE_MESSAGE(__FILE__, __LINE__);

    private:
      const DicomSource&     source_;
      const std::string&     studyInstanceUid_;
      const std::string&     seriesInstanceUid_;
      LoadedDicomResources&  instances_;
      std::string            dicomDirPath_;
      boost::shared_ptr<LoadedDicomResources>  dicomDir_;

    public:
      SuccessMessage(const SeriesMetadataLoader& loader,
                     const DicomSource& source,
                     const std::string& studyInstanceUid,
                     const std::string& seriesInstanceUid,
                     LoadedDicomResources& instances);

      const DicomSource& GetDicomSource() const
      {
        return source_;
      }
      
      const std::string& GetStudyInstanceUid() const
      {
        return studyInstanceUid_;
      }

      const std::string& GetSeriesInstanceUid() const
      {
        return seriesInstanceUid_;
      }

      size_t GetInstancesCount() const
      {
        return instances_.GetSize();
      }

      const Orthanc::DicomMap& GetInstance(size_t index) const
      {
        return instances_.GetResource(index);
      }

      LoadedDicomResources& GetInstances() const
      {
        return instances_;
      }

      void SetDicomDir(const std::string& dicomDirPath,
                       boost::shared_ptr<LoadedDicomResources> dicomDir)
      {
        dicomDirPath_ = dicomDirPath;
        dicomDir_ = dicomDir;
      }

      const std::string& GetDicomDirPath() const
      {
        return dicomDirPath_;
      }

      // Will be NULL on non-DICOMDIR sources
      boost::shared_ptr<LoadedDicomResources> GetDicomDir() const
      {
        return dicomDir_;
      }
    };

  
    class Factory : public ILoaderFactory
    {
    public:
      virtual boost::shared_ptr<IObserver> Create(ILoadersContext::ILock& context) ORTHANC_OVERRIDE
      {
        return SeriesMetadataLoader::Create(context);
      }
    };


    static boost::shared_ptr<SeriesMetadataLoader> Create(const ILoadersContext::ILock& context);

  
    class Accessor : public boost::noncopyable
    {
    private:
      boost::shared_ptr<LoadedDicomResources>  series_;

    public:
      Accessor(SeriesMetadataLoader& that,
               const std::string& seriesInstanceUid);

      bool IsComplete() const
      {
        return series_ != NULL;
      }

      size_t GetInstancesCount() const;

      const Orthanc::DicomMap& GetInstance(size_t index) const;

      bool LookupInstance(Orthanc::DicomMap& target,
                          const std::string& sopInstanceUid) const;
    };


    void ScheduleLoadSeries(int priority,
                            const DicomSource& source,
                            const std::string& studyInstanceUid,
                            const std::string& seriesInstanceUid);

    void ScheduleLoadSeries(int priority,
                            const DicomSource& source,
                            const std::string& patientId,
                            const std::string& studyInstanceUid,
                            const std::string& seriesInstanceUid);

    void ScheduleLoadDicomDir(int priority,
                              const DicomSource& source,
                              const std::string& path);
  };
}

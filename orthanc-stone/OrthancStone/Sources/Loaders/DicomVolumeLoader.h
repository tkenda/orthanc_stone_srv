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

#include "../Volumes/DicomVolumeImage.h"
#include "SeriesFramesLoader.h"
#include "SeriesMetadataLoader.h"

namespace OrthancStone
{
  class DicomVolumeLoader : 
    public ObserverBase<DicomVolumeLoader>,
    public IObservable
  {
  private:
    boost::shared_ptr<SeriesFramesLoader>  framesLoader_;
    boost::shared_ptr<DicomVolumeImage>    volume_;
    bool                                   isValid_;
    bool                                   started_;
    size_t                                 remaining_;

    DicomVolumeLoader(boost::shared_ptr<SeriesFramesLoader>& framesLoader,
                      bool computeRange);

    void Handle(const SeriesFramesLoader::FrameLoadedMessage& message);

  public:
    class VolumeReadyMessage : public OriginMessage<DicomVolumeLoader>
    {
      ORTHANC_STONE_MESSAGE(__FILE__, __LINE__);

    public:
      explicit VolumeReadyMessage(const DicomVolumeLoader& loader) :
        OriginMessage(loader)
      {
      }

      const DicomVolumeImage& GetVolume() const
      {
        assert(GetOrigin().GetVolume());
        return *GetOrigin().GetVolume();
      }
    };


    class VolumeUpdatedMessage : public OriginMessage<DicomVolumeLoader>
    {
      ORTHANC_STONE_MESSAGE(__FILE__, __LINE__);

    private:
      unsigned int   axial_;

    public:
      VolumeUpdatedMessage(const DicomVolumeLoader& loader,
                           unsigned int axial) :
        OriginMessage(loader),
        axial_(axial)
      {
      }

      unsigned int GetAxialIndex() const
      {
        return axial_;
      }

      const DicomVolumeImage& GetVolume() const
      {
        assert(GetOrigin().GetVolume());
        return *GetOrigin().GetVolume();
      }
    };


    class Factory : public ILoaderFactory
    {
    private:
      SeriesFramesLoader::Factory  framesFactory_;
      bool                         computeRange_;

    public:
      explicit Factory(LoadedDicomResources& instances);

      explicit Factory(const SeriesMetadataLoader::SuccessMessage& metadata);

      void SetComputeRange(bool computeRange)
      {
        computeRange_ = computeRange;
      }

      void SetDicomDir(const std::string& dicomDirPath,
                       boost::shared_ptr<LoadedDicomResources> dicomDir)
      {
        framesFactory_.SetDicomDir(dicomDirPath, dicomDir);
      }

      virtual boost::shared_ptr<IObserver> Create(ILoadersContext::ILock& context) ORTHANC_OVERRIDE;
    };

    bool IsValid() const
    {
      return isValid_;
    }

    bool IsFullyLoaded() const
    {
      return remaining_ == 0;
    }

    boost::shared_ptr<DicomVolumeImage> GetVolume() const
    {
      return volume_;
    }

    const SeriesOrderedFrames& GetOrderedFrames() const
    {
      return framesLoader_->GetOrderedFrames();
    }

    void Start(int priority,
               const DicomSource& source);
  };
}

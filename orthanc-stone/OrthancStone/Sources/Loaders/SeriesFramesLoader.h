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

#include "../OrthancStone.h"

#if !defined(ORTHANC_ENABLE_DCMTK)
#  error The macro ORTHANC_ENABLE_DCMTK must be defined
#endif

#include "OracleScheduler.h"
#include "DicomSource.h"
#include "SeriesOrderedFrames.h"
#include "ILoaderFactory.h"

namespace OrthancStone
{  
  class SeriesFramesLoader : 
    public ObserverBase<SeriesFramesLoader>,
    public IObservable
  {
  private:
    class Payload;

    ILoadersContext&                         context_;
    SeriesOrderedFrames                      frames_;
    std::string                              dicomDirPath_;
    boost::shared_ptr<LoadedDicomResources>  dicomDir_;

    SeriesFramesLoader(ILoadersContext& context,
                       LoadedDicomResources& instances,
                       const std::string& dicomDirPath,
                       boost::shared_ptr<LoadedDicomResources> dicomDir);

    void EmitMessage(const Payload& payload,
                     const Orthanc::ImageAccessor& image);

#if ORTHANC_ENABLE_DCMTK == 1
    void HandleDicom(const Payload& payload,
                     const Orthanc::ParsedDicomFile& dicom);
#endif
    
    void HandleDicomWebRendered(const Payload& payload,
                                const std::string& body,
                                const std::map<std::string, std::string>& headers);

#if ORTHANC_ENABLE_DCMTK == 1
    void Handle(const ParseDicomSuccessMessage& message);
#endif

    void Handle(const GetOrthancImageCommand::SuccessMessage& message);

    void Handle(const GetOrthancWebViewerJpegCommand::SuccessMessage& message);

    void Handle(const OrthancRestApiCommand::SuccessMessage& message);

    void Handle(const HttpCommand::SuccessMessage& message);

  public:
    class FrameLoadedMessage : public OriginMessage<SeriesFramesLoader>
    {
      ORTHANC_STONE_MESSAGE(__FILE__, __LINE__);

    private:
      size_t                          frameIndex_;
      unsigned int                    quality_;
      const Orthanc::ImageAccessor&   image_;
      const Orthanc::DicomMap&        instance_;
      const DicomInstanceParameters&  parameters_;
      Orthanc::IDynamicObject*        userPayload_; // Ownership is maintained by the caller

    public:
      FrameLoadedMessage(const SeriesFramesLoader& loader,
                         size_t frameIndex,
                         unsigned int quality,
                         const Orthanc::ImageAccessor& image,
                         const Orthanc::DicomMap& instance,
                         const DicomInstanceParameters&  parameters,
                         Orthanc::IDynamicObject* userPayload) :
        OriginMessage(loader),
        frameIndex_(frameIndex),
        quality_(quality),
        image_(image),
        instance_(instance),
        parameters_(parameters),
        userPayload_(userPayload)
      {
      }

      size_t GetFrameIndex() const
      {
        return frameIndex_;
      }

      unsigned int GetQuality() const
      {
        return quality_;
      }

      const Orthanc::ImageAccessor& GetImage() const
      {
        return image_;
      }

      const Orthanc::DicomMap& GetInstance() const
      {
        return instance_;
      }

      const DicomInstanceParameters& GetInstanceParameters() const
      {
        return parameters_;
      }

      bool HasUserPayload() const
      {
        return userPayload_ != NULL;
      }

      Orthanc::IDynamicObject& GetUserPayload() const;
    };


    class Factory : public ILoaderFactory
    {
    private:
      LoadedDicomResources&                    instances_;
      std::string                              dicomDirPath_;
      boost::shared_ptr<LoadedDicomResources>  dicomDir_;

    public:
      // No "const" because "LoadedDicomResources::GetResource()" will call "Flatten()"
      explicit Factory(LoadedDicomResources& instances) :
        instances_(instances)
      {
      }

      void SetDicomDir(const std::string& dicomDirPath,
                       boost::shared_ptr<LoadedDicomResources> dicomDir);

      virtual boost::shared_ptr<IObserver> Create(ILoadersContext::ILock& context) ORTHANC_OVERRIDE;
    };

    const SeriesOrderedFrames& GetOrderedFrames() const
    {
      return frames_;
    }

    void ScheduleLoadFrame(int priority,
                           const DicomSource& source,
                           size_t index,
                           unsigned int quality,
                           Orthanc::IDynamicObject* userPayload /* transfer ownership */);
  };
}

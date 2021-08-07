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


#include "SeriesFramesLoader.h"

#include "../Oracle/ParseDicomFromFileCommand.h"
#include "../Oracle/ParseDicomFromWadoCommand.h"

#if ORTHANC_ENABLE_DCMTK == 1
#  include <DicomParsing/ParsedDicomFile.h>
#endif

#include <DicomFormat/DicomInstanceHasher.h>
#include <Images/Image.h>
#include <Images/ImageProcessing.h>
#include <Images/JpegReader.h>

#include <boost/algorithm/string/predicate.hpp>

namespace OrthancStone
{  
  class SeriesFramesLoader::Payload : public Orthanc::IDynamicObject
  {
  private:
    DicomSource   source_;
    size_t        seriesIndex_;
    std::string   sopInstanceUid_;  // Only used for debug purpose
    unsigned int  quality_;
    bool          hasWindowing_;
    float         windowingCenter_;
    float         windowingWidth_;
    std::unique_ptr<Orthanc::IDynamicObject>  userPayload_;

  public:
    Payload(const DicomSource& source,
            size_t seriesIndex,
            const std::string& sopInstanceUid,
            unsigned int quality,
            Orthanc::IDynamicObject* userPayload) :
      source_(source),
      seriesIndex_(seriesIndex),
      sopInstanceUid_(sopInstanceUid),
      quality_(quality),
      hasWindowing_(false),
      windowingCenter_(0),
      windowingWidth_(0),
      userPayload_(userPayload)
    {
    }

    size_t GetSeriesIndex() const
    {
      return seriesIndex_;
    }

    const std::string& GetSopInstanceUid() const
    {
      return sopInstanceUid_;
    }

    unsigned int GetQuality() const
    {
      return quality_;
    }

    void SetWindowing(float center,
                      float width)
    {
      hasWindowing_ = true;
      windowingCenter_ = center;
      windowingWidth_ = width;
    }

    bool HasWindowing() const
    {
      return hasWindowing_;
    }

    float GetWindowingCenter() const
    {
      if (hasWindowing_)
      {
        return windowingCenter_;
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
    }

    float GetWindowingWidth() const
    {
      if (hasWindowing_)
      {
        return windowingWidth_;
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
    }

    const DicomSource& GetSource() const
    {
      return source_;
    }

    Orthanc::IDynamicObject* GetUserPayload() const
    {
      return userPayload_.get();
    }
  };
    

  SeriesFramesLoader::SeriesFramesLoader(ILoadersContext& context,
                                         LoadedDicomResources& instances,
                                         const std::string& dicomDirPath,
                                         boost::shared_ptr<LoadedDicomResources> dicomDir) :
    context_(context),
    frames_(instances),
    dicomDirPath_(dicomDirPath),
    dicomDir_(dicomDir)
  {
  }


  void SeriesFramesLoader::EmitMessage(const Payload& payload,
                                       const Orthanc::ImageAccessor& image)
  {
    const DicomInstanceParameters& parameters = frames_.GetInstanceParameters(payload.GetSeriesIndex());
    const Orthanc::DicomMap& instance = frames_.GetInstance(payload.GetSeriesIndex());
    size_t frameIndex = frames_.GetFrameIndex(payload.GetSeriesIndex());

    if (frameIndex >= parameters.GetImageInformation().GetNumberOfFrames() ||
        payload.GetSopInstanceUid() != parameters.GetSopInstanceUid())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }      

    LOG(TRACE) << "Decoded instance " << payload.GetSopInstanceUid() << ", frame "
               << frameIndex << ": " << image.GetWidth() << "x"
               << image.GetHeight() << ", " << Orthanc::EnumerationToString(image.GetFormat())
               << ", quality " << payload.GetQuality();
      
    FrameLoadedMessage message(*this, frameIndex, payload.GetQuality(), image, instance, parameters, payload.GetUserPayload());
    BroadcastMessage(message);
  }


#if ORTHANC_ENABLE_DCMTK == 1
  void SeriesFramesLoader::HandleDicom(const Payload& payload,
                                       const Orthanc::ParsedDicomFile& dicom)
  {     
    size_t frameIndex = frames_.GetFrameIndex(payload.GetSeriesIndex());

    std::unique_ptr<Orthanc::ImageAccessor> decoded;
    decoded.reset(dicom.DecodeFrame(static_cast<unsigned int>(frameIndex)));

    if (decoded.get() == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }

    EmitMessage(payload, *decoded);
  }
#endif

    
  void SeriesFramesLoader::HandleDicomWebRendered(const Payload& payload,
                                                  const std::string& body,
                                                  const std::map<std::string, std::string>& headers)
  {
    assert(payload.GetSource().IsDicomWeb() &&
           payload.HasWindowing());

    bool ok = false;
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it)
    {
      if (boost::iequals("content-type", it->first) &&
          boost::iequals(Orthanc::MIME_JPEG, it->second))
      {
        ok = true;
        break;
      }
    }

    if (!ok)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol,
                                      "The WADO-RS server has not generated a JPEG image on /rendered");
    }

    Orthanc::JpegReader reader;
    reader.ReadFromMemory(body);

    switch (reader.GetFormat())
    {
      case Orthanc::PixelFormat_RGB24:
        EmitMessage(payload, reader);
        break;

      case Orthanc::PixelFormat_Grayscale8:
      {
        const DicomInstanceParameters& parameters = frames_.GetInstanceParameters(payload.GetSeriesIndex());

        Orthanc::Image scaled(parameters.GetExpectedPixelFormat(), reader.GetWidth(), reader.GetHeight(), false);
        Orthanc::ImageProcessing::Convert(scaled, reader);
          
        float w = payload.GetWindowingWidth();
        if (w <= 0.01f)
        {
          w = 0.01f;  // Prevent division by zero
        }

        const float c = payload.GetWindowingCenter();
        const float scaling = w / 255.0f;
        const float offset = (c - w / 2.0f) / scaling;

        Orthanc::ImageProcessing::ShiftScale(scaled, offset, scaling, false /* truncation to speed up */);
        EmitMessage(payload, scaled);
        break;
      }

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }
  }


#if ORTHANC_ENABLE_DCMTK == 1
  void SeriesFramesLoader::Handle(const ParseDicomSuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());

    const Payload& payload = dynamic_cast<const Payload&>(message.GetOrigin().GetPayload());
    if ((payload.GetSource().IsDicomDir() ||
         payload.GetSource().IsDicomWeb()) &&
        message.HasPixelData())
    {
      HandleDicom(dynamic_cast<const Payload&>(message.GetOrigin().GetPayload()), message.GetDicom());
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
  }
#endif


  void SeriesFramesLoader::Handle(const GetOrthancImageCommand::SuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());

    const Payload& payload = dynamic_cast<const Payload&>(message.GetOrigin().GetPayload());
    assert(payload.GetSource().IsOrthanc());

    EmitMessage(payload, message.GetImage());
  }


  void SeriesFramesLoader::Handle(const GetOrthancWebViewerJpegCommand::SuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());

    const Payload& payload = dynamic_cast<const Payload&>(message.GetOrigin().GetPayload());
    assert(payload.GetSource().IsOrthanc());

    EmitMessage(payload, message.GetImage());
  }


  void SeriesFramesLoader::Handle(const OrthancRestApiCommand::SuccessMessage& message)
  {
    // This is to handle "/rendered" in DICOMweb
    assert(message.GetOrigin().HasPayload());
    HandleDicomWebRendered(dynamic_cast<const Payload&>(message.GetOrigin().GetPayload()),
                           message.GetAnswer(), message.GetAnswerHeaders());
  }


  void SeriesFramesLoader::Handle(const HttpCommand::SuccessMessage& message)
  {
    // This is to handle "/rendered" in DICOMweb
    assert(message.GetOrigin().HasPayload());
    HandleDicomWebRendered(dynamic_cast<const Payload&>(message.GetOrigin().GetPayload()),
                           message.GetAnswer(), message.GetAnswerHeaders());
  }


  Orthanc::IDynamicObject& SeriesFramesLoader::FrameLoadedMessage::GetUserPayload() const
  {
    if (userPayload_)
    {
      return *userPayload_;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  void SeriesFramesLoader::Factory::SetDicomDir(const std::string& dicomDirPath,
                                                boost::shared_ptr<LoadedDicomResources> dicomDir)
  {
    dicomDirPath_ = dicomDirPath;
    dicomDir_ = dicomDir;
  }


  boost::shared_ptr<IObserver> SeriesFramesLoader::Factory::Create(ILoadersContext::ILock& stone)
  {
    boost::shared_ptr<SeriesFramesLoader> loader(
      new SeriesFramesLoader(stone.GetContext(), instances_, dicomDirPath_, dicomDir_));
    loader->Register<GetOrthancImageCommand::SuccessMessage>(stone.GetOracleObservable(), &SeriesFramesLoader::Handle);
    loader->Register<GetOrthancWebViewerJpegCommand::SuccessMessage>(stone.GetOracleObservable(), &SeriesFramesLoader::Handle);
    loader->Register<HttpCommand::SuccessMessage>(stone.GetOracleObservable(), &SeriesFramesLoader::Handle);
    loader->Register<OrthancRestApiCommand::SuccessMessage>(stone.GetOracleObservable(), &SeriesFramesLoader::Handle);

#if ORTHANC_ENABLE_DCMTK == 1
    loader->Register<ParseDicomSuccessMessage>(stone.GetOracleObservable(), &SeriesFramesLoader::Handle);
#endif

    return loader;
  }


  void SeriesFramesLoader::ScheduleLoadFrame(int priority,
                                             const DicomSource& source,
                                             size_t index,
                                             unsigned int quality,
                                             Orthanc::IDynamicObject* userPayload)
  {
    std::unique_ptr<Orthanc::IDynamicObject> protection(userPayload);
    
    if (index >= frames_.GetFramesCount() ||
        quality >= source.GetQualityCount())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    const Orthanc::DicomMap& instance = frames_.GetInstance(index);

    std::string sopInstanceUid;
    if (!instance.LookupStringValue(sopInstanceUid, Orthanc::DICOM_TAG_SOP_INSTANCE_UID, false))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat,
                                      "Missing SOPInstanceUID in a DICOM instance");
    }
      
    if (source.IsDicomDir())
    {
      if (dicomDir_.get() == NULL)
      {
        // Should have been set in the factory
        throw Orthanc::OrthancException(
          Orthanc::ErrorCode_BadSequenceOfCalls,
          "SeriesFramesLoader::Factory::SetDicomDir() should have been called");
      }
        
      assert(quality == 0);
        
      std::string file;
      if (dicomDir_->LookupStringValue(file, sopInstanceUid, Orthanc::DICOM_TAG_REFERENCED_FILE_ID))
      {
        std::unique_ptr<ParseDicomFromFileCommand> command(new ParseDicomFromFileCommand(source, dicomDirPath_, file));
        command->SetPixelDataIncluded(true);
        command->AcquirePayload(new Payload(source, index, sopInstanceUid, quality, protection.release()));

        {
          std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
          lock->Schedule(GetSharedObserver(), priority, command.release());
        }
      }
      else
      {
        LOG(WARNING) << "Missing tag ReferencedFileID in a DICOMDIR entry";
      }
    }
    else if (source.IsDicomWeb())
    {
      std::string studyInstanceUid, seriesInstanceUid;
      if (!instance.LookupStringValue(studyInstanceUid, Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, false) ||
          !instance.LookupStringValue(seriesInstanceUid, Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, false))
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat,
                                        "Missing StudyInstanceUID or SeriesInstanceUID in a DICOM instance");
      }

      const std::string uri = ("/studies/" + studyInstanceUid +
                               "/series/" + seriesInstanceUid +
                               "/instances/" + sopInstanceUid);

      if (source.HasDicomWebRendered() &&
          quality == 0)
      {
        const DicomInstanceParameters& parameters = frames_.GetInstanceParameters(index);

        float c, w;
        parameters.GetWindowingPresetsUnion(c, w);

        std::map<std::string, std::string> arguments, headers;
        arguments["window"] = (boost::lexical_cast<std::string>(c) + "," +
                               boost::lexical_cast<std::string>(w) + ",linear");
        headers["Accept"] = "image/jpeg";

        std::unique_ptr<Payload> payload(new Payload(source, index, sopInstanceUid, quality, protection.release()));
        payload->SetWindowing(c, w);

        {
          std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
          lock->Schedule(GetSharedObserver(), priority,
                         source.CreateDicomWebCommand(uri + "/rendered", arguments, headers, payload.release()));
        }
      }
      else
      {
        assert((source.HasDicomWebRendered() && quality == 1) ||
               (!source.HasDicomWebRendered() && quality == 0));

#if ORTHANC_ENABLE_DCMTK == 1
        std::unique_ptr<Payload> payload(new Payload(source, index, sopInstanceUid, quality, protection.release()));

        const std::map<std::string, std::string> empty;

        std::unique_ptr<ParseDicomFromWadoCommand> command(
          new ParseDicomFromWadoCommand(source, sopInstanceUid, false /* no server-side transcoding */,
                                        Orthanc::DicomTransferSyntax_LittleEndianExplicit /* dummy value */,
                                        source.CreateDicomWebCommand(uri, empty, empty, NULL)));
        command->AcquirePayload(payload.release());

        {
          std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
          lock->Schedule(GetSharedObserver(), priority, command.release());
        }
#else
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented,
                                        "DCMTK is not enabled, cannot parse a DICOM instance");
#endif
      }
    }
    else if (source.IsOrthanc())
    {
      std::string orthancId;

      {
        std::string patientId, studyInstanceUid, seriesInstanceUid;
        if (!instance.LookupStringValue(patientId, Orthanc::DICOM_TAG_PATIENT_ID, false) ||
            !instance.LookupStringValue(studyInstanceUid, Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, false) ||
            !instance.LookupStringValue(seriesInstanceUid, Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, false))
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat,
                                          "Missing StudyInstanceUID or SeriesInstanceUID in a DICOM instance");
        }

        Orthanc::DicomInstanceHasher hasher(patientId, studyInstanceUid, seriesInstanceUid, sopInstanceUid);
        orthancId = hasher.HashInstance();
      }

      const DicomInstanceParameters& parameters = frames_.GetInstanceParameters(index);

      if (quality == 0 && source.HasOrthancWebViewer1())
      {
        std::unique_ptr<GetOrthancWebViewerJpegCommand> command(new GetOrthancWebViewerJpegCommand);
        command->SetInstance(orthancId);
        command->SetExpectedPixelFormat(parameters.GetExpectedPixelFormat());
        command->AcquirePayload(new Payload(source, index, sopInstanceUid, quality, protection.release()));

        {
          std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
          lock->Schedule(GetSharedObserver(), priority, command.release());
        }
      }
      else if (quality == 0 && source.HasOrthancAdvancedPreview())
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
      }
      else
      {
        assert(quality <= 1);
        assert(quality == 0 || 
               source.HasOrthancWebViewer1() || 
               source.HasOrthancAdvancedPreview());

        std::unique_ptr<GetOrthancImageCommand> command(new GetOrthancImageCommand);
        command->SetFrameUri(orthancId, frames_.GetFrameIndex(index), parameters.GetExpectedPixelFormat());
        command->SetExpectedPixelFormat(parameters.GetExpectedPixelFormat());
        command->SetHttpHeader("Accept", Orthanc::MIME_PAM);
        command->AcquirePayload(new Payload(source, index, sopInstanceUid, quality, protection.release()));

        {
          std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
          lock->Schedule(GetSharedObserver(), priority, command.release());
        }
      }
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }
  }
}

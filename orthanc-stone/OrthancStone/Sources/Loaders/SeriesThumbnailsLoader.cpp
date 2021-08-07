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


#include "SeriesThumbnailsLoader.h"

#include "LoadedDicomResources.h"
#include "../Oracle/ParseDicomFromWadoCommand.h"
#include "../Toolbox/ImageToolbox.h"

#include <DicomFormat/DicomMap.h>
#include <DicomFormat/DicomInstanceHasher.h>
#include <Images/Image.h>
#include <Images/ImageProcessing.h>
#include <Images/JpegReader.h>
#include <Images/JpegWriter.h>
#include <OrthancException.h>
#include <Toolbox.h>

#include <boost/algorithm/string/predicate.hpp>

#if ORTHANC_ENABLE_DCMTK == 1
#  include <DicomParsing/ParsedDicomFile.h>
#endif 


static const unsigned int JPEG_QUALITY = 70;  // Only used for Orthanc source

namespace OrthancStone
{
  SeriesThumbnailsLoader::Thumbnail::Thumbnail(const std::string& image,
                                               const std::string& mime) :
    type_(SeriesThumbnailType_Image),
    image_(image),
    mime_(mime)
  {
  }


  SeriesThumbnailsLoader::Thumbnail::Thumbnail(SeriesThumbnailType type) :
    type_(type)
  {
    if (type == SeriesThumbnailType_Image)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  Orthanc::ImageAccessor* SeriesThumbnailsLoader::SuccessMessage::DecodeImage() const
  {
    if (GetType() != SeriesThumbnailType_Image)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }

    Orthanc::MimeType mime;
    if (!Orthanc::LookupMimeType(mime, GetMime()))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented,
                                      "Unsupported MIME type for thumbnail: " + GetMime());
    }

    switch (mime)
    {
      case Orthanc::MimeType_Jpeg:
      {
        std::unique_ptr<Orthanc::JpegReader> reader(new Orthanc::JpegReader);
        reader->ReadFromMemory(GetEncodedImage());
        return reader.release();
      }

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented,
                                        "Cannot decode MIME type for thumbnail: " + GetMime());
    }
  }



  void SeriesThumbnailsLoader::AcquireThumbnail(const DicomSource& source,
                                                const std::string& studyInstanceUid,
                                                const std::string& seriesInstanceUid,
                                                SeriesThumbnailsLoader::Thumbnail* thumbnail)
  {
    assert(thumbnail != NULL);
  
    std::unique_ptr<Thumbnail> protection(thumbnail);

    Thumbnails::iterator found = thumbnails_.find(seriesInstanceUid);
    if (found == thumbnails_.end())
    {
      thumbnails_[seriesInstanceUid] = protection.release();
    }
    else
    {
      assert(found->second != NULL);
      if (protection->GetType() == SeriesThumbnailType_NotLoaded ||
          protection->GetType() == SeriesThumbnailType_Unsupported)
      {
        // Don't replace an old entry if the current one is worse
        return;
      }
      else
      {
        delete found->second;
        found->second = protection.release();
      }
    }

    LOG(INFO) << "Thumbnail updated for series: " << seriesInstanceUid << ": " << thumbnail->GetType();
    
    SuccessMessage message(*this, source, studyInstanceUid, seriesInstanceUid, *thumbnail);
    BroadcastMessage(message);
  }


  class SeriesThumbnailsLoader::Handler : public Orthanc::IDynamicObject
  {
  private:
    boost::shared_ptr<SeriesThumbnailsLoader>  loader_;
    DicomSource                                source_;
    std::string                                studyInstanceUid_;
    std::string                                seriesInstanceUid_;

  public:
    Handler(boost::shared_ptr<SeriesThumbnailsLoader> loader,
            const DicomSource& source,
            const std::string& studyInstanceUid,
            const std::string& seriesInstanceUid) :
      loader_(loader),
      source_(source),
      studyInstanceUid_(studyInstanceUid),
      seriesInstanceUid_(seriesInstanceUid)
    {
      if (!loader)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }
    }

    boost::shared_ptr<SeriesThumbnailsLoader> GetLoader()
    {
      return loader_;
    }

    const DicomSource& GetSource() const
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

    virtual void HandleSuccess(const std::string& body,
                               const std::map<std::string, std::string>& headers) = 0;

    virtual void HandleError()
    {
      LOG(INFO) << "Cannot generate thumbnail for SeriesInstanceUID: " << seriesInstanceUid_;
    }
  };


  class SeriesThumbnailsLoader::DicomWebSopClassHandler : public SeriesThumbnailsLoader::Handler
  {
  private:
    static bool GetSopClassUid(std::string& sopClassUid,
                               const Json::Value& json)
    {
      Orthanc::DicomMap dicom;
      dicom.FromDicomWeb(json);

      return dicom.LookupStringValue(sopClassUid, Orthanc::DICOM_TAG_SOP_CLASS_UID, false);
    }
  
  public:
    DicomWebSopClassHandler(boost::shared_ptr<SeriesThumbnailsLoader> loader,
                            const DicomSource& source,
                            const std::string& studyInstanceUid,
                            const std::string& seriesInstanceUid) :
      Handler(loader, source, studyInstanceUid, seriesInstanceUid)
    {
    }

    virtual void HandleSuccess(const std::string& body,
                               const std::map<std::string, std::string>& headers) ORTHANC_OVERRIDE
    {
      Json::Value value;

      if (!Orthanc::Toolbox::ReadJson(value, body) ||
          value.type() != Json::arrayValue)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
      }
      else
      {
        SeriesThumbnailType type = SeriesThumbnailType_Unsupported;

        std::string sopClassUid;
        if (value.size() > 0 &&
            GetSopClassUid(sopClassUid, value[0]))
        {
          bool ok = true;
        
          for (Json::Value::ArrayIndex i = 1; i < value.size() && ok; i++)
          {
            std::string s;
            if (!GetSopClassUid(s, value[i]) ||
                s != sopClassUid)
            {
              ok = false;
            }
          }

          if (ok)
          {
            type = GetSeriesThumbnailType(StringToSopClassUid(sopClassUid));
          }
        }

        GetLoader()->AcquireThumbnail(GetSource(), GetStudyInstanceUid(),
                                      GetSeriesInstanceUid(), new Thumbnail(type));
      }
    }
  };


  class SeriesThumbnailsLoader::DicomWebThumbnailHandler : public SeriesThumbnailsLoader::Handler
  {
  public:
    DicomWebThumbnailHandler(boost::shared_ptr<SeriesThumbnailsLoader> loader,
                             const DicomSource& source,
                             const std::string& studyInstanceUid,
                             const std::string& seriesInstanceUid) :
      Handler(loader, source, studyInstanceUid, seriesInstanceUid)
    {
    }

    virtual void HandleSuccess(const std::string& body,
                               const std::map<std::string, std::string>& headers) ORTHANC_OVERRIDE
    {
      std::string mime = Orthanc::MIME_JPEG;
      for (std::map<std::string, std::string>::const_iterator
             it = headers.begin(); it != headers.end(); ++it)
      {
        if (boost::iequals(it->first, "content-type"))
        {
          mime = it->second;
        }
      }

      GetLoader()->AcquireThumbnail(GetSource(), GetStudyInstanceUid(),
                                    GetSeriesInstanceUid(), new Thumbnail(body, mime));
    }

    virtual void HandleError() ORTHANC_OVERRIDE
    {
      // The DICOMweb wasn't able to generate a thumbnail, try to
      // retrieve the SopClassUID tag using QIDO-RS
      
      std::map<std::string, std::string> arguments, headers;
      arguments["0020000D"] = GetStudyInstanceUid();
      arguments["0020000E"] = GetSeriesInstanceUid();
      arguments["includefield"] = "00080016";  // SOP Class UID
      
      std::unique_ptr<IOracleCommand> command(
        GetSource().CreateDicomWebCommand(
          "/instances", arguments, headers, new DicomWebSopClassHandler(
            GetLoader(), GetSource(), GetStudyInstanceUid(), GetSeriesInstanceUid())));
      GetLoader()->Schedule(command.release());
    }
  };


  class SeriesThumbnailsLoader::ThumbnailInformation : public Orthanc::IDynamicObject
  {
  private:
    DicomSource  source_;
    std::string  studyInstanceUid_;
    std::string  seriesInstanceUid_;

  public:
    ThumbnailInformation(const DicomSource& source,
                         const std::string& studyInstanceUid,
                         const std::string& seriesInstanceUid) :
      source_(source),
      studyInstanceUid_(studyInstanceUid),
      seriesInstanceUid_(seriesInstanceUid)
    {
    }

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
  };


  class SeriesThumbnailsLoader::OrthancSopClassHandler : public SeriesThumbnailsLoader::Handler
  {
  private:
    std::string instanceId_;
      
  public:
    OrthancSopClassHandler(boost::shared_ptr<SeriesThumbnailsLoader> loader,
                           const DicomSource& source,
                           const std::string& studyInstanceUid,
                           const std::string& seriesInstanceUid,
                           const std::string& instanceId) :
      Handler(loader, source, studyInstanceUid, seriesInstanceUid),
      instanceId_(instanceId)
    {
    }

    virtual void HandleSuccess(const std::string& body,
                               const std::map<std::string, std::string>& headers) ORTHANC_OVERRIDE
    {
      SeriesThumbnailType type = GetSeriesThumbnailType(StringToSopClassUid(body));

      if (type == SeriesThumbnailType_Pdf ||
          type == SeriesThumbnailType_Video)
      {
        GetLoader()->AcquireThumbnail(GetSource(), GetStudyInstanceUid(),
                                      GetSeriesInstanceUid(), new Thumbnail(type));
      }
      else
      {
        std::unique_ptr<GetOrthancImageCommand> command(new GetOrthancImageCommand);
        command->SetUri("/instances/" + instanceId_ + "/preview");
        command->SetHttpHeader("Accept", Orthanc::MIME_JPEG);
        command->AcquirePayload(new ThumbnailInformation(
                                  GetSource(), GetStudyInstanceUid(), GetSeriesInstanceUid()));
        GetLoader()->Schedule(command.release());
      }
    }
  };


  class SeriesThumbnailsLoader::SelectOrthancInstanceHandler : public SeriesThumbnailsLoader::Handler
  {
  public:
    SelectOrthancInstanceHandler(boost::shared_ptr<SeriesThumbnailsLoader> loader,
                                 const DicomSource& source,
                                 const std::string& studyInstanceUid,
                                 const std::string& seriesInstanceUid) :
      Handler(loader, source, studyInstanceUid, seriesInstanceUid)
    {
    }

    virtual void HandleSuccess(const std::string& body,
                               const std::map<std::string, std::string>& headers) ORTHANC_OVERRIDE
    {
      static const char* const INSTANCES = "Instances";
      
      Json::Value json;
      if (!Orthanc::Toolbox::ReadJson(json, body) ||
          json.type() != Json::objectValue)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
      }

      if (json.isMember(INSTANCES) &&
          json[INSTANCES].type() == Json::arrayValue &&
          json[INSTANCES].size() > 0)
      {
        // Select one instance of the series to generate the thumbnail
        Json::Value::ArrayIndex index = json[INSTANCES].size() / 2;
        if (json[INSTANCES][index].type() == Json::stringValue)
        {
          const std::string instance = json[INSTANCES][index].asString();

          std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
          command->SetUri("/instances/" + instance + "/metadata/SopClassUid");
          command->AcquirePayload(
            new OrthancSopClassHandler(
              GetLoader(), GetSource(), GetStudyInstanceUid(), GetSeriesInstanceUid(), instance));
          GetLoader()->Schedule(command.release());
        }
      }
    }
  };

    
#if ORTHANC_ENABLE_DCMTK == 1
  class SeriesThumbnailsLoader::SelectDicomWebInstanceHandler : public SeriesThumbnailsLoader::Handler
  {
  public:
    SelectDicomWebInstanceHandler(boost::shared_ptr<SeriesThumbnailsLoader> loader,
                                  const DicomSource& source,
                                  const std::string& studyInstanceUid,
                                  const std::string& seriesInstanceUid) :
      Handler(loader, source, studyInstanceUid, seriesInstanceUid)
    {
    }

    virtual void HandleSuccess(const std::string& body,
                               const std::map<std::string, std::string>& headers) ORTHANC_OVERRIDE
    {
      Json::Value json;
      if (!Orthanc::Toolbox::ReadJson(json, body) ||
          json.type() != Json::arrayValue)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
      }

      LoadedDicomResources instances(Orthanc::DICOM_TAG_SOP_INSTANCE_UID);
      instances.AddFromDicomWeb(json);
      
      std::string sopInstanceUid;      
      if (instances.GetSize() == 0 ||
          !instances.GetResource(0).LookupStringValue(sopInstanceUid, Orthanc::DICOM_TAG_SOP_INSTANCE_UID, false))
      {
        LOG(ERROR) << "Series without an instance: " << GetSeriesInstanceUid();
      }
      else
      {
        GetLoader()->Schedule(
          ParseDicomFromWadoCommand::Create(
            GetSource(), GetStudyInstanceUid(), GetSeriesInstanceUid(), sopInstanceUid, false,
            Orthanc::DicomTransferSyntax_LittleEndianExplicit /* useless, as no transcoding */,
            new ThumbnailInformation(
              GetSource(), GetStudyInstanceUid(), GetSeriesInstanceUid())));
      }
    }
  };
#endif

    
  void SeriesThumbnailsLoader::Schedule(IOracleCommand* command)
  {
    std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
    lock->Schedule(GetSharedObserver(), priority_, command);
  }    

  
  void SeriesThumbnailsLoader::Handle(const HttpCommand::SuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());
    dynamic_cast<Handler&>(message.GetOrigin().GetPayload()).HandleSuccess(message.GetAnswer(), message.GetAnswerHeaders());
  }


  void SeriesThumbnailsLoader::Handle(const OrthancRestApiCommand::SuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());
    dynamic_cast<Handler&>(message.GetOrigin().GetPayload()).HandleSuccess(message.GetAnswer(), message.GetAnswerHeaders());
  }


  void SeriesThumbnailsLoader::Handle(const GetOrthancImageCommand::SuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());
    const ThumbnailInformation& info = dynamic_cast<ThumbnailInformation&>(message.GetOrigin().GetPayload());

    std::unique_ptr<Orthanc::ImageAccessor> resized(Orthanc::ImageProcessing::FitSize(message.GetImage(), width_, height_));

    std::string jpeg;
    Orthanc::JpegWriter writer;
    writer.SetQuality(JPEG_QUALITY);
    Orthanc::IImageWriter::WriteToMemory(writer, jpeg, *resized);

    AcquireThumbnail(info.GetDicomSource(), info.GetStudyInstanceUid(),
                     info.GetSeriesInstanceUid(), new Thumbnail(jpeg, Orthanc::MIME_JPEG));      
  }


#if ORTHANC_ENABLE_DCMTK == 1
  void SeriesThumbnailsLoader::Handle(const ParseDicomSuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());
    const ParseDicomFromWadoCommand& origin =
      dynamic_cast<const ParseDicomFromWadoCommand&>(message.GetOrigin());
    const ThumbnailInformation& info = dynamic_cast<ThumbnailInformation&>(origin.GetPayload());

    Orthanc::DicomTransferSyntax transferSyntax;
    if (!message.GetDicom().LookupTransferSyntax(transferSyntax))
    {      
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat,
                                      "DICOM instance without a transfer syntax: " + origin.GetSopInstanceUid());
    }
    else if (!ImageToolbox::IsDecodingSupported(transferSyntax))
    {
      LOG(INFO) << "Asking the DICOMweb server to transcode, "
                << "as I don't support this transfer syntax: " << Orthanc::GetTransferSyntaxUid(transferSyntax);

      Schedule(ParseDicomFromWadoCommand::Create(
                 origin.GetSource(), info.GetStudyInstanceUid(), info.GetSeriesInstanceUid(),
                 origin.GetSopInstanceUid(), true, Orthanc::DicomTransferSyntax_LittleEndianExplicit,
                 new ThumbnailInformation(
                   origin.GetSource(), info.GetStudyInstanceUid(), info.GetSeriesInstanceUid())));
    }
    else
    {
      std::unique_ptr<Orthanc::ImageAccessor> frame(message.GetDicom().DecodeFrame(0));

      std::unique_ptr<Orthanc::ImageAccessor> thumbnail;

      if (frame->GetFormat() == Orthanc::PixelFormat_RGB24)
      {
        thumbnail.reset(Orthanc::ImageProcessing::FitSizeKeepAspectRatio(*frame, width_, height_));
      }
      else
      {
        std::unique_ptr<Orthanc::ImageAccessor> converted(
          new Orthanc::Image(Orthanc::PixelFormat_Float32, frame->GetWidth(), frame->GetHeight(), false));
        Orthanc::ImageProcessing::Convert(*converted, *frame);

        std::unique_ptr<Orthanc::ImageAccessor> resized(
          Orthanc::ImageProcessing::FitSizeKeepAspectRatio(*converted, width_, height_));
      
        float minValue, maxValue;
        Orthanc::ImageProcessing::GetMinMaxFloatValue(minValue, maxValue, *resized);
        if (minValue + 0.01f < maxValue)
        {
          Orthanc::ImageProcessing::ShiftScale(*resized, -minValue, 255.0f / (maxValue - minValue), false);
        }
        else
        {
          Orthanc::ImageProcessing::Set(*resized, 0);
        }

        converted.reset(NULL);

        thumbnail.reset(new Orthanc::Image(Orthanc::PixelFormat_Grayscale8, width_, height_, false));
        Orthanc::ImageProcessing::Convert(*thumbnail, *resized);
      }
    
      std::string jpeg;
      Orthanc::JpegWriter writer;
      writer.SetQuality(JPEG_QUALITY);
      Orthanc::IImageWriter::WriteToMemory(writer, jpeg, *thumbnail);

      AcquireThumbnail(info.GetDicomSource(), info.GetStudyInstanceUid(),
                       info.GetSeriesInstanceUid(), new Thumbnail(jpeg, Orthanc::MIME_JPEG));
    }
  }
#endif


  void SeriesThumbnailsLoader::Handle(const OracleCommandExceptionMessage& message)
  {
    const OracleCommandBase& command = dynamic_cast<const OracleCommandBase&>(message.GetOrigin());
    assert(command.HasPayload());

    if (command.GetType() == IOracleCommand::Type_GetOrthancImage)
    {
      // This is presumably a HTTP status 301 (Moved permanently)
      // because of an unsupported DICOM file in "/preview"
      const ThumbnailInformation& info = dynamic_cast<const ThumbnailInformation&>(command.GetPayload());
      AcquireThumbnail(info.GetDicomSource(), info.GetStudyInstanceUid(),
                       info.GetSeriesInstanceUid(), new Thumbnail(SeriesThumbnailType_Unsupported));
    }
    else
    {
      dynamic_cast<Handler&>(command.GetPayload()).HandleError();
    }
  }


  SeriesThumbnailsLoader::SeriesThumbnailsLoader(ILoadersContext& context,
                                                 int priority) :
    context_(context),
    priority_(priority),
    width_(128),
    height_(128)
  {
  }
    
  
  boost::shared_ptr<SeriesThumbnailsLoader> SeriesThumbnailsLoader::Create(
    const ILoadersContext::ILock& stone,
    int priority)
  {
    boost::shared_ptr<SeriesThumbnailsLoader> result(new SeriesThumbnailsLoader(stone.GetContext(), priority));
    result->Register<GetOrthancImageCommand::SuccessMessage>(stone.GetOracleObservable(), &SeriesThumbnailsLoader::Handle);
    result->Register<HttpCommand::SuccessMessage>(stone.GetOracleObservable(), &SeriesThumbnailsLoader::Handle);
    result->Register<OracleCommandExceptionMessage>(stone.GetOracleObservable(), &SeriesThumbnailsLoader::Handle);
    result->Register<OrthancRestApiCommand::SuccessMessage>(stone.GetOracleObservable(), &SeriesThumbnailsLoader::Handle);

#if ORTHANC_ENABLE_DCMTK == 1
    result->Register<ParseDicomSuccessMessage>(stone.GetOracleObservable(), &SeriesThumbnailsLoader::Handle);
#endif
    
    return result;
  }


  void SeriesThumbnailsLoader::SetThumbnailSize(unsigned int width,
                                                unsigned int height)
  {
    if (width == 0 ||
        height == 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      width_ = width;
      height_ = height;
    }
  }

    
  void SeriesThumbnailsLoader::Clear()
  {
    for (Thumbnails::iterator it = thumbnails_.begin(); it != thumbnails_.end(); ++it)
    {
      assert(it->second != NULL);
      delete it->second;
    }

    thumbnails_.clear();
  }

    
  SeriesThumbnailType SeriesThumbnailsLoader::GetSeriesThumbnail(std::string& image,
                                                                 std::string& mime,
                                                                 const std::string& seriesInstanceUid) const
  {
    Thumbnails::const_iterator found = thumbnails_.find(seriesInstanceUid);

    if (found == thumbnails_.end())
    {
      return SeriesThumbnailType_NotLoaded;
    }
    else
    {
      assert(found->second != NULL);
      image.assign(found->second->GetImage());
      mime.assign(found->second->GetMime());
      return found->second->GetType();
    }
  }


  void SeriesThumbnailsLoader::ScheduleLoadThumbnail(const DicomSource& source,
                                                     const std::string& patientId,
                                                     const std::string& studyInstanceUid,
                                                     const std::string& seriesInstanceUid)
  {
    if (IsScheduledSeries(seriesInstanceUid))
    {
      return;
    }

    if (source.IsDicomWeb())
    {
      if (!source.HasDicomWebRendered())
      {
#if ORTHANC_ENABLE_DCMTK == 1
        // Issue a QIDO-RS request to select one of the instances in the series
        std::map<std::string, std::string> arguments, headers;
        arguments["0020000D"] = studyInstanceUid;
        arguments["0020000E"] = seriesInstanceUid;
        arguments["includefield"] = "00080018";  // SOP Instance UID is mandatory
        
        std::unique_ptr<IOracleCommand> command(
          source.CreateDicomWebCommand(
            "/instances", arguments, headers, new SelectDicomWebInstanceHandler(
              GetSharedObserver(), source, studyInstanceUid, seriesInstanceUid)));
        Schedule(command.release());
#else
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented,
                                        "Stone of Orthanc was built without support to decode DICOM images");
#endif
      }
      else
      {
        const std::string uri = ("/studies/" + studyInstanceUid +
                                 "/series/" + seriesInstanceUid + "/rendered");

        std::map<std::string, std::string> arguments, headers;
        arguments["viewport"] = (boost::lexical_cast<std::string>(width_) + "," +
                                 boost::lexical_cast<std::string>(height_));

        // Needed to set this header explicitly, as long as emscripten
        // does not include macro "EMSCRIPTEN_FETCH_RESPONSE_HEADERS"
        // https://github.com/emscripten-core/emscripten/pull/8486
        headers["Accept"] = Orthanc::MIME_JPEG;

        std::unique_ptr<IOracleCommand> command(
          source.CreateDicomWebCommand(
            uri, arguments, headers, new DicomWebThumbnailHandler(
              GetSharedObserver(), source, studyInstanceUid, seriesInstanceUid)));
        Schedule(command.release());
      }

      scheduledSeries_.insert(seriesInstanceUid);
    }
    else if (source.IsOrthanc())
    {
      // Dummy SOP Instance UID, as we are working at the "series" level
      Orthanc::DicomInstanceHasher hasher(patientId, studyInstanceUid, seriesInstanceUid, "dummy");

      std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
      command->SetUri("/series/" + hasher.HashSeries());
      command->AcquirePayload(new SelectOrthancInstanceHandler(
                                GetSharedObserver(), source, studyInstanceUid, seriesInstanceUid));
      Schedule(command.release());

      scheduledSeries_.insert(seriesInstanceUid);
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented,
                                      "Can only load thumbnails from Orthanc or DICOMweb");
    }
  }
}

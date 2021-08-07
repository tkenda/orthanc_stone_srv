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


#include "GetOrthancWebViewerJpegCommand.h"

#include "../Toolbox/LinearAlgebra.h"

#include <Images/Image.h>
#include <Images/ImageProcessing.h>
#include <Images/JpegReader.h>
#include <OrthancException.h>
#include <Toolbox.h>

#include <json/value.h>

namespace OrthancStone
{
  GetOrthancWebViewerJpegCommand::GetOrthancWebViewerJpegCommand() :
    frame_(0),
    quality_(95),
    timeout_(600),
    expectedFormat_(Orthanc::PixelFormat_Grayscale8)
  {
  }


  void GetOrthancWebViewerJpegCommand::SetQuality(unsigned int quality)
  {
    if (quality == 0 ||
        quality > 100)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      quality_ = quality;
    }
  }


  std::string GetOrthancWebViewerJpegCommand::GetUri() const
  {
    return ("/web-viewer/instances/jpeg" + boost::lexical_cast<std::string>(quality_) +
            "-" + instanceId_ + "_" + boost::lexical_cast<std::string>(frame_));
  }


  void GetOrthancWebViewerJpegCommand::ProcessHttpAnswer(boost::weak_ptr<IObserver> receiver,
                                                         IMessageEmitter& emitter,
                                                         const std::string& answer) const
  {
    // This code comes from older "OrthancSlicesLoader::ParseSliceImageJpeg()"
      
    Json::Value encoded;
    if (!Orthanc::Toolbox::ReadJson(encoded, answer))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }

    if (encoded.type() != Json::objectValue ||
        !encoded.isMember("Orthanc") ||
        encoded["Orthanc"].type() != Json::objectValue)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
    
    const Json::Value& info = encoded["Orthanc"];
    if (!info.isMember("PixelData") ||
        !info.isMember("Stretched") ||
        !info.isMember("Compression") ||
        info["Compression"].type() != Json::stringValue ||
        info["PixelData"].type() != Json::stringValue ||
        info["Stretched"].type() != Json::booleanValue ||
        info["Compression"].asString() != "Jpeg")
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
    
    bool isSigned = false;
    bool isStretched = info["Stretched"].asBool();
    
    if (info.isMember("IsSigned"))
    {
      if (info["IsSigned"].type() != Json::booleanValue)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }
      else
      {
        isSigned = info["IsSigned"].asBool();
      }
    }
    
    std::unique_ptr<Orthanc::ImageAccessor> reader;
    
    {
      std::string jpeg;
      Orthanc::Toolbox::DecodeBase64(jpeg, info["PixelData"].asString());
      
      reader.reset(new Orthanc::JpegReader);
      dynamic_cast<Orthanc::JpegReader&>(*reader).ReadFromMemory(jpeg);
    }
    
    if (reader->GetFormat() == Orthanc::PixelFormat_RGB24)  // This is a color image
    {
      if (expectedFormat_ != Orthanc::PixelFormat_RGB24)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }
      
      if (isSigned || isStretched)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }
      else
      {
        SuccessMessage message(*this, *reader);
        emitter.EmitMessage(receiver, message);
        return;
      }
    }
    
    if (reader->GetFormat() != Orthanc::PixelFormat_Grayscale8)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
    
    if (!isStretched)
    {
      if (expectedFormat_ != reader->GetFormat())
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }
      else
      {
        SuccessMessage message(*this, *reader);
        emitter.EmitMessage(receiver, message);
        return;
      }
    }
    
    int32_t stretchLow = 0;
    int32_t stretchHigh = 0;
    
    if (!info.isMember("StretchLow") ||
        !info.isMember("StretchHigh") ||
        info["StretchLow"].type() != Json::intValue ||
        info["StretchHigh"].type() != Json::intValue)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
    
    stretchLow = info["StretchLow"].asInt();
    stretchHigh = info["StretchHigh"].asInt();
    
    if (stretchLow < -32768 ||
        stretchHigh > 65535 ||
        (stretchLow < 0 && stretchHigh > 32767))
    {
      // This range cannot be represented with a uint16_t or an int16_t
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
    
    // Decode a grayscale JPEG 8bpp image coming from the Web viewer
    std::unique_ptr<Orthanc::ImageAccessor> image
      (new Orthanc::Image(expectedFormat_, reader->GetWidth(), reader->GetHeight(), false));

    Orthanc::ImageProcessing::Convert(*image, *reader);
    reader.reset();
    
    float scaling = static_cast<float>(stretchHigh - stretchLow) / 255.0f;
    
    if (!LinearAlgebra::IsCloseToZero(scaling))
    {
      float offset = static_cast<float>(stretchLow) / scaling;
      Orthanc::ImageProcessing::ShiftScale(*image, offset, scaling, true);
    }

    SuccessMessage message(*this, *image);
    emitter.EmitMessage(receiver, message);
  }
}

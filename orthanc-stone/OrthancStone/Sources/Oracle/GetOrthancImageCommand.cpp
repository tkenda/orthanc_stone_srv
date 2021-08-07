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


#include "GetOrthancImageCommand.h"

#include <Images/JpegReader.h>
#include <Images/PamReader.h>
#include <Images/PngReader.h>
#include <OrthancException.h>
#include <Toolbox.h>

namespace OrthancStone
{
  GetOrthancImageCommand::GetOrthancImageCommand() :
    uri_("/"),
    timeout_(600),
    hasExpectedFormat_(false),
    expectedFormat_(Orthanc::PixelFormat_Grayscale8)
  {
  }


  void GetOrthancImageCommand::SetExpectedPixelFormat(Orthanc::PixelFormat format)
  {
    hasExpectedFormat_ = true;
    expectedFormat_ = format;
  }


  static std::string GetFormatSuffix(Orthanc::PixelFormat pixelFormat)
  {
    switch (pixelFormat)
    {
      case Orthanc::PixelFormat_RGB24:
        return "preview";
      
      case Orthanc::PixelFormat_Grayscale16:
        return "image-uint16";
      
      case Orthanc::PixelFormat_SignedGrayscale16:
        return "image-int16";
      
      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  void GetOrthancImageCommand::SetInstanceUri(const std::string& instance,
                                              Orthanc::PixelFormat pixelFormat)
  {
    uri_ = "/instances/" + instance + "/" + GetFormatSuffix(pixelFormat);
  }


  void GetOrthancImageCommand::SetFrameUri(const std::string& instance,
                                           unsigned int frame,
                                           Orthanc::PixelFormat pixelFormat)
  {
    uri_ = ("/instances/" + instance + "/frames/" +
            boost::lexical_cast<std::string>(frame) + "/" + GetFormatSuffix(pixelFormat));
  }


  void GetOrthancImageCommand::ProcessHttpAnswer(boost::weak_ptr<IObserver> receiver,
                                                 IMessageEmitter& emitter,
                                                 const std::string& answer,
                                                 const HttpHeaders& answerHeaders) const
  {
    for (HttpHeaders::const_iterator it = answerHeaders.begin(); it != answerHeaders.end(); ++it)
    {
      std::string key = Orthanc::Toolbox::StripSpaces(it->first);
      Orthanc::Toolbox::ToLowerCase(key);
        
      if (key == "content-disposition" &&
          it->second == "filename=\"unsupported.png\"")
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat,
                                        "Orthanc cannot decode this image");
      }
    }
    
    Orthanc::MimeType contentType = Orthanc::MimeType_Binary;

    for (HttpHeaders::const_iterator it = answerHeaders.begin(); 
         it != answerHeaders.end(); ++it)
    {
      std::string s;
      Orthanc::Toolbox::ToLowerCase(s, it->first);

      if (s == "content-type")
      {
        contentType = Orthanc::StringToMimeType(it->second);
        break;
      }
    }

    std::unique_ptr<Orthanc::ImageAccessor> image;

    switch (contentType)
    {
      case Orthanc::MimeType_Png:
      {
        image.reset(new Orthanc::PngReader);
        dynamic_cast<Orthanc::PngReader&>(*image).ReadFromMemory(answer);
        break;
      }

      case Orthanc::MimeType_Pam:
      {
#ifdef __EMSCRIPTEN__
        // "true" means we ask the PamReader to make an extra copy so that
        // the resulting Orthanc::ImageAccessor is aligned (as malloc is).
        // Indeed, even though alignment is not required in Web Assembly,
        // Emscripten seems to check it and bail out if addresses are "odd"
        image.reset(new Orthanc::PamReader(true));
#else
        // potentially unaligned, with is faster and consumes less heap memory
        image.reset(new Orthanc::PamReader(false));
#endif
        dynamic_cast<Orthanc::PamReader&>(*image).ReadFromMemory(answer);
        break;
      }

      case Orthanc::MimeType_Jpeg:
      {
        image.reset(new Orthanc::JpegReader);
        dynamic_cast<Orthanc::JpegReader&>(*image).ReadFromMemory(answer);
        break;
      }

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol,
                                        "Unsupported HTTP Content-Type for an image: " + 
                                        std::string(Orthanc::EnumerationToString(contentType)));
    }

    if (hasExpectedFormat_)
    {
      if (expectedFormat_ == Orthanc::PixelFormat_SignedGrayscale16 &&
          image->GetFormat() == Orthanc::PixelFormat_Grayscale16)
      {
        image->SetFormat(Orthanc::PixelFormat_SignedGrayscale16);
      }

      if (expectedFormat_ != image->GetFormat())
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
      }
    }

    //{
    //  // DEBUG DISPLAY IMAGE PROPERTIES BGO 2020-04-11
    //  const Orthanc::ImageAccessor& source = *image;
    //  const void* sourceBuffer = source.GetConstBuffer();
    //  intptr_t sourceBufferInt = reinterpret_cast<intptr_t>(sourceBuffer);
    //  int sourceWidth = source.GetWidth();
    //  int sourceHeight = source.GetHeight();
    //  int sourcePitch = source.GetPitch();

    //  // TODO: turn error into trace below
    //  LOG(ERROR) << "GetOrthancImageCommand::ProcessHttpAnswer | source:"
    //    << " W = " << sourceWidth << " H = " << sourceHeight
    //    << " P = " << sourcePitch << " B = " << sourceBufferInt
    //    << " B % 4 == " << sourceBufferInt % 4;
    //}


    SuccessMessage message(*this, *image, contentType);
    emitter.EmitMessage(receiver, message);
  }
}

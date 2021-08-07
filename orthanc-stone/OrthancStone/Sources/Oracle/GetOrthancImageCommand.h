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

#include "../Messages/IMessageEmitter.h"
#include "OracleCommandBase.h"

#include <Images/ImageAccessor.h>

#include <map>

namespace OrthancStone
{
  class GetOrthancImageCommand : public OracleCommandBase
  {
  public:
    typedef std::map<std::string, std::string>  HttpHeaders;

    class SuccessMessage : public OriginMessage<GetOrthancImageCommand>
    {
      ORTHANC_STONE_MESSAGE(__FILE__, __LINE__);
      
    private:
      const Orthanc::ImageAccessor&  image_;
      Orthanc::MimeType              mime_;

    public:
      SuccessMessage(const GetOrthancImageCommand& command,
                     const Orthanc::ImageAccessor& image,
                     Orthanc::MimeType mime) :
        OriginMessage(command),
        image_(image),
        mime_(mime)
      {
      }

      const Orthanc::ImageAccessor& GetImage() const
      {
        return image_;
      }

      Orthanc::MimeType GetMimeType() const
      {
        return mime_;
      }
    };


  private:
    std::string           uri_;
    HttpHeaders           headers_;
    unsigned int          timeout_;
    bool                  hasExpectedFormat_;
    Orthanc::PixelFormat  expectedFormat_;

    GetOrthancImageCommand(const GetOrthancImageCommand& other) :
      uri_(other.uri_),
      headers_(other.headers_),
      timeout_(other.timeout_),
      hasExpectedFormat_(other.hasExpectedFormat_),
      expectedFormat_(other.expectedFormat_)
    {
    }

  public:
    GetOrthancImageCommand();

    virtual Type GetType() const ORTHANC_OVERRIDE
    {
      return Type_GetOrthancImage;
    }

    virtual IOracleCommand* Clone() const ORTHANC_OVERRIDE
    {
      return new GetOrthancImageCommand(*this);
    }

    void SetExpectedPixelFormat(Orthanc::PixelFormat format);

    void SetUri(const std::string& uri)
    {
      uri_ = uri;
    }

    void SetInstanceUri(const std::string& instance,
                        Orthanc::PixelFormat pixelFormat);

    void SetFrameUri(const std::string& instance,
                     unsigned int frame,
                     Orthanc::PixelFormat pixelFormat);

    void SetHttpHeader(const std::string& key,
                       const std::string& value)
    {
      headers_[key] = value;
    }

    const std::string& GetUri() const
    {
      return uri_;
    }

    const HttpHeaders& GetHttpHeaders() const
    {
      return headers_;
    }

    void SetTimeout(unsigned int seconds)
    {
      timeout_ = seconds;
    }

    unsigned int GetTimeout() const
    {
      return timeout_;
    }

    void ProcessHttpAnswer(boost::weak_ptr<IObserver> receiver,
                           IMessageEmitter& emitter,
                           const std::string& answer,
                           const HttpHeaders& answerHeaders) const;
  };
}

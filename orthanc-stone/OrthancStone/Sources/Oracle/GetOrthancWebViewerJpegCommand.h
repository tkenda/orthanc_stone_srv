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
  class GetOrthancWebViewerJpegCommand : public OracleCommandBase
  {
  public:
    typedef std::map<std::string, std::string>  HttpHeaders;

    class SuccessMessage : public OriginMessage<GetOrthancWebViewerJpegCommand>
    {
      ORTHANC_STONE_MESSAGE(__FILE__, __LINE__);
      
    private:
      const Orthanc::ImageAccessor&  image_;

    public:
      SuccessMessage(const GetOrthancWebViewerJpegCommand& command,
                     const Orthanc::ImageAccessor& image) :
        OriginMessage(command),
        image_(image)
      {
      }

      const Orthanc::ImageAccessor& GetImage() const
      {
        return image_;
      }
    };

  private:
    std::string           instanceId_;
    unsigned int          frame_;
    unsigned int          quality_;
    HttpHeaders           headers_;
    unsigned int          timeout_;
    Orthanc::PixelFormat  expectedFormat_;

    GetOrthancWebViewerJpegCommand(const GetOrthancWebViewerJpegCommand& other) :
      instanceId_(other.instanceId_),
      frame_(other.frame_),
      quality_(other.quality_),
      headers_(other.headers_),
      timeout_(other.timeout_),
      expectedFormat_(other.expectedFormat_)
    {
    }
    
  public:
    GetOrthancWebViewerJpegCommand();

    virtual Type GetType() const ORTHANC_OVERRIDE
    {
      return Type_GetOrthancWebViewerJpeg;
    }

    virtual IOracleCommand* Clone() const ORTHANC_OVERRIDE
    {
      return new GetOrthancWebViewerJpegCommand(*this);
    }

    void SetExpectedPixelFormat(Orthanc::PixelFormat format)
    {
      expectedFormat_ = format;
    }

    void SetInstance(const std::string& instanceId)
    {
      instanceId_ = instanceId;
    }

    void SetFrame(unsigned int frame)
    {
      frame_ = frame;
    }

    void SetQuality(unsigned int quality);

    void SetHttpHeader(const std::string& key,
                       const std::string& value)
    {
      headers_[key] = value;
    }

    Orthanc::PixelFormat GetExpectedPixelFormat() const
    {
      return expectedFormat_;
    }

    const std::string& GetInstanceId() const
    {
      return instanceId_;
    }

    unsigned int GetFrame() const
    {
      return frame_;
    }

    unsigned int GetQuality() const
    {
      return quality_;
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

    std::string GetUri() const;

    void ProcessHttpAnswer(boost::weak_ptr<IObserver> receiver,
                           IMessageEmitter& emitter,
                           const std::string& answer) const;
  };
}

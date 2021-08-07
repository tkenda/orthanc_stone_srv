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

#include "../Messages/IMessage.h"
#include "OracleCommandBase.h"

#include <Enumerations.h>

#include <map>
#include <json/value.h>

namespace OrthancStone
{
  class HttpCommand : public OracleCommandBase
  {
  public:
    typedef std::map<std::string, std::string>  HttpHeaders;

    class SuccessMessage : public OriginMessage<HttpCommand>
    {
      ORTHANC_STONE_MESSAGE(__FILE__, __LINE__);
      
    private:
      const HttpHeaders&  headers_;
      const std::string&  answer_;

    public:
      SuccessMessage(const HttpCommand& command,
                     const HttpHeaders& answerHeaders,
                     const std::string& answer) :
        OriginMessage(command),
        headers_(answerHeaders),
        answer_(answer)
      {
      }

      const std::string& GetAnswer() const
      {
        return answer_;
      }

      void ParseJsonBody(Json::Value& target) const;

      const HttpHeaders& GetAnswerHeaders() const
      {
        return headers_;
      }
    };


  private:
    Orthanc::HttpMethod  method_;
    std::string          url_;
    std::string          body_;
    HttpHeaders          headers_;
    unsigned int         timeout_;
    std::string          username_;
    std::string          password_;

    HttpCommand(const HttpCommand& other) :
      method_(other.method_),
      url_(other.url_),
      body_(other.body_),
      headers_(other.headers_),
      timeout_(other.timeout_),
      username_(other.username_),
      password_(other.password_)
    {
    }

  public:
    HttpCommand();

    virtual Type GetType() const ORTHANC_OVERRIDE
    {
      return Type_Http;
    }

    virtual IOracleCommand* Clone() const ORTHANC_OVERRIDE
    {
      return new HttpCommand(*this);
    }

    void SetMethod(Orthanc::HttpMethod method)
    {
      method_ = method;
    }

    void SetUrl(const std::string& url)
    {
      url_ = url;
    }

    void SetBody(const std::string& body)
    {
      body_ = body;
    }

    void SetBody(const Json::Value& json);

    void SwapBody(std::string& body)
    {
      body_.swap(body);
    }

    void SetHttpHeaders(const HttpHeaders& headers)
    {
      headers_ = headers;
    }

    void SetHttpHeader(const std::string& key,
                       const std::string& value)
    {
      headers_[key] = value;
    }

    Orthanc::HttpMethod GetMethod() const
    {
      return method_;
    }

    const std::string& GetUrl() const
    {
      return url_;
    }

    const std::string& GetBody() const;

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

    void SetCredentials(const std::string& username,
                        const std::string& password)
    {
      username_ = username;
      password_ = password;
    }

    void ClearCredentials()
    {
      username_.clear();
      password_.clear();
    }

    bool HasCredentials() const
    {
      return !username_.empty();
    }

    const std::string& GetUsername() const;

    const std::string& GetPassword() const;
  };
}

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


#include "HttpCommand.h"

#include <Logging.h>
#include <OrthancException.h>
#include <Toolbox.h>


namespace OrthancStone
{
  void HttpCommand::SuccessMessage::ParseJsonBody(Json::Value& target) const
  {
    if (!Orthanc::Toolbox::ReadJson(target, answer_))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
  }


  HttpCommand::HttpCommand() :
    method_(Orthanc::HttpMethod_Get),
    url_("/"),
    timeout_(600)
  {
  }


  void HttpCommand::SetBody(const Json::Value& json)
  {
    Orthanc::Toolbox::WriteFastJson(body_, json);
  }


  const std::string& HttpCommand::GetBody() const
  {
    if (method_ == Orthanc::HttpMethod_Post ||
        method_ == Orthanc::HttpMethod_Put)
    {
      return body_;
    }
    else
    {
      LOG(ERROR) << "HttpCommand::GetBody(): method_  not _Post or _Put";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  const std::string& HttpCommand::GetUsername() const
  {
    if (HasCredentials())
    {
      return username_;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  const std::string& HttpCommand::GetPassword() const
  {
    if (HasCredentials())
    {
      return password_;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }
}
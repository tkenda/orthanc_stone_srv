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


#include "DicomSource.h"

#include "../Oracle/HttpCommand.h"
#include "../Oracle/OrthancRestApiCommand.h"

#include <OrthancException.h>
#include <Toolbox.h>

#include <boost/algorithm/string/predicate.hpp>

namespace OrthancStone
{
  static std::string EncodeGetArguments(const std::map<std::string, std::string>& arguments)
  {
    std::string s;
    bool first = true;

    for (std::map<std::string, std::string>::const_iterator
           it = arguments.begin(); it != arguments.end(); ++it)
    {
      if (first)
      {
        s += "?";
        first = false;
      }
      else
      {
        s += "&";
      }

      s += it->first + "=" + it->second;
    }

    // TODO: Call Orthanc::Toolbox::UriEncode() ?

    return s;
  }


  static std::string AddUriSuffix(const std::string& base,
                                  const std::string& suffix)
  {
    if (base.empty())
    {
      return suffix;
    }
    else if (suffix.empty())
    {
      return base;
    }
    else
    {
      char lastBase = base[base.size() - 1];
      
      if (lastBase == '/' &&
          suffix[0] == '/')
      {
        return base + suffix.substr(1);
      }
      else if (lastBase == '/' ||
               suffix[0] == '/')
      {
        return base + suffix;
      }
      else
      {
        return base + "/" + suffix;
      }
    }
  }


  void DicomSource::SetOrthancSource(const Orthanc::WebServiceParameters& parameters)
  {
    type_ = DicomSourceType_Orthanc;
    webService_ = parameters;
    hasOrthancWebViewer1_ = false;
    hasOrthancAdvancedPreview_ = false;
  }


  bool DicomSource::IsSameSource(const DicomSource& other) const
  {
    if (type_ != other.type_)
    {
      return false;
    }
    else
    {
      switch (type_)
      {
        case DicomSourceType_Orthanc:
          return (webService_.GetUrl() == other.webService_.GetUrl() &&
                  webService_.GetUsername() == other.webService_.GetUsername() &&
                  webService_.GetHttpHeaders() == other.webService_.GetHttpHeaders() &&
                  hasOrthancWebViewer1_ == other.hasOrthancWebViewer1_ &&
                  hasOrthancAdvancedPreview_ == other.hasOrthancAdvancedPreview_);
          
        case DicomSourceType_DicomWeb:
          return (webService_.GetUrl() == other.webService_.GetUrl() &&
                  webService_.GetUsername() == other.webService_.GetUsername() &&
                  webService_.GetHttpHeaders() == other.webService_.GetHttpHeaders() &&
                  hasDicomWebRendered_ == other.hasDicomWebRendered_);
          
        case DicomSourceType_DicomWebThroughOrthanc:
          return (webService_.GetUrl() == other.webService_.GetUrl() &&
                  webService_.GetUsername() == other.webService_.GetUsername() &&
                  webService_.GetHttpHeaders() == other.webService_.GetHttpHeaders() &&
                  orthancDicomWebRoot_ == other.orthancDicomWebRoot_ &&
                  serverName_ == other.serverName_ &&
                  hasDicomWebRendered_ == other.hasDicomWebRendered_);
          
        case DicomSourceType_DicomDir:
          return true;
          
        default:
          throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }
    }
  }


  void DicomSource::SetOrthancSource()
  {
    Orthanc::WebServiceParameters parameters;
    parameters.SetUrl("http://localhost:8042/");
    SetOrthancSource(parameters);
  }


  const Orthanc::WebServiceParameters& DicomSource::GetOrthancParameters() const
  {
    if (type_ == DicomSourceType_Orthanc ||
        type_ == DicomSourceType_DicomWebThroughOrthanc)
    {
      return webService_;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  void DicomSource::SetDicomDirSource()
  {
    type_ = DicomSourceType_DicomDir;
  }


  void DicomSource::SetDicomWebSource(const std::string& baseUrl)
  {
    type_ = DicomSourceType_DicomWeb;
    webService_.SetUrl(baseUrl);
    webService_.ClearCredentials();
  }

  
  void DicomSource::SetDicomWebSource(const std::string& baseUrl,
                                      const std::string& username,
                                      const std::string& password)
  {
    type_ = DicomSourceType_DicomWeb;
    webService_.SetUrl(baseUrl);
    webService_.SetCredentials(username, password);
  }

  
  void DicomSource::SetDicomWebThroughOrthancSource(const Orthanc::WebServiceParameters& orthancParameters,
                                                    const std::string& dicomWebRoot,
                                                    const std::string& serverName)
  {
    type_ = DicomSourceType_DicomWebThroughOrthanc;
    webService_ = orthancParameters;
    orthancDicomWebRoot_ = dicomWebRoot;
    serverName_ = serverName;
  }

  
  void DicomSource::SetDicomWebThroughOrthancSource(const std::string& serverName)
  {
    Orthanc::WebServiceParameters orthanc;
    orthanc.SetUrl("http://localhost:8042/");
    SetDicomWebThroughOrthancSource(orthanc, "/dicom-web/", serverName);
  }

  
  bool DicomSource::IsDicomWeb() const
  {
    return (type_ == DicomSourceType_DicomWeb ||
            type_ == DicomSourceType_DicomWebThroughOrthanc);
  }


  IOracleCommand* DicomSource::CreateDicomWebCommand(const std::string& uri,
                                                     const std::map<std::string, std::string>& arguments,
                                                     const std::map<std::string, std::string>& headers,
                                                     Orthanc::IDynamicObject* payload) const
  {
    std::unique_ptr<Orthanc::IDynamicObject> protection(payload);

    switch (type_)
    {
      case DicomSourceType_DicomWeb:
      {
        std::unique_ptr<HttpCommand> command(new HttpCommand);
        
        command->SetMethod(Orthanc::HttpMethod_Get);
        command->SetUrl(AddUriSuffix(webService_.GetUrl(), uri + EncodeGetArguments(arguments)));
        command->SetHttpHeaders(webService_.GetHttpHeaders());

        for (std::map<std::string, std::string>::const_iterator
               it = headers.begin(); it != headers.end(); ++it)
        {
          command->SetHttpHeader(it->first, it->second);
        }
      
        if (!webService_.GetUsername().empty())
        {
          command->SetCredentials(webService_.GetUsername(), webService_.GetPassword());
        }         

        if (protection.get())
        {
          command->AcquirePayload(protection.release());
        }
        
        return command.release();
      }

      case DicomSourceType_DicomWebThroughOrthanc:
      {
        Json::Value args = Json::objectValue;
        for (std::map<std::string, std::string>::const_iterator
               it = arguments.begin(); it != arguments.end(); ++it)
        {
          args[it->first] = it->second;
        }
          
        Json::Value h = Json::objectValue;
        for (std::map<std::string, std::string>::const_iterator
               it = headers.begin(); it != headers.end(); ++it)
        {
          h[it->first] = it->second;
        }
        
        Json::Value body = Json::objectValue;
        body["Uri"] = uri;
        body["Arguments"] = args;
        body["HttpHeaders"] = h;

        std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
        command->SetMethod(Orthanc::HttpMethod_Post);
        command->SetUri(AddUriSuffix(orthancDicomWebRoot_, "/servers/" + serverName_ + "/get"));
        command->SetBody(body);

        if (protection.get())
        {
          command->AcquirePayload(protection.release());
        }
        
        return command.release();
      }

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  void DicomSource::AutodetectOrthancFeatures(const std::string& system,
                                              const std::string& plugins)
  {
    static const char* const REST_API_VERSION = "ApiVersion";

    if (IsDicomWeb())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }

    Json::Value a, b;
    if (Orthanc::Toolbox::ReadJson(a, system) &&
        Orthanc::Toolbox::ReadJson(b, plugins) &&
        a.type() == Json::objectValue &&
        b.type() == Json::arrayValue &&
        a.isMember(REST_API_VERSION) &&
        a[REST_API_VERSION].type() == Json::intValue)
    {
      SetOrthancAdvancedPreview(a[REST_API_VERSION].asInt() >= 5);

      hasOrthancWebViewer1_ = false;

      for (Json::Value::ArrayIndex i = 0; i < b.size(); i++)
      {
        if (b[i].type() != Json::stringValue)
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
        }

        if (boost::iequals(b[i].asString(), "web-viewer"))
        {
          hasOrthancWebViewer1_ = true;
        }
      }
    }
    else
    {
      printf("[%s] [%s]\n", system.c_str(), plugins.c_str());

      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
  }


  void DicomSource::SetOrthancWebViewer1(bool hasPlugin)
  {
    if (IsOrthanc())
    {
      hasOrthancWebViewer1_ = hasPlugin;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }

  bool DicomSource::HasOrthancWebViewer1() const
  {
    if (IsOrthanc())
    {
      return hasOrthancWebViewer1_;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }

  void DicomSource::SetOrthancAdvancedPreview(bool hasFeature)
  {
    if (IsOrthanc())
    {
      hasOrthancAdvancedPreview_ = hasFeature;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }

  bool DicomSource::HasOrthancAdvancedPreview() const
  {
    if (IsOrthanc())
    {
      return hasOrthancAdvancedPreview_;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }

  void DicomSource::SetDicomWebRendered(bool hasFeature)
  {
    if (IsDicomWeb())
    {
      hasDicomWebRendered_ = hasFeature;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }

  bool DicomSource::HasDicomWebRendered() const
  {
    if (IsDicomWeb())
    {
      return hasDicomWebRendered_;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  unsigned int DicomSource::GetQualityCount() const
  {
    if (IsDicomWeb())
    {
      return (HasDicomWebRendered() ? 2 : 1);
    }
    else if (IsOrthanc())
    {
      return (HasOrthancWebViewer1() || 
              HasOrthancAdvancedPreview() ? 2 : 1);
    }
    else if (IsDicomDir())
    {
      return 1;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }
  }


  void DicomSource::AddHttpHeader(const std::string& header,
                                  const std::string& value)
  {
    if (type_ == DicomSourceType_Orthanc ||
        type_ == DicomSourceType_DicomWeb ||
        type_ == DicomSourceType_DicomWebThroughOrthanc)
    {
      webService_.AddHttpHeader(header, value);
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadParameterType);
    }
  }
}

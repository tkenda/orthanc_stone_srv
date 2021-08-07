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


#include "IOrthancConnection.h"

#include <OrthancException.h>
#include <Toolbox.h>

#if !defined(ORTHANC_FRAMEWORK_VERSION_IS_ABOVE)
#  error You are using a version of the Orthanc framework that is too old
#endif

#if !ORTHANC_FRAMEWORK_VERSION_IS_ABOVE(1, 9, 0)
#  include <json/reader.h>
#endif


namespace OrthancStone
{
  void IOrthancConnection::ParseJson(Json::Value& result,
                                     const std::string& content)
  {
    bool ok;
    
#if ORTHANC_FRAMEWORK_VERSION_IS_ABOVE(1, 9, 0)
    ok = Orthanc::Toolbox::ReadJson(result, content);
#else
    // Backward compatibility (for use in orthanc-wsi 1.0)
    Json::Reader reader;
    ok = reader.parse(content, result);
#endif

    if (!ok)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
  }


  void IOrthancConnection::ParseJson(Json::Value& result,
                                     const void* content,
                                     size_t size)
  {
    bool ok;

#if ORTHANC_FRAMEWORK_VERSION_IS_ABOVE(1, 9, 0)
    ok = Orthanc::Toolbox::ReadJson(result, content, size);
#else
    // Backward compatibility (for use in orthanc-wsi 1.0)
    Json::Reader reader;
    ok = reader.parse(reinterpret_cast<const char*>(content),
                      reinterpret_cast<const char*>(content) + size, result);
#endif
    
    if (!ok)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
  }


  void IOrthancConnection::RestApiGet(Json::Value& result,
                                      IOrthancConnection& orthanc,
                                      const std::string& uri)
  {
    std::string content;
    orthanc.RestApiGet(content, uri);
    ParseJson(result, content);
  }


  void IOrthancConnection::RestApiPost(Json::Value& result,
                                       IOrthancConnection& orthanc,
                                       const std::string& uri,
                                       const std::string& body)
  {
    std::string content;
    orthanc.RestApiPost(content, uri, body);
    ParseJson(result, content);
  }


  void IOrthancConnection::RestApiPut(Json::Value& result,
                                      IOrthancConnection& orthanc,
                                      const std::string& uri,
                                      const std::string& body)
  {
    std::string content;
    orthanc.RestApiPut(content, uri, body);
    ParseJson(result, content);
  }
}

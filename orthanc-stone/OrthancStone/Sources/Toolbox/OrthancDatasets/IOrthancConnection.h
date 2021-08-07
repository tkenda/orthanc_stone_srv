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

#include <boost/noncopyable.hpp>
#include <string>
#include <json/value.h>

namespace OrthancStone
{
  class IOrthancConnection : public boost::noncopyable
  {
  public:
    virtual ~IOrthancConnection()
    {
    }

    virtual void RestApiGet(std::string& result,
                            const std::string& uri) = 0;

    virtual void RestApiPost(std::string& result,
                             const std::string& uri,
                             const std::string& body) = 0;

    virtual void RestApiPut(std::string& result,
                            const std::string& uri,
                            const std::string& body) = 0;

    virtual void RestApiDelete(const std::string& uri) = 0;

    static void ParseJson(Json::Value& result,
                          const std::string& content);

    static void ParseJson(Json::Value& result,
                          const void* content,
                          size_t size);

    static void RestApiGet(Json::Value& result,
                           IOrthancConnection& orthanc,
                           const std::string& uri);

    static void RestApiPost(Json::Value& result,
                            IOrthancConnection& orthanc,
                            const std::string& uri,
                            const std::string& body);

    static void RestApiPut(Json::Value& result,
                           IOrthancConnection& orthanc,
                           const std::string& uri,
                           const std::string& body);
  };
}

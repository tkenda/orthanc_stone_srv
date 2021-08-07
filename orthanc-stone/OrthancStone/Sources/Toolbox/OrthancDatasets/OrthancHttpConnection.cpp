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


#include "OrthancHttpConnection.h"

namespace OrthancStone
{
  void OrthancHttpConnection::Setup()
  {
    url_ = client_.GetUrl();

    // Don't follow 3xx HTTP (avoid redirections to "unsupported.png" in Orthanc)
    client_.SetRedirectionFollowed(false);  
  }


  OrthancHttpConnection::OrthancHttpConnection() :
    client_(Orthanc::WebServiceParameters(), "")
  {
    Setup();
  }


  OrthancHttpConnection::OrthancHttpConnection(const Orthanc::WebServiceParameters& parameters) :
    client_(parameters, "")
  {
    Setup();
  }


  void OrthancHttpConnection::RestApiGet(std::string& result,
                                         const std::string& uri)
  {
    boost::mutex::scoped_lock lock(mutex_);

    client_.SetMethod(Orthanc::HttpMethod_Get);
    client_.SetUrl(url_ + uri);
    client_.ApplyAndThrowException(result);
  }


  void OrthancHttpConnection::RestApiPost(std::string& result,
                                          const std::string& uri,
                                          const std::string& body)
  {
    boost::mutex::scoped_lock lock(mutex_);

    client_.SetMethod(Orthanc::HttpMethod_Post);
    client_.SetUrl(url_ + uri);

#if defined(ORTHANC_FRAMEWORK_VERSION_IS_ABOVE) && ORTHANC_FRAMEWORK_VERSION_IS_ABOVE(1, 9, 3)
    client_.SetExternalBody(body);
    client_.ApplyAndThrowException(result);
    client_.ClearBody();
#else
    client_.SetBody(body);
    client_.ApplyAndThrowException(result);
#endif
  }


  void OrthancHttpConnection::RestApiPut(std::string& result,
                                         const std::string& uri,
                                         const std::string& body)
  {
    boost::mutex::scoped_lock lock(mutex_);

    client_.SetMethod(Orthanc::HttpMethod_Put);
    client_.SetUrl(url_ + uri);

#if defined(ORTHANC_FRAMEWORK_VERSION_IS_ABOVE) && ORTHANC_FRAMEWORK_VERSION_IS_ABOVE(1, 9, 3)
    client_.SetExternalBody(body);
    client_.ApplyAndThrowException(result);
    client_.ClearBody();
#else
    client_.SetBody(body);
    client_.ApplyAndThrowException(result);
#endif
  }


  void OrthancHttpConnection::RestApiDelete(const std::string& uri)
  {
    boost::mutex::scoped_lock lock(mutex_);

    std::string result;

    client_.SetMethod(Orthanc::HttpMethod_Delete);
    client_.SetUrl(url_ + uri);
    client_.ApplyAndThrowException(result);
  }
}

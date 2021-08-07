/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#include "CurlOrthancConnection.h"

#if ORTHANC_ENABLE_CURL == 1

#include "../../Resources/Orthanc/Core/HttpClient.h"
#include "../../Resources/Orthanc/Core/OrthancException.h"

namespace OrthancStone
{
  void CurlOrthancConnection::RestApiGet(std::string& result,
                                         const std::string& uri)
  {
    /**
     * TODO: This function sometimes crashes if compiled with
     * MinGW-W64 (32 bit) in Release mode, on Windows XP. Introducing
     * a mutex here fixes the issue. Not sure of what is the
     * culprit. Maybe a bug in a old version of MinGW?
     **/

    Orthanc::HttpClient client(parameters_, uri);

    // Don't follow 3xx HTTP (avoid redirections to "unsupported.png" in Orthanc)
    client.SetRedirectionFollowed(false);  
   
    if (!client.Apply(result))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_UnknownResource);
    }
  }


  void CurlOrthancConnection::RestApiPost(std::string& result,
                                          const std::string& uri,
                                          const std::string& body)
  {
    Orthanc::HttpClient client(parameters_, uri);

    // Don't follow 3xx HTTP (avoid redirections to "unsupported.png" in Orthanc)
    client.SetRedirectionFollowed(false);  

    client.SetBody(body);
    client.SetMethod(Orthanc::HttpMethod_Post);
   
    if (!client.Apply(result))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_UnknownResource);
    }
  }
}

#endif

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

#include "IOrthancConnection.h"

#include <Compatibility.h>  // For ORTHANC_OVERRIDE
#include <HttpClient.h>

#include <boost/thread/mutex.hpp>

namespace OrthancStone
{
  // This class is thread-safe
  class OrthancHttpConnection : public IOrthancConnection
  {
  private:
    boost::mutex         mutex_;
    Orthanc::HttpClient  client_;
    std::string          url_;

    void Setup();

  public:
    OrthancHttpConnection();

    explicit OrthancHttpConnection(const Orthanc::WebServiceParameters& parameters);

    virtual void RestApiGet(std::string& result,
                            const std::string& uri) ORTHANC_OVERRIDE;

    virtual void RestApiPost(std::string& result,
                             const std::string& uri,
                             const std::string& body) ORTHANC_OVERRIDE;

    virtual void RestApiPut(std::string& result,
                            const std::string& uri,
                            const std::string& body) ORTHANC_OVERRIDE;

    virtual void RestApiDelete(const std::string& uri) ORTHANC_OVERRIDE;
  };
}

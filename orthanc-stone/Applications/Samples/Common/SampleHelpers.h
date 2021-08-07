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


#pragma once

#include <Logging.h>

#include <boost/algorithm/string.hpp>

#include <string>
#include <iostream>

namespace OrthancStoneHelpers
{
  inline void SetLogLevel(std::string logLevel)
  {
    boost::to_lower(logLevel);
    if (logLevel == "warning")
    {
      Orthanc::Logging::EnableInfoLevel(false);
      Orthanc::Logging::EnableTraceLevel(false);
    }
    else if (logLevel == "info")
    {
      Orthanc::Logging::EnableInfoLevel(true);
      Orthanc::Logging::EnableTraceLevel(false);
    }
    else if (logLevel == "trace")
    {
      Orthanc::Logging::EnableInfoLevel(true);
      Orthanc::Logging::EnableTraceLevel(true);
    }
    else
    {
      std::cerr << "Unknown log level \"" << logLevel << "\". Will use TRACE as default!";
      Orthanc::Logging::EnableInfoLevel(true);
      Orthanc::Logging::EnableTraceLevel(true);
    }
  }
}

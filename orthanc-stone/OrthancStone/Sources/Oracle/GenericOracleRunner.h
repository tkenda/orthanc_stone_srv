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

#include <OrthancFramework.h>  // To have the macros properly defined

#if !defined(ORTHANC_ENABLE_DCMTK)
#  error The macro ORTHANC_ENABLE_DCMTK must be defined
#endif

#if ORTHANC_ENABLE_DCMTK == 1
#  include "../Toolbox/ParsedDicomCache.h"
#endif

#include "IOracleCommand.h"
#include "../Messages/IMessageEmitter.h"

#include <Enumerations.h>  // For ORTHANC_OVERRIDE
#include <WebServiceParameters.h>

namespace OrthancStone
{
  class GenericOracleRunner : public boost::noncopyable
  {
  private:
    Orthanc::WebServiceParameters  orthanc_;
    std::string                    rootDirectory_;

#if ORTHANC_ENABLE_DCMTK == 1
    boost::shared_ptr<ParsedDicomCache>  dicomCache_;
#endif

  public:
    GenericOracleRunner() :
      rootDirectory_(".")
    {
    }

    void SetOrthanc(const Orthanc::WebServiceParameters& orthanc)
    {
      orthanc_ = orthanc;
    }

    const Orthanc::WebServiceParameters& GetOrthanc() const
    {
      return orthanc_;
    }

    void SetRootDirectory(const std::string& rootDirectory)
    {
      rootDirectory_ = rootDirectory;
    }

    const std::string GetRootDirectory() const
    {
      return rootDirectory_;
    }

#if ORTHANC_ENABLE_DCMTK == 1
    void SetDicomCache(boost::shared_ptr<ParsedDicomCache> cache)
    {
      dicomCache_ = cache;
    }
#endif

    void Run(boost::weak_ptr<IObserver> receiver,
             IMessageEmitter& emitter,
             const IOracleCommand& command);
  };
}

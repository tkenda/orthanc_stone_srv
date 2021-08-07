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

#include "../../../OrthancStone/Sources/Loaders/ILoadersContext.h"
#include "../../../OrthancStone/Sources/Loaders/OracleScheduler.h"
#include "WebAssemblyOracle.h"

#include <list>

namespace OrthancStone
{
  class WebAssemblyLoadersContext : public ILoadersContext
  {
  private:
    class Locker;
    
    WebAssemblyOracle                          oracle_;
    boost::shared_ptr<OracleScheduler>         scheduler_;
    std::list< boost::shared_ptr<IObserver> >  loaders_;
    
  public:
    WebAssemblyLoadersContext(unsigned int maxHighPriority,
                              unsigned int maxStandardPriority,
                              unsigned int maxLowPriority);

    void SetLocalOrthanc(const std::string& root)
    {
      oracle_.SetLocalOrthanc(root);
    }

    void SetRemoteOrthanc(const Orthanc::WebServiceParameters& orthanc)
    {
      oracle_.SetRemoteOrthanc(orthanc);
    }

    void SetDicomCacheSize(size_t size)
    {
      oracle_.SetDicomCacheSize(size);
    }

    WebAssemblyOracle::CachedInstanceAccessor* AccessCachedInstance(const std::string& sopInstanceUid)
    {
      return new WebAssemblyOracle::CachedInstanceAccessor(oracle_, sopInstanceUid);
    }

    virtual ILock* Lock() ORTHANC_OVERRIDE;
  };
}

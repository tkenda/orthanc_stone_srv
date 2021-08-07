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

#include "../Messages/IMessageEmitter.h"
#include "../Oracle/ThreadedOracle.h"
#include "ILoadersContext.h"
#include "DicomSource.h"
#include "OracleScheduler.h"

#include <boost/thread/recursive_mutex.hpp>

namespace OrthancStone
{
  class GenericLoadersContext : 
    public ILoadersContext,
    private IMessageEmitter
  {
  private:
    class Locker;

    // "Recursive mutex" is necessary, to be able to run
    // "ILoaderFactory" from a message handler triggered by
    // "EmitMessage()"
    boost::recursive_mutex  mutex_;

    IObservable                         oracleObservable_;
    std::unique_ptr<ThreadedOracle>     oracle_;
    boost::shared_ptr<OracleScheduler>  scheduler_;

    // Necessary to keep the loaders persistent (including global
    // function promises), after the function that created them is
    // left. This avoids creating one global variable for each loader.
    std::list< boost::shared_ptr<IObserver> >  loaders_; 

    virtual void EmitMessage(boost::weak_ptr<IObserver> observer,
                             const IMessage& message) ORTHANC_OVERRIDE;

  public:
    GenericLoadersContext(unsigned int maxHighPriority,
                          unsigned int maxStandardPriority,
                          unsigned int maxLowPriority);

    virtual ~GenericLoadersContext();
   
    virtual ILock* Lock() ORTHANC_OVERRIDE;

    void SetOrthancParameters(const Orthanc::WebServiceParameters& parameters);

    void SetRootDirectory(const std::string& root);
    
    void SetDicomCacheSize(size_t size);

    void StartOracle();

    void StopOracle();

    void WaitUntilComplete();
  };
}

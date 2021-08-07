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
#include <Compatibility.h>  // ORTHANC_OVERRIDE

#if !defined(ORTHANC_ENABLE_THREADS)
#  error The macro ORTHANC_ENABLE_THREADS must be defined
#endif

#if !defined(ORTHANC_ENABLE_DCMTK)
#  error The macro ORTHANC_ENABLE_DCMTK must be defined
#endif

#if ORTHANC_ENABLE_THREADS != 1
#  error This file can only compiled for native targets
#endif

#if ORTHANC_ENABLE_DCMTK == 1
#  include "../Toolbox/ParsedDicomCache.h"
#endif

#include "IOracle.h"
#include "GenericOracleRunner.h"
#include "../Messages/IMessageEmitter.h"

#include <MultiThreading/SharedMessageQueue.h>


namespace OrthancStone
{
  class ThreadedOracle : public IOracle
  {
  private:
    enum State
    {
      State_Setup,
      State_Running,
      State_Stopped
    };

    class Item;
    class SleepingCommands;

    IMessageEmitter&                     emitter_;
    Orthanc::WebServiceParameters        orthanc_;
    std::string                          rootDirectory_;
    Orthanc::SharedMessageQueue          queue_;
    State                                state_;
    boost::mutex                         mutex_;
    std::vector<boost::thread*>          workers_;
    boost::shared_ptr<SleepingCommands>  sleepingCommands_;
    boost::thread                        sleepingWorker_;
    unsigned int                         sleepingTimeResolution_;

#if ORTHANC_ENABLE_DCMTK == 1
    boost::shared_ptr<ParsedDicomCache>  dicomCache_;
#endif
    
    void Step();

    static void Worker(ThreadedOracle* that);

    static void SleepingWorker(ThreadedOracle* that);

    void StopInternal();

  public:
    explicit ThreadedOracle(IMessageEmitter& emitter);

    virtual ~ThreadedOracle();

    void SetOrthancParameters(const Orthanc::WebServiceParameters& orthanc);

    void SetRootDirectory(const std::string& rootDirectory);

    void SetThreadsCount(unsigned int count);

    void SetSleepingTimeResolution(unsigned int milliseconds);

    void SetDicomCacheSize(size_t size);

    void Start();

    void Stop()
    {
      StopInternal();
    }

    virtual bool Schedule(boost::shared_ptr<IObserver> receiver,
                          IOracleCommand* command) ORTHANC_OVERRIDE;
  };
}

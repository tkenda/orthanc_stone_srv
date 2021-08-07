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

#include "../OrthancStone.h"

#if !defined(ORTHANC_ENABLE_DCMTK)
#  error The macro ORTHANC_ENABLE_DCMTK must be defined
#endif

#include "../Messages/IMessageEmitter.h"
#include "../Messages/ObserverBase.h"
#include "../Oracle/GetOrthancImageCommand.h"
#include "../Oracle/GetOrthancWebViewerJpegCommand.h"
#include "../Oracle/HttpCommand.h"
#include "../Oracle/IOracle.h"
#include "../Oracle/OracleCommandExceptionMessage.h"
#include "../Oracle/OrthancRestApiCommand.h"
#include "../Oracle/ReadFileCommand.h"

#if ORTHANC_ENABLE_DCMTK == 1
#  include "../Oracle/ParseDicomSuccessMessage.h"
#endif

namespace OrthancStone
{
  class OracleScheduler : public ObserverBase<OracleScheduler>
  {
  public:
    static const int PRIORITY_HIGH = -1;
    static const int PRIORITY_LOW = 100;
  
  private:
    enum Priority
    {
      Priority_Low,
      Priority_Standard,
      Priority_High
    };

    class ReceiverPayload;
    class ScheduledCommand;

    typedef std::multimap<int, ScheduledCommand*>  Queue;

    IOracle&  oracle_;
    IMessageEmitter&  emitter_;
    Queue          standardPriorityQueue_;
    Queue          highPriorityQueue_;
    Queue          lowPriorityQueue_;
    unsigned int   maxHighPriorityCommands_;  // Used if priority <= PRIORITY_HIGH
    unsigned int   maxStandardPriorityCommands_;
    unsigned int   maxLowPriorityCommands_;  // Used if priority >= PRIORITY_LOW
    unsigned int   activeHighPriorityCommands_;
    unsigned int   activeStandardPriorityCommands_;
    unsigned int   activeLowPriorityCommands_;
    uint64_t       totalScheduled_;
    uint64_t       totalProcessed_;

    void ClearQueue(Queue& queue);

    void RemoveReceiverFromQueue(Queue& queue,
                                 boost::shared_ptr<IObserver> receiver);

    void CheckInvariants() const;

    void SpawnFromQueue(Queue& queue,
                        Priority priority);

    void SpawnCommands();

    void RemoveActiveCommand(const ReceiverPayload& payload);

    void Handle(const GetOrthancImageCommand::SuccessMessage& message);

    void Handle(const GetOrthancWebViewerJpegCommand::SuccessMessage& message);

    void Handle(const HttpCommand::SuccessMessage& message);

    void Handle(const OrthancRestApiCommand::SuccessMessage& message);

#if ORTHANC_ENABLE_DCMTK == 1
    void Handle(const ParseDicomSuccessMessage& message);
#endif

    void Handle(const ReadFileCommand::SuccessMessage& message);

    void Handle(const OracleCommandExceptionMessage& message);

    OracleScheduler(IOracle& oracle,
                    IMessageEmitter& emitter,
                    unsigned int maxHighPriority,
                    unsigned int maxStandardPriority,
                    unsigned int maxLowPriority);
    
  public:
    static boost::shared_ptr<OracleScheduler> Create(IOracle& oracle,
                                                     IObservable& oracleObservable,
                                                     IMessageEmitter& emitter)
    {
      return Create(oracle, oracleObservable, emitter, 1, 4, 1);
    }

    static boost::shared_ptr<OracleScheduler> Create(IOracle& oracle,
                                                     IObservable& oracleObservable,
                                                     IMessageEmitter& emitter,
                                                     unsigned int maxHighPriority,
                                                     unsigned int maxStandardPriority,
                                                     unsigned int maxLowPriority);

    ~OracleScheduler();

    unsigned int GetMaxHighPriorityCommands() const
    {
      return maxHighPriorityCommands_;
    }

    unsigned int GetMaxStandardPriorityCommands() const
    {
      return maxStandardPriorityCommands_;
    }

    unsigned int GetMaxLowPriorityCommands() const
    {
      return maxLowPriorityCommands_;
    }

    uint64_t GetTotalScheduled() const
    {
      return totalScheduled_;
    }

    uint64_t GetTotalProcessed() const
    {
      return totalProcessed_;
    }

    // Cancel the HTTP requests that are still pending in the queues,
    // and that are associated with the given receiver. Note that the
    // receiver might still receive answers to HTTP requests that were
    // already submitted to the oracle.
    void CancelRequests(boost::shared_ptr<IObserver> receiver);

    void CancelAllRequests();

    void Schedule(boost::shared_ptr<IObserver> receiver,
                  int priority,
                  IOracleCommand* command /* Takes ownership */);
  };
}

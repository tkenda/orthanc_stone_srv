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


#include "OracleScheduler.h"

#include "../Oracle/ParseDicomFromFileCommand.h"

namespace OrthancStone
{
  class OracleScheduler::ReceiverPayload : public Orthanc::IDynamicObject
  {
  private:
    Priority   priority_;
    boost::weak_ptr<IObserver>  receiver_;
    std::unique_ptr<IOracleCommand>  command_;

  public:
    ReceiverPayload(Priority priority,
                    boost::weak_ptr<IObserver> receiver,
                    IOracleCommand* command) :
      priority_(priority),
      receiver_(receiver),
      command_(command)
    {
      if (command == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }
    }

    Priority GetActivePriority() const
    {
      return priority_;
    }

    boost::weak_ptr<IObserver> GetOriginalReceiver() const
    {
      return receiver_;
    }

    const IOracleCommand& GetOriginalCommand() const
    {
      assert(command_.get() != NULL);
      return *command_;
    }
  }; 


  class OracleScheduler::ScheduledCommand : public boost::noncopyable
  {
  private:
    boost::weak_ptr<IObserver>     receiver_;
    std::unique_ptr<IOracleCommand>  command_;

  public:
    ScheduledCommand(boost::shared_ptr<IObserver> receiver,
                     IOracleCommand* command) :
      receiver_(receiver),
      command_(command)
    {
      if (command == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }
    }

    boost::weak_ptr<IObserver> GetReceiver() 
    {
      return receiver_;
    }
  
    bool IsSameReceiver(boost::shared_ptr<IObserver> receiver) const
    {
      boost::shared_ptr<IObserver> lock(receiver_.lock());

      return (lock &&
              lock.get() == receiver.get());
    }

    IOracleCommand* WrapCommand(Priority priority)
    {
      if (command_.get() == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
      else
      {
        std::unique_ptr<IOracleCommand> wrapped(command_->Clone());
        dynamic_cast<OracleCommandBase&>(*wrapped).AcquirePayload(new ReceiverPayload(priority, receiver_, command_.release()));
        return wrapped.release();
      }
    }
  };



  void OracleScheduler::ClearQueue(Queue& queue)
  {
    for (Queue::iterator it = queue.begin(); it != queue.end(); ++it)
    {
      assert(it->second != NULL);
      delete it->second;

      totalProcessed_ ++;
    }

    queue.clear();
  }

  
  void OracleScheduler::RemoveReceiverFromQueue(Queue& queue,
                                                boost::shared_ptr<IObserver> receiver)
  {
    if (!receiver)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    
    Queue tmp;
  
    for (Queue::iterator it = queue.begin(); it != queue.end(); ++it)
    {
      assert(it->second != NULL);

      if (!(it->second->IsSameReceiver(receiver)))
      {
        // This promise is still active
        tmp.insert(std::make_pair(it->first, it->second));
      }
      else
      {
        delete it->second;
        
        totalProcessed_ ++;
      }
    }

    queue = tmp;
  }

  
  void OracleScheduler::CheckInvariants() const
  {
#ifndef NDEBUG
    /*char buf[1024];
      sprintf(buf, "active: %d %d %d ; pending: %lu %lu %lu", 
      activeHighPriorityCommands_, activeStandardPriorityCommands_, activeLowPriorityCommands_,
      highPriorityQueue_.size(), standardPriorityQueue_.size(), lowPriorityQueue_.size());
      LOG(INFO) << buf;*/
  
    assert(activeHighPriorityCommands_ <= maxHighPriorityCommands_);
    assert(activeStandardPriorityCommands_ <= maxStandardPriorityCommands_);
    assert(activeLowPriorityCommands_ <= maxLowPriorityCommands_);
    assert(totalProcessed_ <= totalScheduled_);
    
    for (Queue::const_iterator it = standardPriorityQueue_.begin(); it != standardPriorityQueue_.end(); ++it)
    {
      assert(it->first > PRIORITY_HIGH &&
             it->first < PRIORITY_LOW);
    }

    for (Queue::const_iterator it = highPriorityQueue_.begin(); it != highPriorityQueue_.end(); ++it)
    {
      assert(it->first <= PRIORITY_HIGH);
    }

    for (Queue::const_iterator it = lowPriorityQueue_.begin(); it != lowPriorityQueue_.end(); ++it)
    {
      assert(it->first >= PRIORITY_LOW);
    }
#endif
  }

  
  void OracleScheduler::SpawnFromQueue(Queue& queue,
                                       Priority priority)
  {
    CheckInvariants();

    Queue::iterator item = queue.begin();
    assert(item != queue.end());

    std::unique_ptr<ScheduledCommand> command(dynamic_cast<ScheduledCommand*>(item->second));
    queue.erase(item);

    if (command.get() != NULL)
    {
      /**
       * Only schedule the command for execution in the oracle, if its
       * receiver has not been destroyed yet.
       **/
      boost::shared_ptr<IObserver> observer(command->GetReceiver().lock());
      if (observer)
      {
        if (oracle_.Schedule(GetSharedObserver(), command->WrapCommand(priority)))
        {
          /**
           * Executing this code if "Schedule()" returned "false"
           * above, will result in a memory leak within
           * "OracleScheduler", as the scheduler believes that some
           * command is still active (i.e. pending to be executed by
           * the oracle), hereby stalling the scheduler during its
           * destruction, and not freeing the
           * "shared_ptr<OracleScheduler>" of the Stone context (check
           * out "sjo-playground/WebViewer/Backend/Leak")
           **/

          switch (priority)
          {
            case Priority_High:
              activeHighPriorityCommands_ ++;
              break;

            case Priority_Standard:
              activeStandardPriorityCommands_ ++;
              break;

            case Priority_Low:
              activeLowPriorityCommands_ ++;
              break;

            default:
              throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
          }
        }
        else
        {
          totalProcessed_ ++;
        }
      }
    }
    else
    {
      LOG(ERROR) << "NULL command, should never happen";
    }

    CheckInvariants();
  }

  
  void OracleScheduler::SpawnCommands()
  {
    // Send as many commands as possible to the oracle
    while (!highPriorityQueue_.empty())
    {
      if (activeHighPriorityCommands_ < maxHighPriorityCommands_)
      {
        // First fill the high-priority lane
        SpawnFromQueue(highPriorityQueue_, Priority_High);
      }
      else if (activeStandardPriorityCommands_ < maxStandardPriorityCommands_)
      {
        // There remain too many high-priority commands for the
        // high-priority lane, schedule them to the standard-priority lanes
        SpawnFromQueue(highPriorityQueue_, Priority_Standard);
      }
      else if (activeLowPriorityCommands_ < maxLowPriorityCommands_)
      {
        SpawnFromQueue(highPriorityQueue_, Priority_Low);
      }
      else
      {
        return;   // No slot available
      }
    }
  
    while (!standardPriorityQueue_.empty())
    {
      if (activeStandardPriorityCommands_ < maxStandardPriorityCommands_)
      {
        SpawnFromQueue(standardPriorityQueue_, Priority_Standard);
      }
      else if (activeLowPriorityCommands_ < maxLowPriorityCommands_)
      {
        SpawnFromQueue(standardPriorityQueue_, Priority_Low);
      }
      else
      {
        return;
      }
    }
  
    while (!lowPriorityQueue_.empty())
    {
      if (activeLowPriorityCommands_ < maxLowPriorityCommands_)
      {
        SpawnFromQueue(lowPriorityQueue_, Priority_Low);
      }
      else
      {
        return;
      }
    }  
  }
  

  void OracleScheduler::RemoveActiveCommand(const ReceiverPayload& payload)
  {
    CheckInvariants();

    totalProcessed_ ++;

    switch (payload.GetActivePriority())
    {
      case Priority_High:
        assert(activeHighPriorityCommands_ > 0);
        activeHighPriorityCommands_ --;
        break;

      case Priority_Standard:
        assert(activeStandardPriorityCommands_ > 0);
        activeStandardPriorityCommands_ --;
        break;

      case Priority_Low:
        assert(activeLowPriorityCommands_ > 0);
        activeLowPriorityCommands_ --;
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    SpawnCommands();

    CheckInvariants();
  }

  
  void OracleScheduler::Handle(const GetOrthancImageCommand::SuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());
    const ReceiverPayload& payload = dynamic_cast<const ReceiverPayload&>(message.GetOrigin().GetPayload());
    
    RemoveActiveCommand(payload);

    GetOrthancImageCommand::SuccessMessage bis(
      dynamic_cast<const GetOrthancImageCommand&>(payload.GetOriginalCommand()),
      message.GetImage(), message.GetMimeType());
    emitter_.EmitMessage(payload.GetOriginalReceiver(), bis);
  }
  

  void OracleScheduler::Handle(const GetOrthancWebViewerJpegCommand::SuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());
    const ReceiverPayload& payload = dynamic_cast<const ReceiverPayload&>(message.GetOrigin().GetPayload());
    
    RemoveActiveCommand(payload);

    GetOrthancWebViewerJpegCommand::SuccessMessage bis(
      dynamic_cast<const GetOrthancWebViewerJpegCommand&>(payload.GetOriginalCommand()),
      message.GetImage());
    emitter_.EmitMessage(payload.GetOriginalReceiver(), bis);
  }

  
  void OracleScheduler::Handle(const HttpCommand::SuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());
    const ReceiverPayload& payload = dynamic_cast<const ReceiverPayload&>(message.GetOrigin().GetPayload());
    
    RemoveActiveCommand(payload);

    HttpCommand::SuccessMessage bis(
      dynamic_cast<const HttpCommand&>(payload.GetOriginalCommand()),
      message.GetAnswerHeaders(), message.GetAnswer());
    emitter_.EmitMessage(payload.GetOriginalReceiver(), bis);
  }

  
  void OracleScheduler::Handle(const OrthancRestApiCommand::SuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());
    const ReceiverPayload& payload = dynamic_cast<const ReceiverPayload&>(message.GetOrigin().GetPayload());
    
    RemoveActiveCommand(payload);

    OrthancRestApiCommand::SuccessMessage bis(
      dynamic_cast<const OrthancRestApiCommand&>(payload.GetOriginalCommand()),
      message.GetAnswerHeaders(), message.GetAnswer());
    emitter_.EmitMessage(payload.GetOriginalReceiver(), bis);
  }

  
#if ORTHANC_ENABLE_DCMTK == 1
  void OracleScheduler::Handle(const ParseDicomSuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());
    const ReceiverPayload& payload = dynamic_cast<const ReceiverPayload&>(message.GetOrigin().GetPayload());
    
    RemoveActiveCommand(payload);

    ParseDicomSuccessMessage bis(
      dynamic_cast<const OracleCommandBase&>(payload.GetOriginalCommand()),
      message.GetSource(), message.GetDicom(), message.GetFileSize(), message.HasPixelData());
    emitter_.EmitMessage(payload.GetOriginalReceiver(), bis);
  }
#endif
  

  void OracleScheduler::Handle(const ReadFileCommand::SuccessMessage& message)
  {
    assert(message.GetOrigin().HasPayload());
    const ReceiverPayload& payload = dynamic_cast<const ReceiverPayload&>(message.GetOrigin().GetPayload());
    
    RemoveActiveCommand(payload);

    ReadFileCommand::SuccessMessage bis(
      dynamic_cast<const ReadFileCommand&>(payload.GetOriginalCommand()),
      message.GetContent());
    emitter_.EmitMessage(payload.GetOriginalReceiver(), bis);
  }
  

  void OracleScheduler::Handle(const OracleCommandExceptionMessage& message)
  {
    const OracleCommandBase& command = dynamic_cast<const OracleCommandBase&>(message.GetOrigin());
    
    assert(command.HasPayload());
    const ReceiverPayload& payload = dynamic_cast<const ReceiverPayload&>(command.GetPayload());
    
    RemoveActiveCommand(payload);

    OracleCommandExceptionMessage bis(payload.GetOriginalCommand(), message.GetException());
    emitter_.EmitMessage(payload.GetOriginalReceiver(), bis);
  }  

  
  OracleScheduler::OracleScheduler(IOracle& oracle,
                                   IMessageEmitter& emitter,
                                   unsigned int maxHighPriority,
                                   unsigned int maxStandardPriority,
                                   unsigned int maxLowPriority) :
    oracle_(oracle),
    emitter_(emitter),
    maxHighPriorityCommands_(maxHighPriority),
    maxStandardPriorityCommands_(maxStandardPriority),
    maxLowPriorityCommands_(maxLowPriority),
    activeHighPriorityCommands_(0),
    activeStandardPriorityCommands_(0),
    activeLowPriorityCommands_(0),
    totalScheduled_(0),
    totalProcessed_(0)
  {
    assert(PRIORITY_HIGH < 0 &&
           PRIORITY_LOW > 0);

    if (maxLowPriority == 0)
    {
      // There must be at least 1 lane available to deal with low-priority commands
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }

    
  boost::shared_ptr<OracleScheduler> OracleScheduler::Create(IOracle& oracle,
                                                             IObservable& oracleObservable,
                                                             IMessageEmitter& emitter,
                                                             unsigned int maxHighPriority,
                                                             unsigned int maxStandardPriority,
                                                             unsigned int maxLowPriority)
  {
    boost::shared_ptr<OracleScheduler> scheduler
      (new OracleScheduler(oracle, emitter, maxHighPriority, maxStandardPriority, maxLowPriority));
    scheduler->Register<GetOrthancImageCommand::SuccessMessage>(oracleObservable, &OracleScheduler::Handle);
    scheduler->Register<GetOrthancWebViewerJpegCommand::SuccessMessage>(oracleObservable, &OracleScheduler::Handle);
    scheduler->Register<HttpCommand::SuccessMessage>(oracleObservable, &OracleScheduler::Handle);
    scheduler->Register<OrthancRestApiCommand::SuccessMessage>(oracleObservable, &OracleScheduler::Handle);
    scheduler->Register<ReadFileCommand::SuccessMessage>(oracleObservable, &OracleScheduler::Handle);
    scheduler->Register<OracleCommandExceptionMessage>(oracleObservable, &OracleScheduler::Handle);

#if ORTHANC_ENABLE_DCMTK == 1
    scheduler->Register<ParseDicomSuccessMessage>(oracleObservable, &OracleScheduler::Handle);
#endif

    return scheduler;
  }
    

  OracleScheduler::~OracleScheduler()
  {      
    CancelAllRequests();
  }


  void OracleScheduler::CancelRequests(boost::shared_ptr<IObserver> receiver)
  {
    RemoveReceiverFromQueue(standardPriorityQueue_, receiver);
    RemoveReceiverFromQueue(highPriorityQueue_, receiver);
    RemoveReceiverFromQueue(lowPriorityQueue_, receiver);
  }

  
  void OracleScheduler::CancelAllRequests()
  {      
    ClearQueue(standardPriorityQueue_);
    ClearQueue(highPriorityQueue_);
    ClearQueue(lowPriorityQueue_);
  }


  void OracleScheduler::Schedule(boost::shared_ptr<IObserver> receiver,
                                 int priority,
                                 IOracleCommand* command /* Takes ownership */)
  {
    std::unique_ptr<ScheduledCommand> pending(new ScheduledCommand(receiver, dynamic_cast<IOracleCommand*>(command)));

    /**
     * Safeguard to remember that a new "Handle()" method and a call
     * to "scheduler->Register()" must be implemented for each
     * possible oracle command.
     **/
    assert(command->GetType() == IOracleCommand::Type_GetOrthancImage ||
           command->GetType() == IOracleCommand::Type_GetOrthancWebViewerJpeg ||
           command->GetType() == IOracleCommand::Type_Http ||
           command->GetType() == IOracleCommand::Type_OrthancRestApi ||
           command->GetType() == IOracleCommand::Type_ParseDicomFromFile ||
           command->GetType() == IOracleCommand::Type_ParseDicomFromWado ||
           command->GetType() == IOracleCommand::Type_ReadFile);

    if (priority <= PRIORITY_HIGH)
    {
      highPriorityQueue_.insert(std::make_pair(priority, pending.release()));
    }
    else if (priority >= PRIORITY_LOW)
    {
      lowPriorityQueue_.insert(std::make_pair(priority, pending.release()));
    }
    else
    {
      standardPriorityQueue_.insert(std::make_pair(priority, pending.release()));
    }

    totalScheduled_ ++;

    SpawnCommands();
  }
}

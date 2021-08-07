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


#include "LoaderStateMachine.h"

#include "../Loaders/ILoadersContext.h"

#include <OrthancException.h>

namespace OrthancStone
{
  void LoaderStateMachine::State::Handle(const OrthancRestApiCommand::SuccessMessage& message)
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
  }
      

  void LoaderStateMachine::State::Handle(const GetOrthancImageCommand::SuccessMessage& message)
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
  }

      
  void LoaderStateMachine::State::Handle(const GetOrthancWebViewerJpegCommand::SuccessMessage& message)
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
  }


  void LoaderStateMachine::Schedule(OracleCommandBase* command)
  {
    LOG(TRACE) << "LoaderStateMachine(" << std::hex << this << std::dec << ")::Schedule()";

    std::unique_ptr<OracleCommandBase> protection(command);

    if (command == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
      
    if (!command->HasPayload())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange,
                                      "The payload must contain the next state");
    }
    pendingCommands_.push_back(protection.release());

    Step();
  }


  void LoaderStateMachine::Start()
  {
    LOG(TRACE) << "LoaderStateMachine(" << std::hex << this << std::dec << ")::Start()";

    if (active_)
    {
      LOG(TRACE) << "LoaderStateMachine::Start() called while active_ is true";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }

    active_ = true;

    for (size_t i = 0; i < simultaneousDownloads_; i++)
    {
      Step();
    }
  }


  void LoaderStateMachine::Step()
  {
    if (!pendingCommands_.empty() &&
        activeCommands_ < simultaneousDownloads_)
    {

      IOracleCommand* nextCommand = pendingCommands_.front();

      LOG(TRACE) << "    LoaderStateMachine(" << std::hex << this << std::dec << 
        ")::Step(): activeCommands_ (" << activeCommands_ << 
        ") < simultaneousDownloads_ (" << simultaneousDownloads_ << 
        ") --> will Schedule command addr " << std::hex << nextCommand << std::dec;

      {
        std::unique_ptr<ILoadersContext::ILock> lock(loadersContext_.Lock());
        boost::shared_ptr<IObserver> observer(GetSharedObserver());
        lock->Schedule(observer, 0, nextCommand); // TODO: priority!
      }
      pendingCommands_.pop_front();

      activeCommands_++;
    }
    else
    {
      LOG(TRACE) << "    LoaderStateMachine(" << std::hex << this << std::dec << 
        ")::Step(): activeCommands_ (" << activeCommands_ << 
        ") >= simultaneousDownloads_ (" << simultaneousDownloads_ << 
        ") --> will NOT Schedule command";
    }
  }


  void LoaderStateMachine::Clear()
  {
    LOG(TRACE) << "LoaderStateMachine(" << std::hex << this << std::dec << ")::Clear()";
    for (PendingCommands::iterator it = pendingCommands_.begin();
         it != pendingCommands_.end(); ++it)
    {
      delete *it;
    }

    pendingCommands_.clear();
  }
  

  void LoaderStateMachine::HandleExceptionMessage(const OracleCommandExceptionMessage& message)
  {
    LOG(ERROR) << "LoaderStateMachine::HandleExceptionMessage: error in the state machine, stopping all processing. Caller: "
      << message.GetOrigin().GetCallerName();
    LOG(ERROR) << "Error: " << message.GetException().What() << " Details: " <<
      message.GetException().GetDetails();
    Clear();
  }

  template <typename T>
  void LoaderStateMachine::HandleSuccessMessage(const T& message)
  {
    if (activeCommands_ == 0) {
      LOG(ERROR) << "LoaderStateMachine(" << std::hex << this << std::dec << ")::HandleSuccessMessage : activeCommands_ should be > 0 but is: " << activeCommands_;
    }
    else {
      activeCommands_--;
      try
      {
        dynamic_cast<State&>(message.GetOrigin().GetPayload()).Handle(message);
        Step();
      }
      catch (Orthanc::OrthancException& e)
      {
        LOG(ERROR) << "Error in the state machine, stopping all processing. Caller: " <<
          message.GetOrigin().GetCallerName() << ". Error: " << e.What() << " Details: " << e.GetDetails();
        Clear();
      }
    }
  }


  LoaderStateMachine::LoaderStateMachine(
    ILoadersContext& loadersContext)
    : loadersContext_(loadersContext)
    , active_(false)
    , simultaneousDownloads_(4)
    , activeCommands_(0)
  {
    LOG(TRACE) << "LoaderStateMachine(" << std::hex << this 
               << std::dec << ")::LoaderStateMachine()";
  }

  void LoaderStateMachine::PostConstructor()
  {
    std::unique_ptr<ILoadersContext::ILock>
      lock(loadersContext_.Lock());

    IObservable& observable = lock->GetOracleObservable();

    // TODO => Move this out of constructor
    Register<OrthancRestApiCommand::SuccessMessage>(
      observable, &LoaderStateMachine::HandleSuccessMessage);
    Register<GetOrthancImageCommand::SuccessMessage>(
      observable, &LoaderStateMachine::HandleSuccessMessage);
    Register<GetOrthancWebViewerJpegCommand::SuccessMessage>(
      observable, &LoaderStateMachine::HandleSuccessMessage);
    Register<OracleCommandExceptionMessage>(
      observable, &LoaderStateMachine::HandleExceptionMessage);
  }

  LoaderStateMachine::~LoaderStateMachine()
  {
    LOG(TRACE) << "LoaderStateMachine(" << std::hex << this << std::dec << ")::~LoaderStateMachine()";
    Clear();
  }

  void LoaderStateMachine::SetSimultaneousDownloads(unsigned int count)
  {
    if (active_)
    {
      LOG(ERROR) << "LoaderStateMachine::SetSimultaneousDownloads called while active_ is true";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else if (count == 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);        
    }
    else
    {
      simultaneousDownloads_ = count;
    }
  }
}

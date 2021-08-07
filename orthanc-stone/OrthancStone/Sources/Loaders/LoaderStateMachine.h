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

#include "../Messages/IObservable.h"
#include "../Messages/ObserverBase.h"
#include "../Oracle/GetOrthancImageCommand.h"
#include "../Oracle/GetOrthancWebViewerJpegCommand.h"
#include "../Oracle/IOracle.h"
#include "../Oracle/OracleCommandExceptionMessage.h"
#include "../Oracle/OrthancRestApiCommand.h"

#include <IDynamicObject.h>

#include <list>

namespace OrthancStone
{
  class ILoadersContext;

  /**
     This class is supplied with Oracle commands and will schedule up to 
     simultaneousDownloads_ of them at the same time, then will schedule the 
     rest once slots become available. It is used, a.o., by the 
     OrtancMultiframeVolumeLoader class.

     To use it, you need to create commands that derive from State.

     You need to initialize them with the object that must be called when 
     an answer is received. 
  */

  class LoaderStateMachine : public ObserverBase<LoaderStateMachine>
  {
  public:
    class State : public Orthanc::IDynamicObject
    {
    private:
      LoaderStateMachine&  that_;

    public:
      explicit State(LoaderStateMachine& that) :
        that_(that)
      {
      }

      explicit State(const State& currentState) :
        that_(currentState.that_)
      {
      }

      void Schedule(OracleCommandBase* command) const
      {
        that_.Schedule(command);
      }

      template <typename T>
      T& GetLoader() const
      {
        return dynamic_cast<T&>(that_);
      }
      
      virtual void Handle(const OrthancRestApiCommand::SuccessMessage& message);
      
      virtual void Handle(const GetOrthancImageCommand::SuccessMessage& message);
      
      virtual void Handle(const GetOrthancWebViewerJpegCommand::SuccessMessage& message);
    };

    void Schedule(OracleCommandBase* command);

    void Start();

  private:
    void Step();

    void Clear();

    void HandleExceptionMessage(const OracleCommandExceptionMessage& message);

    template <typename T>
    void HandleSuccessMessage(const T& message);

    typedef std::list<IOracleCommand*>  PendingCommands;

    ILoadersContext&  loadersContext_;
    bool                            active_;
    unsigned int                    simultaneousDownloads_;
    PendingCommands                 pendingCommands_;
    unsigned int                    activeCommands_;


  public:
    explicit LoaderStateMachine(ILoadersContext& loadersContext);

    void PostConstructor();

    virtual ~LoaderStateMachine();

    bool IsActive() const
    {
      return active_;
    }

    void SetSimultaneousDownloads(unsigned int count);  
  };
}

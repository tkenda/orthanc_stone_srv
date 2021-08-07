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

#include "../Messages/IObserver.h"
#include "../Messages/IObservable.h"
#include "../Oracle/IOracleCommand.h"

#include <boost/shared_ptr.hpp>

namespace OrthancStone
{
  class ILoadersContext : public boost::noncopyable
  {
  public:
    class ILock : public boost::noncopyable
    {
    public:
      virtual ~ILock()
      {
      }

      /**
       * This method is useful for loaders that must be able to
       * re-lock the Stone loaders context in the future (for instance
       * to schedule new commands once some command is processed).
       **/
      virtual ILoadersContext& GetContext() const = 0;

      /**
       * Get a reference to the observable against which a loader must
       * listen to be informed of messages issued by the oracle once
       * some command is processed.
       **/
      virtual IObservable& GetOracleObservable() const = 0;

      /**
       * Schedule a new command for further processing by the
       * oracle. The "receiver" argument indicates to which object the
       * notification messages are sent by the oracle upon completion
       * of the command. The command is possibly not directly sent to
       * the oracle: Instead, an internal "OracleScheduler" object is
       * often used as a priority queue to rule the order in which
       * commands are actually sent to the oracle. Hence the
       * "priority" argument (commands with lower value are executed
       * first).
       **/
      virtual void Schedule(boost::shared_ptr<IObserver> receiver,
                            int priority,
                            IOracleCommand* command /* Takes ownership */) = 0;

      /**
       * Cancel all the commands that are waiting in the
       * "OracleScheduler" queue and that are linked to the given
       * receiver (i.e. the observer that was specified at the time
       * method "Schedule()" was called). This is useful for real-time
       * processing, as it allows to replace commands that were
       * scheduled in the past by more urgent commands.
       *
       * Note that this call does not affect commands that would have
       * already be sent to the oracle. As a consequence, the receiver
       * might still receive messages that were sent to the oracle
       * before the cancellation (be prepared to handle such
       * messages).
       **/
      virtual void CancelRequests(boost::shared_ptr<IObserver> receiver) = 0;

      /**
       * Same as "CancelRequests()", but targets all the receivers.
       **/
      virtual void CancelAllRequests() = 0;

      /**
       * Add a reference to the given observer in the Stone loaders
       * context. This can be used to match the lifetime of a loader
       * with the lifetime of the Stone context: This is useful if
       * your Stone application does not keep a reference to the
       * loader by itself (typically in global promises), which would
       * make the loader disappear as soon as the scope of the
       * variable is left.
       **/
      virtual void AddLoader(boost::shared_ptr<IObserver> loader) = 0;

      /**
       * Returns the number of commands that were scheduled and
       * processed using the "Schedule()" method. By "processed"
       * commands, we refer to the number of commands that were either
       * executed by the oracle, or canceled by the user. So the
       * counting sequences are monotonically increasing over time.
       **/
      virtual void GetStatistics(uint64_t& scheduledCommands,
                                 uint64_t& processedCommands) = 0;
    };

    virtual ~ILoadersContext()
    {
    }

    /**
     * Locks the Stone loaders context, to give access to its
     * underlying features. This is important for Stone applications
     * running in a multi-threaded environment, for which a global
     * mutex is locked.
     **/
    virtual ILock* Lock() = 0;
  };
}

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


#include "ThreadedOracle.h"

#include "SleepOracleCommand.h"

#include <Logging.h>
#include <OrthancException.h>

namespace OrthancStone
{
  class ThreadedOracle::Item : public Orthanc::IDynamicObject
  {
  private:
    boost::weak_ptr<IObserver>      receiver_;
    std::unique_ptr<IOracleCommand>   command_;

  public:
    Item(boost::weak_ptr<IObserver> receiver,
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

    IOracleCommand& GetCommand()
    {
      assert(command_.get() != NULL);
      return *command_;
    }
  };


  class ThreadedOracle::SleepingCommands : public boost::noncopyable
  {
  private:
    class Item
    {
    private:
      boost::weak_ptr<IObserver>         receiver_;
      std::unique_ptr<SleepOracleCommand>  command_;
      boost::posix_time::ptime           expiration_;

    public:
      Item(boost::weak_ptr<IObserver> receiver,
           SleepOracleCommand* command) :
        receiver_(receiver),
        command_(command)
      {
        if (command == NULL)
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
        }

        expiration_ = (boost::posix_time::microsec_clock::local_time() + 
                       boost::posix_time::milliseconds(command_->GetDelay()));
      }

      const boost::posix_time::ptime& GetExpirationTime() const
      {
        return expiration_;
      }

      void Awake(IMessageEmitter& emitter)
      {
        assert(command_.get() != NULL);

        SleepOracleCommand::TimeoutMessage message(*command_);
        emitter.EmitMessage(receiver_, message);
      }
    };

    typedef std::list<Item*>  Content;

    boost::mutex  mutex_;
    Content       content_;

  public:
    ~SleepingCommands()
    {
      for (Content::iterator it = content_.begin(); it != content_.end(); ++it)
      {
        if (*it != NULL)
        {
          delete *it;
        }
      }
    }

    void Add(boost::weak_ptr<IObserver> receiver,
             SleepOracleCommand* command)   // Takes ownership
    {
      boost::mutex::scoped_lock lock(mutex_);

      content_.push_back(new Item(receiver, command));
    }

    void AwakeExpired(IMessageEmitter& emitter)
    {
      boost::mutex::scoped_lock lock(mutex_);

      const boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();

      Content  stillSleeping;
        
      for (Content::iterator it = content_.begin(); it != content_.end(); ++it)
      {
        if (*it != NULL &&
            (*it)->GetExpirationTime() <= now)
        {
          (*it)->Awake(emitter);
          delete *it;
          *it = NULL;
        }
        else
        {
          stillSleeping.push_back(*it);
        }
      }

      // Compact the still-sleeping commands
      content_ = stillSleeping;
    }
  };


  void ThreadedOracle::Step()
  {
    std::unique_ptr<Orthanc::IDynamicObject>  object(queue_.Dequeue(100));

    if (object.get() != NULL)
    {
      Item& item = dynamic_cast<Item&>(*object);

      if (item.GetCommand().GetType() == IOracleCommand::Type_Sleep)
      {
        SleepOracleCommand& command = dynamic_cast<SleepOracleCommand&>(item.GetCommand());
          
        std::unique_ptr<SleepOracleCommand> copy(new SleepOracleCommand(command.GetDelay()));
          
        if (command.HasPayload())
        {
          copy->AcquirePayload(command.ReleasePayload());
        }
          
        sleepingCommands_->Add(item.GetReceiver(), copy.release());
      }
      else
      {
        GenericOracleRunner runner;

        {
          boost::mutex::scoped_lock lock(mutex_);
          runner.SetOrthanc(orthanc_);
          runner.SetRootDirectory(rootDirectory_);

#if ORTHANC_ENABLE_DCMTK == 1
          if (dicomCache_)
          {
            runner.SetDicomCache(dicomCache_);
          }
#endif
        }

        runner.Run(item.GetReceiver(), emitter_, item.GetCommand());
      }
    }
  }


  void ThreadedOracle::Worker(ThreadedOracle* that)
  {
    assert(that != NULL);
      
    for (;;)
    {
      {
        boost::mutex::scoped_lock lock(that->mutex_);
        if (that->state_ != State_Running)
        {
          return;
        }
      }

      that->Step();
    }
  }


  void ThreadedOracle::SleepingWorker(ThreadedOracle* that)
  {
    assert(that != NULL);
      
    for (;;)
    {
      {
        boost::mutex::scoped_lock lock(that->mutex_);
        if (that->state_ != State_Running)
        {
          return;
        }
      }

      that->sleepingCommands_->AwakeExpired(that->emitter_);

      boost::this_thread::sleep(boost::posix_time::milliseconds(that->sleepingTimeResolution_));
    }
  }


  void ThreadedOracle::StopInternal()
  {
    {
      boost::mutex::scoped_lock lock(mutex_);

      if (state_ == State_Setup ||
          state_ == State_Stopped)
      {
        return;
      }
      else
      {
        state_ = State_Stopped;
      }
    }

    if (sleepingWorker_.joinable())
    {
      sleepingWorker_.join();
    }

    for (size_t i = 0; i < workers_.size(); i++)
    {
      if (workers_[i] != NULL)
      {
        if (workers_[i]->joinable())
        {
          workers_[i]->join();
        }

        delete workers_[i];
      }
    } 
  }


  ThreadedOracle::ThreadedOracle(IMessageEmitter& emitter) :
    emitter_(emitter),
    rootDirectory_("."),
    state_(State_Setup),
    workers_(4),
    sleepingCommands_(new SleepingCommands),
    sleepingTimeResolution_(50)  // By default, time resolution of 50ms
  {
  }


  ThreadedOracle::~ThreadedOracle()
  {
    if (state_ == State_Running)
    {
      LOG(ERROR) << "The threaded oracle is still running, explicit call to "
                 << "Stop() is mandatory to avoid crashes";
    }

    try
    {
      StopInternal();
    }
    catch (Orthanc::OrthancException& e)
    {
      LOG(ERROR) << "Exception while stopping the threaded oracle: " << e.What();
    }
    catch (...)
    {
      LOG(ERROR) << "Native exception while stopping the threaded oracle";
    }
  }

  
  void ThreadedOracle::SetOrthancParameters(const Orthanc::WebServiceParameters& orthanc)
  {
    boost::mutex::scoped_lock lock(mutex_);
    orthanc_ = orthanc;
  }


  void ThreadedOracle::SetRootDirectory(const std::string& rootDirectory)
  {
    boost::mutex::scoped_lock lock(mutex_);
    rootDirectory_ = rootDirectory;
  }


  void ThreadedOracle::SetThreadsCount(unsigned int count)
  {
    boost::mutex::scoped_lock lock(mutex_);

    if (count == 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else if (state_ != State_Setup)
    {
      LOG(ERROR) << "ThreadedOracle::SetThreadsCount(): (state_ != State_Setup)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      workers_.resize(count);
    }
  }


  void ThreadedOracle::SetSleepingTimeResolution(unsigned int milliseconds)
  {
    boost::mutex::scoped_lock lock(mutex_);

    if (milliseconds == 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else if (state_ != State_Setup)
    {
      LOG(ERROR) << "ThreadedOracle::SetSleepingTimeResolution(): (state_ != State_Setup)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      sleepingTimeResolution_ = milliseconds;
    }
  }


  void ThreadedOracle::SetDicomCacheSize(size_t size)
  {
#if ORTHANC_ENABLE_DCMTK == 1
    boost::mutex::scoped_lock lock(mutex_);

    if (state_ != State_Setup)
    {
      LOG(ERROR) << "ThreadedOracle::SetDicomCacheSize(): (state_ != State_Setup)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      if (size == 0)
      {
        dicomCache_.reset();
      }
      else
      {
        dicomCache_.reset(new ParsedDicomCache(size));
      }
    }
#endif
  }


  void ThreadedOracle::Start()
  {
    boost::mutex::scoped_lock lock(mutex_);

    if (state_ != State_Setup)
    {
      LOG(ERROR) << "ThreadedOracle::Start(): (state_ != State_Setup)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      LOG(INFO) << "Starting oracle with " << workers_.size() << " worker threads";
      state_ = State_Running;

      for (unsigned int i = 0; i < workers_.size(); i++)
      {
        workers_[i] = new boost::thread(Worker, this);
      }

      sleepingWorker_ = boost::thread(SleepingWorker, this);
    }      
  }


  bool ThreadedOracle::Schedule(boost::shared_ptr<IObserver> receiver,
                                IOracleCommand* command)
  {
    std::unique_ptr<Item> item(new Item(receiver, command));

    {
      boost::mutex::scoped_lock lock(mutex_);

      if (state_ == State_Running)
      {
        //LOG(INFO) << "New oracle command queued";
        queue_.Enqueue(item.release());
        return true;
      }
      else
      {
        LOG(TRACE) << "Command not enqueued, as the oracle has stopped";
        return false;
      }
    }
  }
}

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


#include "GenericLoadersContext.h"

namespace OrthancStone
{
  class GenericLoadersContext::Locker : public ILoadersContext::ILock
  {
  private:
    GenericLoadersContext& that_;
    boost::recursive_mutex::scoped_lock lock_;

  public:
    explicit Locker(GenericLoadersContext& that) :
      that_(that),
      lock_(that.mutex_)
    {
      if (!that_.scheduler_)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }
    }
      
    virtual ILoadersContext& GetContext() const ORTHANC_OVERRIDE
    {
      return that_;
    };

    virtual void AddLoader(boost::shared_ptr<IObserver> loader) ORTHANC_OVERRIDE
    {
      that_.loaders_.push_back(loader);
    }

    virtual IObservable& GetOracleObservable() const ORTHANC_OVERRIDE
    {
      return that_.oracleObservable_;
    }

    virtual void Schedule(boost::shared_ptr<IObserver> receiver,
                          int priority,
                          IOracleCommand* command /* Takes ownership */) ORTHANC_OVERRIDE
    {
      that_.scheduler_->Schedule(receiver, priority, command);
    };

    virtual void CancelRequests(boost::shared_ptr<IObserver> receiver) ORTHANC_OVERRIDE
    {
      that_.scheduler_->CancelRequests(receiver);
    }

    virtual void CancelAllRequests() ORTHANC_OVERRIDE
    {
      that_.scheduler_->CancelAllRequests();
    }

    virtual void GetStatistics(uint64_t& scheduledCommands,
                               uint64_t& processedCommands) ORTHANC_OVERRIDE
    {
      scheduledCommands = that_.scheduler_->GetTotalScheduled();
      processedCommands = that_.scheduler_->GetTotalProcessed();
    }
  };


  void GenericLoadersContext::EmitMessage(boost::weak_ptr<IObserver> observer,
                                          const IMessage& message)
  {
    boost::recursive_mutex::scoped_lock lock(mutex_);
    //LOG(INFO) << "  inside emit lock: " << message.GetIdentifier().AsString();
    oracleObservable_.EmitMessage(observer, message);
    //LOG(INFO) << "  outside emit lock";
  }


  GenericLoadersContext::GenericLoadersContext(unsigned int maxHighPriority,
                                               unsigned int maxStandardPriority,
                                               unsigned int maxLowPriority)
  {
    oracle_.reset(new ThreadedOracle(*this));
    scheduler_ = OracleScheduler::Create(*oracle_, oracleObservable_, *this,
                                         maxHighPriority, maxStandardPriority, maxLowPriority);

    if (!scheduler_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
  }


  GenericLoadersContext::~GenericLoadersContext()
  {
    LOG(INFO) << "scheduled commands: " << scheduler_->GetTotalScheduled()
                 << ", processed commands: " << scheduler_->GetTotalProcessed();
    scheduler_.reset();
    //LOG(INFO) << "counter: " << scheduler_.use_count();
  }

  
  void GenericLoadersContext::SetOrthancParameters(const Orthanc::WebServiceParameters& parameters)
  {
    boost::recursive_mutex::scoped_lock lock(mutex_);
    oracle_->SetOrthancParameters(parameters);
  }

  
  void GenericLoadersContext::SetRootDirectory(const std::string& root)
  {
    boost::recursive_mutex::scoped_lock lock(mutex_);
    oracle_->SetRootDirectory(root);
  }

  
  void GenericLoadersContext::SetDicomCacheSize(size_t size)
  {
    boost::recursive_mutex::scoped_lock lock(mutex_);
    oracle_->SetDicomCacheSize(size);
  }

  
  void GenericLoadersContext::StartOracle()
  {
    boost::recursive_mutex::scoped_lock lock(mutex_);
    oracle_->Start();
    //LOG(INFO) << "STARTED ORACLE";
  }

  
  void GenericLoadersContext::StopOracle()
  {
    /**
     * DON'T lock "mutex_" here, otherwise Stone won't be able to
     * stop if one command being executed by the oracle has to emit
     * a message (method "EmitMessage()" would have to lock the
     * mutex too).
     **/
      
    //LOG(INFO) << "STOPPING ORACLE";
    oracle_->Stop();
    //LOG(INFO) << "STOPPED ORACLE";
  }

  
  void GenericLoadersContext::WaitUntilComplete()
  {
    for (;;)
    {
      {
        boost::recursive_mutex::scoped_lock lock(mutex_);
        if (scheduler_ &&
            scheduler_->GetTotalScheduled() == scheduler_->GetTotalProcessed())
        {
          return;
        }
      }

      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
  }
   
  ILoadersContext::ILock* GenericLoadersContext::Lock()
  {
    return new Locker(*this);
  }
}

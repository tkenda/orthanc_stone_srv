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


#include "WebAssemblyLoadersContext.h"

namespace OrthancStone
{
  class WebAssemblyLoadersContext::Locker : public ILoadersContext::ILock
  {
  private:
    WebAssemblyLoadersContext&  that_;

  public:
    explicit Locker(WebAssemblyLoadersContext& that) :
      that_(that)
    {
    }      
      
    virtual ILoadersContext& GetContext() const ORTHANC_OVERRIDE
    {
      return that_;
    }

    virtual IObservable& GetOracleObservable() const ORTHANC_OVERRIDE
    {
      return that_.oracle_.GetOracleObservable();
    }

    virtual void Schedule(boost::shared_ptr<IObserver> receiver,
                          int priority,
                          IOracleCommand* command /* Takes ownership */) ORTHANC_OVERRIDE
    {
      that_.scheduler_->Schedule(receiver, priority, command);
    }

    virtual void CancelRequests(boost::shared_ptr<IObserver> receiver) ORTHANC_OVERRIDE
    {
      that_.scheduler_->CancelRequests(receiver);
    }

    virtual void CancelAllRequests() ORTHANC_OVERRIDE
    {
      that_.scheduler_->CancelAllRequests();
    }

    virtual void AddLoader(boost::shared_ptr<IObserver> loader) ORTHANC_OVERRIDE
    {
      that_.loaders_.push_back(loader);
    }

    virtual void GetStatistics(uint64_t& scheduledCommands,
                               uint64_t& processedCommands) ORTHANC_OVERRIDE
    {
      scheduledCommands = that_.scheduler_->GetTotalScheduled();
      processedCommands = that_.scheduler_->GetTotalProcessed();
    }
  };
    

  WebAssemblyLoadersContext::WebAssemblyLoadersContext(unsigned int maxHighPriority,
                                                       unsigned int maxStandardPriority,
                                                       unsigned int maxLowPriority)
  {
    oracle_.GetOracleObservable();
    scheduler_ = OracleScheduler::Create(oracle_, oracle_.GetOracleObservable(), oracle_,
                                         maxHighPriority, maxStandardPriority, maxLowPriority);

    if (!scheduler_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
  }


  ILoadersContext::ILock* WebAssemblyLoadersContext::Lock()
  {
    return new Locker(*this);
  }
}

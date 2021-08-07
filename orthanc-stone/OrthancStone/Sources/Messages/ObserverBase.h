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

#include "ICallable.h"
#include "IObserver.h"
#include "IObservable.h"

#include <OrthancException.h>

#include <boost/enable_shared_from_this.hpp>

namespace OrthancStone 
{
  template <typename TObserver>
  class ObserverBase : 
    public IObserver,
    public boost::enable_shared_from_this<TObserver>
  {
  public:
    boost::shared_ptr<TObserver> GetSharedObserver()
    {
      try
      {
        return this->shared_from_this();
      }
      catch (boost::bad_weak_ptr&)
      {
        throw Orthanc::OrthancException(
          Orthanc::ErrorCode_InternalError,
          "Cannot get a shared pointer to an observer from its constructor, "
          "or the observer is not created as a shared pointer");
      }
    }

    template <typename TMessage>
    ICallable* CreateCallable(void (TObserver::* MemberMethod) (const TMessage&))
    {
      return new Callable<TObserver, TMessage>(GetSharedObserver(), MemberMethod);
    }

    template <typename TMessage>
    void Register(IObservable& observable,
                  void (TObserver::* MemberMethod) (const TMessage&))
    {
      observable.RegisterCallable(CreateCallable(MemberMethod));
    }
  };
}

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

#include "IMessage.h"
#include "IObserver.h"

#include <Logging.h>

#include <boost/noncopyable.hpp>
#include <boost/weak_ptr.hpp>

#include <string>
#include <stdint.h>

namespace OrthancStone 
{
  // This is referencing an object and member function that can be notified
  // by an IObservable.  The object must derive from IO
  // The member functions must be of type "void Method(const IMessage& message)" or reference a derived class of IMessage
  class ICallable : public boost::noncopyable
  {
  public:
    virtual ~ICallable()
    {
    }

    virtual void Apply(const IMessage& message) = 0;

    virtual const MessageIdentifier& GetMessageIdentifier() = 0;

    // TODO - Is this needed?
    virtual boost::weak_ptr<IObserver> GetObserver() const = 0;
  };


  template <typename TObserver,
            typename TMessage>
  class Callable : public ICallable
  {
  private:
    typedef void (TObserver::* MemberMethod) (const TMessage&);

    boost::weak_ptr<IObserver>  observer_;
    MemberMethod                function_;

  public:
    Callable(boost::shared_ptr<TObserver> observer,
             MemberMethod function) :
      observer_(observer),
      function_(function)
    {
    }

    virtual void Apply(const IMessage& message)
    {
      boost::shared_ptr<IObserver> lock(observer_);
      if (lock)
      {
        TObserver& observer = dynamic_cast<TObserver&>(*lock);
        const TMessage& typedMessage = dynamic_cast<const TMessage&>(message);
        (observer.*function_) (typedMessage);
      }
    }

    virtual const MessageIdentifier& GetMessageIdentifier()
    {
      return TMessage::GetStaticIdentifier();
    }

    virtual boost::weak_ptr<IObserver> GetObserver() const
    {
      return observer_;
    }
  };
}

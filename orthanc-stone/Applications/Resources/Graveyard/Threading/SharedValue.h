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


#pragma once

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

namespace OrthancStone
{
  // A value that is protected by a mutex, in order to be shared by
  // multiple threads
  template <typename T>
  class SharedValue : public boost::noncopyable
  {
  private:
    boost::mutex   mutex_;
    T              value_;
    
  public:
    class Locker : public boost::noncopyable
    {
    private:
      boost::mutex::scoped_lock  lock_;
      T&                         value_;

    public:
      Locker(SharedValue& shared) :
        lock_(shared.mutex_),
        value_(shared.value_)
      {
      }

      T& GetValue() const
      {
        return value_;
      }
    };
  };
}

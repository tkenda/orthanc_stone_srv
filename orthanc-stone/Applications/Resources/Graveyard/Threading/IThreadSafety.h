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

namespace OrthancStone
{
  /**
   * Dummy interface to explicitely tag the interfaces whose derived
   * class must be thread-safe. The different methods of such classes
   * might be simlultaneously invoked by several threads, and should
   * be properly protected by mutexes.
   **/
  class IThreadSafe : public boost::noncopyable
  {
  public:
    virtual ~IThreadSafe()
    {
    }
  };


  /**
   * Dummy interface to explicitely tag the interfaces that are NOT
   * expected to be thread-safe. The Orthanc Stone framework ensures
   * that at most one method of such classes will be invoked at a
   * given time. Such classes are automatically protected by the
   * Orthanc Stone framework wherever required.
   **/
  class IThreadUnsafe : public boost::noncopyable
  {
  public:
    virtual ~IThreadUnsafe()
    {
    }
  };
}

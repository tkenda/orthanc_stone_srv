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

#include <boost/noncopyable.hpp>

namespace OrthancStone
{
  class IFetchingStrategy : public boost::noncopyable
  {
  public:
    virtual ~IFetchingStrategy()
    {
    }

    virtual unsigned int GetItemsCount() const = 0;

    virtual unsigned int GetMaxQuality() const = 0;

    virtual bool GetNext(unsigned int& item,
                         unsigned int& quality) = 0;

    virtual void SetCurrent(unsigned int item) = 0;

    // Ask the strategy to re-schedule the item with the lowest
    // priority in the fetching order. This allows to know which item
    // should be dropped from a cache.
    virtual void RecycleFurthest(unsigned int& item) = 0;
  };
};

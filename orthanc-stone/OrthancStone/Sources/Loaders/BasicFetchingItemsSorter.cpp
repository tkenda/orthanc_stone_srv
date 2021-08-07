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


#include "BasicFetchingItemsSorter.h"

#include <OrthancException.h>

#include <algorithm>
#include <cassert>

namespace OrthancStone
{
  BasicFetchingItemsSorter::BasicFetchingItemsSorter(unsigned int itemsCount) :
    itemsCount_(itemsCount)
  {
    if (itemsCount == 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  void BasicFetchingItemsSorter::Sort(std::vector<unsigned int>& target,
                                      unsigned int current)
  {
    if (current >= itemsCount_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    target.clear();
    target.reserve(itemsCount_);
    target.push_back(current);

    const unsigned int countBelow = current;
    const unsigned int countAbove = (itemsCount_ - 1) - current;
    const unsigned int n = std::min(countBelow, countAbove);

    for (unsigned int i = 1; i <= n; i++)
    {
      assert(current + i < itemsCount_ &&
             current >= i);
      target.push_back(current + i);
      target.push_back(current - i);
    }

    for (unsigned int i = current - n; i > 0; i--)
    {
      target.push_back(i - 1);
    }

    for (unsigned int i = current + n + 1; i < itemsCount_; i++)
    {
      target.push_back(i);
    }

    assert(target.size() == itemsCount_);
  }
}

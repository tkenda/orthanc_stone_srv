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


#include "BasicFetchingStrategy.h"

#include <OrthancException.h>

#include <cassert>

namespace OrthancStone
{
  void BasicFetchingStrategy::Schedule(unsigned int item,
                                       unsigned int quality)
  {
    assert(item < GetItemsCount() &&
           quality <= maxQuality_);
      
    if (nextQuality_[item] <= quality)
    {
      content_.push_back(ContentItem(item, quality));
    }
  }
    

  BasicFetchingStrategy::BasicFetchingStrategy(IFetchingItemsSorter* sorter,   // Takes ownership
                                               unsigned int maxQuality,
                                               unsigned int initialItem) :
    sorter_(sorter),
    maxQuality_(maxQuality),
    position_(0),
    blockSize_(2)
  {
    if (sorter == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }

    nextQuality_.resize(sorter_->GetItemsCount(), 0);   // Does not change along calls to "SetCurrent()"
      
    SetCurrent(initialItem);
  }


  void BasicFetchingStrategy::SetBlockSize(unsigned int size)
  {
    if (size == 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    blockSize_ = size;
  }

  
  bool BasicFetchingStrategy::GetNext(unsigned int& item,
                                      unsigned int& quality)
  {
    if (position_ >= content_.size())
    {
      return false;
    }
    else
    {
      item = content_[position_].GetItem();       
      quality = content_[position_].GetQuality();

      assert(nextQuality_[item] <= quality);
      nextQuality_[item] = quality + 1;

      position_ ++;
      return true;
    }
  }

  
  void BasicFetchingStrategy::SetCurrent(unsigned int item)
  {
    // TODO - This function is O(N) complexity where "N" is the
    // number of items times the max quality. Could use a LRU index.

    position_ = 0;
      
    std::vector<unsigned int> v;
    sorter_->Sort(v, item);

    assert(v.size() == GetItemsCount());

    if (v.size() == 0)
    {
      return;
    }
      
    content_.clear();
    content_.reserve(v.size() * maxQuality_);

    Schedule(v.front(), maxQuality_);

    for (unsigned int q = 0; q <= maxQuality_; q++)
    {
      unsigned int start = 1 + q * blockSize_;
      unsigned int end = start + blockSize_;

      if (q == maxQuality_ ||
          end > v.size())
      {
        end = static_cast<int>(v.size());
      }

      unsigned int a = 0;
      if (maxQuality_ >= q + 1)
      {
        a = maxQuality_ - q - 1;
      }
        
      for (unsigned int j = a; j <= maxQuality_; j++)
      {
        for (unsigned int i = start; i < end; i++)
        {
          Schedule(v[i], j);
        }
      }
    }
  }

  
  void BasicFetchingStrategy::RecycleFurthest(unsigned int& item)
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
  }
}

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

#include "IFetchingItemsSorter.h"
#include "IFetchingStrategy.h"

#include <Compatibility.h>

#include <memory>

namespace OrthancStone
{
  class BasicFetchingStrategy : public IFetchingStrategy
  {
  private:
    class ContentItem
    {
    private:
      unsigned int  item_;
      unsigned int  quality_;

    public:
      ContentItem(unsigned int item,
           unsigned int quality) :
        item_(item),
        quality_(quality)
      {
      }

      unsigned int GetItem() const
      {
        return item_;
      }

      unsigned int GetQuality() const
      {
        return quality_;
      }
    };

    std::unique_ptr<IFetchingItemsSorter>  sorter_;
    std::vector<unsigned int>            nextQuality_;
    unsigned int                         maxQuality_;
    std::vector<ContentItem>             content_;
    size_t                               position_;
    unsigned int                         blockSize_;

    void Schedule(unsigned int item,
                  unsigned int quality);
    
  public:
    BasicFetchingStrategy(IFetchingItemsSorter* sorter,   // Takes ownership
                          unsigned int maxQuality,
                          unsigned int initialItem = 0);

    virtual unsigned int GetItemsCount() const ORTHANC_OVERRIDE
    {
      return sorter_->GetItemsCount();
    }

    virtual unsigned int GetMaxQuality() const ORTHANC_OVERRIDE
    {
      return maxQuality_;
    }

    // WARNING - This parameters is only considered during the next
    // call to SetCurrent().
    void SetBlockSize(unsigned int size);

    virtual bool GetNext(unsigned int& item,
                         unsigned int& quality) ORTHANC_OVERRIDE;
    
    virtual void SetCurrent(unsigned int item) ORTHANC_OVERRIDE;

    virtual void RecycleFurthest(unsigned int& item) ORTHANC_OVERRIDE;
  };
}

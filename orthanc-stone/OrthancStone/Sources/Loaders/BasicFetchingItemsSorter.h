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

#include <Compatibility.h>  // For ORTHANC_OVERRIDE

namespace OrthancStone
{
  class BasicFetchingItemsSorter : public IFetchingItemsSorter
  {
  private:
    unsigned int  itemsCount_;

  public:
    class Factory : public IFactory
    {
    public:
      virtual IFetchingItemsSorter* CreateSorter(unsigned int itemsCount) const ORTHANC_OVERRIDE
      {
        return new BasicFetchingItemsSorter(itemsCount);
      }
    };

    explicit BasicFetchingItemsSorter(unsigned int itemsCount);

    virtual unsigned int GetItemsCount() const ORTHANC_OVERRIDE
    {
      return itemsCount_;
    }

    virtual void Sort(std::vector<unsigned int>& target,
                      unsigned int current) ORTHANC_OVERRIDE;
  };
}

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

#include <vector> 

#include "../StoneException.h"

namespace OrthancStone
{
  class DisjointDataSet
  {
  public:
    DisjointDataSet(size_t itemCount) :
      parents_(itemCount),
      ranks_(itemCount)
    {
      for (size_t index = 0; index < parents_.size(); index++)
      {
        SetParent(index,index);
        ranks_[index] = 1;
      }
    }

    size_t Find(size_t item)
    {
      /*
      If parents_[i] == i, it means i is representative of a set.
      Otherwise, we go up the tree...
      */
      if (GetParent(item) != item)
      {
        // if item is not a top item (representative of its set),
        // we use path compression to improve future lookups 
        // see: https://en.wikipedia.org/wiki/Disjoint-set_data_structure#Path_compression
        SetParent(item, Find(parents_[item]));
      }

      // now that paths have been compressed, we are positively certain
      // that item's parent is a set ("X is a set" means that X is the 
      // representative of a set)
      return GetParent(item);
    }

    /*
    This merge the two sets that contains itemA and itemB
    */
    void Union(size_t itemA, size_t itemB)
    {
      // Find current sets of x and y 
      size_t setA = Find(itemA);
      size_t setB = Find(itemB);

      // if setA == setB, it means they are already in the same set and 
      // do not need to be merged!
      if (setA != setB)
      {
        // we need to merge the sets, which means that the trees representing
        // the sets needs to be merged (there must be a single top parent to 
        // all the items originally belonging to setA and setB must be the same)

        // since the algorithm speed is inversely proportional to the tree
        // height (the rank), we need to combine trees in a way that 
        // minimizes this rank. See "Union by rank" at 
        // https://en.wikipedia.org/wiki/Disjoint-set_data_structure#by_rank
        if (GetRank(setA) < GetRank(setB))
        {
          SetParent(setA, setB);
        }
        else if (GetRank(setA) > GetRank(setB))
        {
          SetParent(setB, setA);
        }
        else
        {
          SetParent(setB, setA);
          BumpRank(setA);
          // the trees had the same height but we attached the whole of setB
          // under setA (under its parent), so the resulting tree is now 
          // 1 higher. setB is NOT representative of a set anymore.
        }
      }
    }

  private:
    size_t GetRank(size_t i) const
    {
      ORTHANC_ASSERT(i < ranks_.size());
      ORTHANC_ASSERT(ranks_.size() == parents_.size());
      return ranks_[i];
    }

    size_t GetParent(size_t i) const
    {
      ORTHANC_ASSERT(i < parents_.size());
      ORTHANC_ASSERT(ranks_.size() == parents_.size());
      return parents_[i];
    }

    void SetParent(size_t i, size_t parent)
    {
      ORTHANC_ASSERT(i < parents_.size());
      ORTHANC_ASSERT(ranks_.size() == parents_.size());
      parents_[i] = parent;
    }

    void BumpRank(size_t i)
    {
      ORTHANC_ASSERT(i < ranks_.size());
      ORTHANC_ASSERT(ranks_.size() == parents_.size());
      ranks_[i] = ranks_[i] + 1u;
    }

    /*
    This vector contains the direct parent of each item
    */
    std::vector<size_t> parents_;

    /*
    This vector contains the tree height of each set. The values in the
    vector for non-representative items is UNDEFINED!
    */
    std::vector<size_t> ranks_;
  };

}

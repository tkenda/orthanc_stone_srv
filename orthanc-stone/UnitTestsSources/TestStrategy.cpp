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


#include <gtest/gtest.h>

#include "../OrthancStone/Sources/Loaders/BasicFetchingStrategy.h"
#include "../OrthancStone/Sources/Loaders/BasicFetchingItemsSorter.h"

#include <OrthancException.h>


namespace
{
  class StrategyTester : public boost::noncopyable
  {
  private:
    std::map<unsigned int, unsigned int>  qualities_;

  public:
    bool IsValidCommand(unsigned int item,
                        unsigned int quality)
    {
      if (qualities_.find(item) != qualities_.end() &&
          qualities_[item] >= quality)
      {
        return false;
      }
      else
      {
        qualities_[item] = quality;
        return true;
      }
    }

    bool HasFinished(OrthancStone::BasicFetchingStrategy& strategy)
    {
      for (unsigned int i = 0; i < strategy.GetItemsCount(); i++)
      {
        if (qualities_.find(i) == qualities_.end() ||
            qualities_[i] != strategy.GetMaxQuality())
        {
          return false;
        }
      }

      return true;
    }
  };
}


TEST(BasicFetchingStrategy, Test1)
{
  ASSERT_THROW(OrthancStone::BasicFetchingStrategy(NULL, 0), Orthanc::OrthancException);
  ASSERT_THROW(OrthancStone::BasicFetchingStrategy(new OrthancStone::BasicFetchingItemsSorter(0), 0), Orthanc::OrthancException);

  {
    OrthancStone::BasicFetchingStrategy s(new OrthancStone::BasicFetchingItemsSorter(1), 0);
    unsigned int i, q;
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(0u, i);  ASSERT_EQ(0u, q);
    ASSERT_FALSE(s.GetNext(i, q));
  }

  {
    OrthancStone::BasicFetchingStrategy s(new OrthancStone::BasicFetchingItemsSorter(1), 5);
    unsigned int i, q;
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(0u, i);  ASSERT_EQ(5u, q);
    ASSERT_FALSE(s.GetNext(i, q));
  }

  {
    OrthancStone::BasicFetchingStrategy s(new OrthancStone::BasicFetchingItemsSorter(2), 2);
    unsigned int i, q;
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(0u, i);  ASSERT_EQ(2u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(1u, i);  ASSERT_EQ(1u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(1u, i);  ASSERT_EQ(2u, q);
    ASSERT_FALSE(s.GetNext(i, q));
  }

  {
    OrthancStone::BasicFetchingStrategy s(new OrthancStone::BasicFetchingItemsSorter(3), 2);
    unsigned int i, q;
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(0u, i);  ASSERT_EQ(2u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(1u, i);  ASSERT_EQ(1u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(2u, i);  ASSERT_EQ(1u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(1u, i);  ASSERT_EQ(2u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(2u, i);  ASSERT_EQ(2u, q);
    ASSERT_FALSE(s.GetNext(i, q));
  }

  {
    OrthancStone::BasicFetchingStrategy s(new OrthancStone::BasicFetchingItemsSorter(3), 2);
    s.SetBlockSize(1);
    s.SetCurrent(0);
    unsigned int i, q;
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(0u, i);  ASSERT_EQ(2u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(1u, i);  ASSERT_EQ(1u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(1u, i);  ASSERT_EQ(2u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(2u, i);  ASSERT_EQ(0u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(2u, i);  ASSERT_EQ(1u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(2u, i);  ASSERT_EQ(2u, q);
    ASSERT_FALSE(s.GetNext(i, q));
  }

  {
    OrthancStone::BasicFetchingStrategy s(new OrthancStone::BasicFetchingItemsSorter(5), 0);
    ASSERT_THROW(s.SetCurrent(5), Orthanc::OrthancException);
    s.SetCurrent(2);

    unsigned int i, q;
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(2u, i);  ASSERT_EQ(0u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(3u, i);  ASSERT_EQ(0u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(1u, i);  ASSERT_EQ(0u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(4u, i);  ASSERT_EQ(0u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(0u, i);  ASSERT_EQ(0u, q);
    ASSERT_FALSE(s.GetNext(i, q));
  }

  {
    OrthancStone::BasicFetchingStrategy s(new OrthancStone::BasicFetchingItemsSorter(5), 0);
    s.SetCurrent(4);

    unsigned int i, q;
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(4u, i);  ASSERT_EQ(0u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(3u, i);  ASSERT_EQ(0u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(2u, i);  ASSERT_EQ(0u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(1u, i);  ASSERT_EQ(0u, q);
    ASSERT_TRUE(s.GetNext(i, q));   ASSERT_EQ(0u, i);  ASSERT_EQ(0u, q);
    ASSERT_FALSE(s.GetNext(i, q));
  }
}


TEST(BasicFetchingStrategy, Test2)
{
  OrthancStone::BasicFetchingStrategy s(new OrthancStone::BasicFetchingItemsSorter(20), 2);
  ASSERT_EQ(20u, s.GetItemsCount());
  ASSERT_EQ(2u, s.GetMaxQuality());

  StrategyTester t;

  s.SetCurrent(10);

  unsigned int i, q;
  while (s.GetNext(i, q))
  {
    ASSERT_TRUE(t.IsValidCommand(i, q));
  }

  ASSERT_TRUE(t.HasFinished(s));
}




TEST(BasicFetchingItemsSorter, Small)
{
  ASSERT_THROW(OrthancStone::BasicFetchingItemsSorter(0), Orthanc::OrthancException);
  std::vector<unsigned int> v;

  {
    OrthancStone::BasicFetchingItemsSorter s(1);
    s.Sort(v, 0);
    ASSERT_EQ(1u, v.size());
    ASSERT_EQ(0u, v[0]);

    ASSERT_THROW(s.Sort(v, 1), Orthanc::OrthancException);
  }

  {
    OrthancStone::BasicFetchingItemsSorter s(2);
    s.Sort(v, 0);
    ASSERT_EQ(2u, v.size());
    ASSERT_EQ(0u, v[0]);
    ASSERT_EQ(1u, v[1]);

    s.Sort(v, 1);
    ASSERT_EQ(2u, v.size());
    ASSERT_EQ(1u, v[0]);
    ASSERT_EQ(0u, v[1]);

    ASSERT_THROW(s.Sort(v, 2), Orthanc::OrthancException);
  }

  {
    OrthancStone::BasicFetchingItemsSorter s(3);
    s.Sort(v, 0);
    ASSERT_EQ(3u, v.size());
    ASSERT_EQ(0u, v[0]);
    ASSERT_EQ(1u, v[1]);
    ASSERT_EQ(2u, v[2]);
    
    s.Sort(v, 1);
    ASSERT_EQ(3u, v.size());
    ASSERT_EQ(1u, v[0]);
    ASSERT_EQ(2u, v[1]);
    ASSERT_EQ(0u, v[2]);
    
    s.Sort(v, 2);
    ASSERT_EQ(3u, v.size());
    ASSERT_EQ(2u, v[0]);
    ASSERT_EQ(1u, v[1]);
    ASSERT_EQ(0u, v[2]);
    
    ASSERT_THROW(s.Sort(v, 3), Orthanc::OrthancException);
  }
}


TEST(BasicFetchingItemsSorter, Odd)
{
  OrthancStone::BasicFetchingItemsSorter s(7);
  std::vector<unsigned int> v;

  ASSERT_THROW(s.Sort(v, 7), Orthanc::OrthancException);

  {
    s.Sort(v, 0);
    ASSERT_EQ(7u, v.size());
    ASSERT_EQ(0u, v[0]);
    ASSERT_EQ(1u, v[1]);
    ASSERT_EQ(2u, v[2]);
    ASSERT_EQ(3u, v[3]);
    ASSERT_EQ(4u, v[4]);
    ASSERT_EQ(5u, v[5]);
    ASSERT_EQ(6u, v[6]);
  }

  {
    s.Sort(v, 1);
    ASSERT_EQ(7u, v.size());
    ASSERT_EQ(1u, v[0]);
    ASSERT_EQ(2u, v[1]);
    ASSERT_EQ(0u, v[2]);
    ASSERT_EQ(3u, v[3]);
    ASSERT_EQ(4u, v[4]);
    ASSERT_EQ(5u, v[5]);
    ASSERT_EQ(6u, v[6]);
  }

  {
    s.Sort(v, 2);
    ASSERT_EQ(7u, v.size());
    ASSERT_EQ(2u, v[0]);
    ASSERT_EQ(3u, v[1]);
    ASSERT_EQ(1u, v[2]);
    ASSERT_EQ(4u, v[3]);
    ASSERT_EQ(0u, v[4]);
    ASSERT_EQ(5u, v[5]);
    ASSERT_EQ(6u, v[6]);
  }

  {
    s.Sort(v, 3);
    ASSERT_EQ(7u, v.size());
    ASSERT_EQ(3u, v[0]);
    ASSERT_EQ(4u, v[1]);
    ASSERT_EQ(2u, v[2]);
    ASSERT_EQ(5u, v[3]);
    ASSERT_EQ(1u, v[4]);
    ASSERT_EQ(6u, v[5]);
    ASSERT_EQ(0u, v[6]);
  }

  {
    s.Sort(v, 4);
    ASSERT_EQ(7u, v.size());
    ASSERT_EQ(4u, v[0]);
    ASSERT_EQ(5u, v[1]);
    ASSERT_EQ(3u, v[2]);
    ASSERT_EQ(6u, v[3]);
    ASSERT_EQ(2u, v[4]);
    ASSERT_EQ(1u, v[5]);
    ASSERT_EQ(0u, v[6]);
  }

  {
    s.Sort(v, 5);
    ASSERT_EQ(7u, v.size());
    ASSERT_EQ(5u, v[0]);
    ASSERT_EQ(6u, v[1]);
    ASSERT_EQ(4u, v[2]);
    ASSERT_EQ(3u, v[3]);
    ASSERT_EQ(2u, v[4]);
    ASSERT_EQ(1u, v[5]);
    ASSERT_EQ(0u, v[6]);
  }

  {
    s.Sort(v, 6);
    ASSERT_EQ(7u, v.size());
    ASSERT_EQ(6u, v[0]);
    ASSERT_EQ(5u, v[1]);
    ASSERT_EQ(4u, v[2]);
    ASSERT_EQ(3u, v[3]);
    ASSERT_EQ(2u, v[4]);
    ASSERT_EQ(1u, v[5]);
    ASSERT_EQ(0u, v[6]);
  }
}


TEST(BasicFetchingItemsSorter, Even)
{
  OrthancStone::BasicFetchingItemsSorter s(6);
  std::vector<unsigned int> v;

  {
    s.Sort(v, 0);
    ASSERT_EQ(6u, v.size());
    ASSERT_EQ(0u, v[0]);
    ASSERT_EQ(1u, v[1]);
    ASSERT_EQ(2u, v[2]);
    ASSERT_EQ(3u, v[3]);
    ASSERT_EQ(4u, v[4]);
    ASSERT_EQ(5u, v[5]);
  }

  {
    s.Sort(v, 1);
    ASSERT_EQ(6u, v.size());
    ASSERT_EQ(1u, v[0]);
    ASSERT_EQ(2u, v[1]);
    ASSERT_EQ(0u, v[2]);
    ASSERT_EQ(3u, v[3]);
    ASSERT_EQ(4u, v[4]);
    ASSERT_EQ(5u, v[5]);
  }

  {
    s.Sort(v, 2);
    ASSERT_EQ(6u, v.size());
    ASSERT_EQ(2u, v[0]);
    ASSERT_EQ(3u, v[1]);
    ASSERT_EQ(1u, v[2]);
    ASSERT_EQ(4u, v[3]);
    ASSERT_EQ(0u, v[4]);
    ASSERT_EQ(5u, v[5]);
  }

  {
    s.Sort(v, 3);
    ASSERT_EQ(6u, v.size());
    ASSERT_EQ(3u, v[0]);
    ASSERT_EQ(4u, v[1]);
    ASSERT_EQ(2u, v[2]);
    ASSERT_EQ(5u, v[3]);
    ASSERT_EQ(1u, v[4]);
    ASSERT_EQ(0u, v[5]);
  }

  {
    s.Sort(v, 4);
    ASSERT_EQ(6u, v.size());
    ASSERT_EQ(4u, v[0]);
    ASSERT_EQ(5u, v[1]);
    ASSERT_EQ(3u, v[2]);
    ASSERT_EQ(2u, v[3]);
    ASSERT_EQ(1u, v[4]);
    ASSERT_EQ(0u, v[5]);
  }

  {
    s.Sort(v, 5);
    ASSERT_EQ(6u, v.size());
    ASSERT_EQ(5u, v[0]);
    ASSERT_EQ(4u, v[1]);
    ASSERT_EQ(3u, v[2]);
    ASSERT_EQ(2u, v[3]);
    ASSERT_EQ(1u, v[4]);
    ASSERT_EQ(0u, v[5]);
  }
}

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

#include "../OrthancStone/Sources/Toolbox/ImageToolbox.h"

// #include <boost/chrono.hpp>
// #include <boost/lexical_cast.hpp>

#include <Compatibility.h>
#include <Images/Image.h>
#include <Images/PixelTraits.h>

#include "stdint.h"

#include <gtest/gtest.h>

#include <cmath>


TEST(ImageToolbox, SimpleHisto_Grayscale8_BinSize1)
{
  using OrthancStone::HistogramData;
  using OrthancStone::DumpHistogramResult;
  using OrthancStone::ComputeHistogram;

  const unsigned int W = 16;
  const unsigned int H = 16;

  // 256/17 = 15,...
  // 256 % 17 = 1
  // 0 will be 16 times
  // 1 will be 15 times
  // 2 will be 15 times
  // ...
  // 16 will be 15 times

  size_t pixCounter = 0;

  std::unique_ptr<Orthanc::Image> image(new Orthanc::Image(
    Orthanc::PixelFormat_Grayscale8, W, H, false));

  for (unsigned int y = 0; y < H; ++y)
  {
    uint8_t* buffer = reinterpret_cast<uint8_t*>(image->GetRow(y));
    for (unsigned int x = 0; x < W; ++x, ++buffer, ++pixCounter)
    {
      *buffer = static_cast<uint8_t>(pixCounter % 17);
    }
  }

  HistogramData hd;
  ComputeHistogram(*image, hd, 1);
  ASSERT_EQ(-0.5, hd.minValue);
  ASSERT_EQ(17u, hd.bins.size());
  ASSERT_EQ(16u, hd.bins[0]);
  for (size_t i = 1; i < hd.bins.size(); ++i)
    ASSERT_EQ(15u, hd.bins[i]);
}

TEST(ImageToolbox, SimpleHisto_Grayscale8_BinSize1_FormatString)
{
  using OrthancStone::HistogramData;
  using OrthancStone::DumpHistogramResult;
  using OrthancStone::ComputeHistogram;

  const unsigned int W = 16;
  const unsigned int H = 16;

  // 256/17 = 15,...
  // 256 % 17 = 1
  // 0 will be 16 times
  // 1 will be 15 times
  // 2 will be 15 times
  // ...
  // 16 will be 15 times

  size_t pixCounter = 0;

  std::unique_ptr<Orthanc::Image> image(new Orthanc::Image(
    Orthanc::PixelFormat_Grayscale8, W, H, false));

  for (unsigned int y = 0; y < H; ++y)
  {
    uint8_t* buffer = reinterpret_cast<uint8_t*>(image->GetRow(y));
    for (unsigned int x = 0; x < W; ++x, ++buffer, ++pixCounter)
    {
      *buffer = static_cast<uint8_t>(pixCounter % 17);
    }
  }

  HistogramData hd;
  ComputeHistogram(*image, hd, 1);

  // void DumpHistogramResult(std::string& s, const HistogramData& hd)
  std::string s;
  DumpHistogramResult(s, hd);
  std::cout << s;
}

template<Orthanc::PixelFormat Format>
void SimpleHisto_T_BinSize1_2()
{
  using OrthancStone::HistogramData;
  using OrthancStone::DumpHistogramResult;
  using OrthancStone::ComputeHistogram;

  const unsigned int W = 16;
  const unsigned int H = 16;

  // 256/17 = 15,...
  // 256 % 17 = 1
  // 0 will be 16 times
  // 1 will be 15 times
  // 2 will be 15 times
  // ...
  // 16 will be 15 times

  size_t pixCounter = 0;

  std::unique_ptr<Orthanc::Image> image(new Orthanc::Image(
    Format, W, H, false));

  typedef typename Orthanc::PixelTraits<Format>::PixelType PixelType;

  PixelType pixValue = 0;

  for (unsigned int y = 0; y < H; ++y)
  {
    PixelType* buffer = reinterpret_cast<PixelType*>(image->GetRow(y));
    for (unsigned int x = 0; x < W; ++x, ++buffer, ++pixCounter)
    {
      // 0..99 0..99 0..55
      *buffer = pixValue;
      pixValue++;
      if (pixValue >= 100)
        pixValue = 0;
    }
  }

  HistogramData hd;
  ComputeHistogram(*image, hd, 1);
  ASSERT_EQ(-0.5, hd.minValue);
  ASSERT_EQ(100u, hd.bins.size());
  for (size_t i = 0; i <= 55; ++i)
    ASSERT_EQ(3u, hd.bins[i]);
  for (size_t i = 56; i <= 99; ++i)
    ASSERT_EQ(2u, hd.bins[i]);
}

TEST(ImageToolbox, SimpleHisto_Grayscale8_BinSize1_2)
{               
  SimpleHisto_T_BinSize1_2<Orthanc::PixelFormat_Grayscale8>();
}

TEST(ImageToolbox, SimpleHisto_Grayscale16_BinSize1_2)
{
  SimpleHisto_T_BinSize1_2<Orthanc::PixelFormat_Grayscale16>();
}

TEST(ImageToolbox, SimpleHisto_SignedGrayscale16_BinSize1_2)
{
  SimpleHisto_T_BinSize1_2<Orthanc::PixelFormat_SignedGrayscale16>();
}

TEST(ImageToolbox, SimpleHisto_Grayscale32_BinSize1_2)
{
  SimpleHisto_T_BinSize1_2<Orthanc::PixelFormat_Grayscale32>();
}

template<Orthanc::PixelFormat Format>
void SimpleHisto_T_BinSize10_2()
{
  using OrthancStone::HistogramData;
  using OrthancStone::DumpHistogramResult;
  using OrthancStone::ComputeHistogram;

  const unsigned int W = 16;
  const unsigned int H = 16;

  // 256/17 = 15,...
  // 256 % 17 = 1
  // 0 will be 16 times
  // 1 will be 15 times
  // 2 will be 15 times
  // ...
  // 16 will be 15 times

  size_t pixCounter = 0;

  std::unique_ptr<Orthanc::Image> image(new Orthanc::Image(
    Format, W, H, false));

  typedef typename Orthanc::PixelTraits<Format>::PixelType PixelType;

  PixelType pixValue = 0;

  for (unsigned int y = 0; y < H; ++y)
  {
    PixelType* buffer = reinterpret_cast<PixelType*>(image->GetRow(y));
    for (unsigned int x = 0; x < W; ++x, ++buffer, ++pixCounter)
    {
      // 0..99 0..99 0..55
      *buffer = pixValue;
      pixValue++;
      if (pixValue >= 100)
        pixValue = 0;
    }
  }

  HistogramData hd;
  ComputeHistogram(*image, hd, 10);
  ASSERT_EQ(-0.5, hd.minValue);
  ASSERT_EQ(10u, hd.bins.size());

  for (size_t i = 0; i <= 4; ++i)
    ASSERT_EQ(30u, hd.bins[i]);

  ASSERT_EQ(26u, hd.bins[5]);

  for (size_t i = 6; i <= 9; ++i)
    ASSERT_EQ(20u, hd.bins[i]);
}

TEST(ImageToolbox, SimpleHisto_Grayscale8_BinSize10_2)
{
  SimpleHisto_T_BinSize10_2<Orthanc::PixelFormat_Grayscale8>();
}

TEST(ImageToolbox, SimpleHisto_Grayscale16_BinSize10_2)
{
  SimpleHisto_T_BinSize10_2<Orthanc::PixelFormat_Grayscale16>();
}

TEST(ImageToolbox, SimpleHisto_SignedGrayscale16_BinSize10_2)
{
  SimpleHisto_T_BinSize10_2<Orthanc::PixelFormat_SignedGrayscale16>();
}

TEST(ImageToolbox, SimpleHisto_Grayscale32_BinSize10_2)
{
  SimpleHisto_T_BinSize10_2<Orthanc::PixelFormat_Grayscale32>();
}


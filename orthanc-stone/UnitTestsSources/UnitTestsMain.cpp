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

#include "../OrthancStone/Sources/StoneEnumerations.h"
#include "../OrthancStone/Sources/StoneInitialization.h"

#include <Logging.h>

#if defined(__EMSCRIPTEN__)
#  include <emscripten.h>
#endif



TEST(Enumerations, Basic)
{
  using namespace OrthancStone;
  ASSERT_EQ(SopClassUid_EncapsulatedPdf, StringToSopClassUid("1.2.840.10008.5.1.4.1.1.104.1"));
  ASSERT_EQ(SopClassUid_RTStruct, StringToSopClassUid("1.2.840.10008.5.1.4.1.1.481.3"));
  ASSERT_EQ(SopClassUid_RTDose, StringToSopClassUid("1.2.840.10008.5.1.4.1.1.481.2"));
  ASSERT_EQ(SopClassUid_RTPlan, StringToSopClassUid("1.2.840.10008.5.1.4.1.1.481.5"));
  ASSERT_EQ(SopClassUid_VideoEndoscopicImageStorage, StringToSopClassUid("1.2.840.10008.5.1.4.1.1.77.1.1.1"));
  ASSERT_EQ(SopClassUid_VideoMicroscopicImageStorage, StringToSopClassUid("1.2.840.10008.5.1.4.1.1.77.1.2.1"));
  ASSERT_EQ(SopClassUid_VideoPhotographicImageStorage, StringToSopClassUid("1.2.840.10008.5.1.4.1.1.77.1.4.1"));
  ASSERT_EQ(SopClassUid_Other, StringToSopClassUid("nope"));

  ASSERT_EQ(SeriesThumbnailType_Pdf, GetSeriesThumbnailType(SopClassUid_EncapsulatedPdf));
  ASSERT_EQ(SeriesThumbnailType_Video, GetSeriesThumbnailType(SopClassUid_VideoEndoscopicImageStorage));
  ASSERT_EQ(SeriesThumbnailType_Video, GetSeriesThumbnailType(SopClassUid_VideoMicroscopicImageStorage));
  ASSERT_EQ(SeriesThumbnailType_Video, GetSeriesThumbnailType(SopClassUid_VideoPhotographicImageStorage));
  ASSERT_EQ(SeriesThumbnailType_Unsupported, GetSeriesThumbnailType(SopClassUid_Other));
  ASSERT_EQ(SeriesThumbnailType_Unsupported, GetSeriesThumbnailType(SopClassUid_RTDose));
  ASSERT_EQ(SeriesThumbnailType_Unsupported, GetSeriesThumbnailType(SopClassUid_RTStruct));
  ASSERT_EQ(SeriesThumbnailType_Unsupported, GetSeriesThumbnailType(SopClassUid_RTPlan));
}


int main(int argc, char **argv)
{
#if defined(__EMSCRIPTEN__)
  std::string output;
#endif
  
  int result;

  {
    OrthancStone::StoneInitialize();
    Orthanc::Logging::EnableInfoLevel(true);

    ::testing::InitGoogleTest(&argc, argv);
    
#if defined(__EMSCRIPTEN__)
    ::testing::internal::CaptureStdout();
#endif
  
    result = RUN_ALL_TESTS();

#if defined(__EMSCRIPTEN__)
    output = testing::internal::GetCapturedStdout();
#endif

    OrthancStone::StoneFinalize();
  }

#if defined(__EMSCRIPTEN__)
  EM_ASM({
      document.getElementById("output").innerHTML = UTF8ToString($0);
      window.scrollTo(0, document.body.scrollHeight); // Scroll to the end of the page
    },
    output.c_str());
#endif

  return result;
}

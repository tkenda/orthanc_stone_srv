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


// Make sure to define FRIEND_TEST before including "SortedFrames.h"
#include <gtest/gtest.h>

#include "../OrthancStone/Sources/Toolbox/SortedFrames.h"

#include <OrthancException.h>


// Namespace is necessary for FRIEND_TEST() to work in "SortedFrames.h"
namespace OrthancStone
{
  TEST(SortedFrames, Basic)
  {
    SortedFrames f;
    ASSERT_TRUE(f.GetStudyInstanceUid().empty());
    ASSERT_TRUE(f.GetSeriesInstanceUid().empty());
    ASSERT_EQ(0u, f.GetInstancesCount());
    ASSERT_THROW(f.GetInstance(0).GetTags(), Orthanc::OrthancException);
    ASSERT_THROW(f.GetInstance(0).GetSopInstanceUid(), Orthanc::OrthancException);
    ASSERT_TRUE(f.IsSorted());
    ASSERT_EQ(0u, f.GetFramesCount());
    ASSERT_THROW(f.GetInstanceOfFrame(0).GetTags(), Orthanc::OrthancException);
    ASSERT_THROW(f.GetInstanceOfFrame(0).GetSopInstanceUid(), Orthanc::OrthancException);
    ASSERT_THROW(f.GetInstanceOfFrame(0).GetNumberOfFrames(), Orthanc::OrthancException);
    ASSERT_THROW(f.GetFrameNumberInInstance(0), Orthanc::OrthancException);

    Orthanc::DicomMap tags;
    ASSERT_THROW(f.AddInstance(tags), Orthanc::OrthancException);
    tags.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, "study", false);
    ASSERT_THROW(f.AddInstance(tags), Orthanc::OrthancException);
    tags.SetValue(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, "series", false);
    ASSERT_THROW(f.AddInstance(tags), Orthanc::OrthancException);
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop", false);
    f.AddInstance(tags);

    ASSERT_EQ("study", f.GetStudyInstanceUid());
    ASSERT_EQ("series", f.GetSeriesInstanceUid());
    ASSERT_EQ(1u, f.GetInstancesCount());
    std::string s;
    ASSERT_TRUE(f.GetInstance(0).GetTags().LookupStringValue(s, Orthanc::DICOM_TAG_SOP_INSTANCE_UID, false));
    ASSERT_EQ("sop", s);
    ASSERT_EQ("sop", f.GetInstance(0).GetSopInstanceUid());
    ASSERT_FALSE(f.IsSorted());
    ASSERT_THROW(f.GetFramesCount(), Orthanc::OrthancException);
    ASSERT_THROW(f.GetInstanceOfFrame(0).GetTags(), Orthanc::OrthancException);
    ASSERT_THROW(f.GetInstanceOfFrame(0).GetSopInstanceUid(), Orthanc::OrthancException);
    ASSERT_THROW(f.GetInstanceOfFrame(0).GetNumberOfFrames(), Orthanc::OrthancException);
    ASSERT_THROW(f.GetFrameNumberInInstance(0), Orthanc::OrthancException);

    f.Sort();
    ASSERT_TRUE(f.IsSorted());
    ASSERT_EQ(1u, f.GetFramesCount());
    ASSERT_TRUE(f.GetInstanceOfFrame(0).GetTags().LookupStringValue(s, Orthanc::DICOM_TAG_SOP_INSTANCE_UID, false));
    ASSERT_EQ("sop", s);
    ASSERT_EQ("sop", f.GetInstanceOfFrame(0).GetSopInstanceUid());
    ASSERT_EQ(1u, f.GetInstanceOfFrame(0).GetNumberOfFrames());
    ASSERT_EQ(0u, f.GetFrameNumberInInstance(0));
    ASSERT_THROW(f.GetInstanceOfFrame(1).GetTags(), Orthanc::OrthancException);
  }


  TEST(SortedFrames, SortSopInstanceUid)
  {
    Orthanc::DicomMap tags;
    tags.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, "study", false);
    tags.SetValue(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, "series", false);
    
    SortedFrames f;
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop3", false);
    tags.SetValue(Orthanc::DICOM_TAG_NUMBER_OF_FRAMES, "1", false);
    f.AddInstance(tags);
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop1", false);
    tags.SetValue(Orthanc::DICOM_TAG_NUMBER_OF_FRAMES, "3", false);
    f.AddInstance(tags);
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop2", false);
    tags.SetValue(Orthanc::DICOM_TAG_NUMBER_OF_FRAMES, "2", false);
    f.AddInstance(tags);

    size_t i;
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop3"));  ASSERT_EQ(0u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop1"));  ASSERT_EQ(1u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop2"));  ASSERT_EQ(2u, i);
    ASSERT_FALSE(f.LookupSopInstanceUid(i, "nope"));

    ASSERT_THROW(f.LookupFrame(i, "sop3", 0), Orthanc::OrthancException);  // Not sorted yet
  
    f.Sort();
    ASSERT_EQ(3u, f.GetInstancesCount());
    ASSERT_EQ("sop3", f.GetInstance(0).GetSopInstanceUid());
    ASSERT_EQ("sop1", f.GetInstance(1).GetSopInstanceUid());
    ASSERT_EQ("sop2", f.GetInstance(2).GetSopInstanceUid());
    ASSERT_EQ(6u, f.GetFramesCount());
    ASSERT_EQ("sop1", f.GetInstanceOfFrame(0).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(0));
    ASSERT_EQ("sop1", f.GetInstanceOfFrame(1).GetSopInstanceUid());  ASSERT_EQ(1u, f.GetFrameNumberInInstance(1));
    ASSERT_EQ("sop1", f.GetInstanceOfFrame(2).GetSopInstanceUid());  ASSERT_EQ(2u, f.GetFrameNumberInInstance(2));
    ASSERT_EQ("sop2", f.GetInstanceOfFrame(3).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(3));
    ASSERT_EQ("sop2", f.GetInstanceOfFrame(4).GetSopInstanceUid());  ASSERT_EQ(1u, f.GetFrameNumberInInstance(4));
    ASSERT_EQ("sop3", f.GetInstanceOfFrame(5).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(5));

    // The instances must not have been reordered, only the frames
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop1"));  ASSERT_EQ(1u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop2"));  ASSERT_EQ(2u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop3"));  ASSERT_EQ(0u, i);
    ASSERT_FALSE(f.LookupSopInstanceUid(i, "nope"));

    ASSERT_TRUE(f.LookupFrame(i, "sop1", 0));  ASSERT_EQ(0u, i);
    ASSERT_TRUE(f.LookupFrame(i, "sop1", 1));  ASSERT_EQ(1u, i);
    ASSERT_TRUE(f.LookupFrame(i, "sop1", 2));  ASSERT_EQ(2u, i);
    ASSERT_TRUE(f.LookupFrame(i, "sop2", 0));  ASSERT_EQ(3u, i);
    ASSERT_TRUE(f.LookupFrame(i, "sop2", 1));  ASSERT_EQ(4u, i);
    ASSERT_TRUE(f.LookupFrame(i, "sop3", 0));  ASSERT_EQ(5u, i);

    ASSERT_FALSE(f.LookupFrame(i, "nope", 0));
    ASSERT_FALSE(f.LookupFrame(i, "sop1", 3));
    ASSERT_FALSE(f.LookupFrame(i, "sop2", 2));
    ASSERT_FALSE(f.LookupFrame(i, "sop3", 1));
  }


  TEST(SortedFrames, SortInstanceNumber)
  {
    Orthanc::DicomMap tags;
    tags.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, "study", false);
    tags.SetValue(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, "series", false);
    
    SortedFrames f;
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop1", false);
    tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "20", false);
    f.AddInstance(tags);
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop2", false);
    tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "-20", false);
    f.AddInstance(tags);
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop2a", false);
    tags.Remove(Orthanc::DICOM_TAG_INSTANCE_NUMBER);
    f.AddInstance(tags);
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop4", false);
    tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "10", false);
    f.AddInstance(tags);
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop3", false);
    tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "10", false);
    f.AddInstance(tags);
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop5", false);
    tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "10", false);
    f.AddInstance(tags);
    
    size_t i;
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop1"));  ASSERT_EQ(0u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop2"));  ASSERT_EQ(1u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop2a")); ASSERT_EQ(2u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop4"));  ASSERT_EQ(3u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop3"));  ASSERT_EQ(4u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop5"));  ASSERT_EQ(5u, i);
    ASSERT_FALSE(f.LookupSopInstanceUid(i, "nope"));

    ASSERT_THROW(f.LookupFrame(i, "sop1", 0), Orthanc::OrthancException);  // Not sorted yet

    f.Sort();
    ASSERT_EQ(6u, f.GetInstancesCount());
    ASSERT_EQ("sop1", f.GetInstance(0).GetSopInstanceUid());
    ASSERT_EQ("sop2", f.GetInstance(1).GetSopInstanceUid());
    ASSERT_EQ("sop2a", f.GetInstance(2).GetSopInstanceUid());
    ASSERT_EQ("sop4", f.GetInstance(3).GetSopInstanceUid());
    ASSERT_EQ("sop3", f.GetInstance(4).GetSopInstanceUid());
    ASSERT_EQ("sop5", f.GetInstance(5).GetSopInstanceUid());
    ASSERT_EQ(6u, f.GetFramesCount());
    ASSERT_EQ("sop2", f.GetInstanceOfFrame(0).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(0));
    ASSERT_EQ("sop3", f.GetInstanceOfFrame(1).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(1));
    ASSERT_EQ("sop4", f.GetInstanceOfFrame(2).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(2));
    ASSERT_EQ("sop5", f.GetInstanceOfFrame(3).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(3));
    ASSERT_EQ("sop1", f.GetInstanceOfFrame(4).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(4));
    ASSERT_EQ("sop2a", f.GetInstanceOfFrame(5).GetSopInstanceUid()); ASSERT_EQ(0u, f.GetFrameNumberInInstance(5));

    // The instances must not have been reordered, only the frames
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop1"));  ASSERT_EQ(0u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop2"));  ASSERT_EQ(1u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop2a")); ASSERT_EQ(2u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop4"));  ASSERT_EQ(3u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop3"));  ASSERT_EQ(4u, i);
    ASSERT_TRUE(f.LookupSopInstanceUid(i, "sop5"));  ASSERT_EQ(5u, i);
    ASSERT_FALSE(f.LookupSopInstanceUid(i, "nope"));

    ASSERT_TRUE(f.LookupFrame(i, "sop2", 0));  ASSERT_EQ(0u, i);
    ASSERT_TRUE(f.LookupFrame(i, "sop3", 0));  ASSERT_EQ(1u, i);
    ASSERT_TRUE(f.LookupFrame(i, "sop4", 0));  ASSERT_EQ(2u, i);
    ASSERT_TRUE(f.LookupFrame(i, "sop5", 0));  ASSERT_EQ(3u, i);
    ASSERT_TRUE(f.LookupFrame(i, "sop1", 0));  ASSERT_EQ(4u, i);
    ASSERT_TRUE(f.LookupFrame(i, "sop2a", 0)); ASSERT_EQ(5u, i);

    ASSERT_FALSE(f.LookupFrame(i, "nope", 0));
    ASSERT_FALSE(f.LookupFrame(i, "sop1", 1));
  }


  TEST(SortedFrames, SortInstanceNumberAndImageIndex)
  {
    Orthanc::DicomMap tags;
    tags.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, "study", false);
    tags.SetValue(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, "series", false);
    
    SortedFrames f;
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop1", false);
    tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "20", false);
    f.AddInstance(tags);
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop2", false);
    tags.Remove(Orthanc::DICOM_TAG_INSTANCE_NUMBER);
    tags.SetValue(Orthanc::DICOM_TAG_IMAGE_INDEX, "20", false);
    f.AddInstance(tags);
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop3", false);
    tags.SetValue(Orthanc::DICOM_TAG_IMAGE_INDEX, "30", false);
    f.AddInstance(tags);
    tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop4", false);
    tags.Remove(Orthanc::DICOM_TAG_IMAGE_INDEX);
    tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "30", false);
    f.AddInstance(tags);

    f.Sort();
    ASSERT_EQ(4u, f.GetInstancesCount());
    ASSERT_EQ("sop1", f.GetInstance(0).GetSopInstanceUid());
    ASSERT_EQ("sop2", f.GetInstance(1).GetSopInstanceUid());
    ASSERT_EQ("sop3", f.GetInstance(2).GetSopInstanceUid());
    ASSERT_EQ("sop4", f.GetInstance(3).GetSopInstanceUid());
    ASSERT_EQ(4u, f.GetFramesCount());
    // First instance number, then image index
    ASSERT_EQ("sop1", f.GetInstanceOfFrame(0).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(0));
    ASSERT_EQ("sop4", f.GetInstanceOfFrame(1).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(1));
    ASSERT_EQ("sop2", f.GetInstanceOfFrame(2).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(2));
    ASSERT_EQ("sop3", f.GetInstanceOfFrame(3).GetSopInstanceUid());  ASSERT_EQ(0u, f.GetFrameNumberInInstance(3));
  }
}


TEST(SortedFrames, FrameOffset)
{
  Orthanc::DicomMap tags;
  tags.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, "study", false);
  tags.SetValue(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, "series", false);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "1.2.276.0.7230010.3.1.4.296485376.1.1568899779.944133", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-350\\-145\\-985", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "1\\0\\0\\0\\1\\0", false);
  tags.SetValue(Orthanc::DICOM_TAG_NUMBER_OF_FRAMES, "2", false);

  for (unsigned int i = 0; i < 3; i++)
  {
    tags.SetValue(Orthanc::DICOM_TAG_FRAME_INCREMENT_POINTER, i != 1 ? "3004,000c" : "nope", false);
    tags.SetValue(Orthanc::DICOM_TAG_GRID_FRAME_OFFSET_VECTOR, i != 2 ? "8\\11" : "1\\2\\3", false);

    OrthancStone::SortedFrames f;
    f.AddInstance(tags);
    f.Sort();

    ASSERT_EQ(2u, f.GetFramesCount());
    ASSERT_TRUE(f.GetFrameGeometry(0).IsValid());
    ASSERT_TRUE(f.GetFrameGeometry(1).IsValid());
    ASSERT_FLOAT_EQ(-350.0f, static_cast<float>(f.GetFrameGeometry(0).GetOrigin() [0]));
    ASSERT_FLOAT_EQ(-145.0f, static_cast<float>(f.GetFrameGeometry(0).GetOrigin() [1]));
    ASSERT_FLOAT_EQ(-350.0f, static_cast<float>(f.GetFrameGeometry(1).GetOrigin() [0]));
    ASSERT_FLOAT_EQ(-145.0f, static_cast<float>(f.GetFrameGeometry(1).GetOrigin() [1]));

    if (i == 0)
    {
      ASSERT_FLOAT_EQ(-985.0f + 8.0f, static_cast<float>(f.GetFrameGeometry(0).GetOrigin() [2]));
      ASSERT_FLOAT_EQ(-985.0f + 11.0f, static_cast<float>(f.GetFrameGeometry(1).GetOrigin() [2]));
    }
    else
    {
      ASSERT_FLOAT_EQ(-985.0f, static_cast<float>(f.GetFrameGeometry(0).GetOrigin() [2]));
      ASSERT_FLOAT_EQ(-985.0f, static_cast<float>(f.GetFrameGeometry(1).GetOrigin() [2]));
    }
  }
}


TEST(SortedFrames, Knix)  // Created using "SortedFramesCreateTest.py"
{
  Orthanc::DicomMap tags;
  tags.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, "1.2.840.113619.2.176.2025.1499492.7391.1171285944.390", false);
  tags.SetValue(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, "1.2.840.113619.2.176.2025.1499492.7391.1171285944.392", false);    
  OrthancStone::SortedFrames f;

  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "67b44a5e-8997f88d-6e527bd6-df342483-dab1674c", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-60.7285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "10", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "a8ee83f9-1cc26ad9-ebba3043-8afc47c2-bd784610", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-42.7285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "6", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "5a2acb03-063f5063-cac452d1-a55992f9-769900fb", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-114.729\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "22", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "23d12f39-e9a4fc21-8da338c4-97feff30-48e95534", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-83.2285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "15", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "16606f69-83b48518-ab34304a-c8871b7f-a9298d74", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-78.7285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "14", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "63d595f3-327a306d-1709bb8b-2a72e11c-4f7221fe", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-96.7285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "18", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "8bdecadd-e3477e28-bbbf0297-22b0b680-37b13a7c", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-65.2285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "11", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "b590cc95-55789755-ebd10b76-911e855e-f24e4fe7", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-74.2285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "13", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "eaa49a94-b9042041-7f45150b-e414f800-d7232874", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-38.2285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "5", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "6824db93-ed4e2740-07be953f-6d0a8fb3-af0a3a0b", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-105.729\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "20", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "e0d82343-9cef01e9-e21df50a-11886a94-1d0216ea", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-51.7285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "8", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "dc1576ee-25b0b1ef-e038df76-d296fcad-a1456169", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-110.229\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "21", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "b9cf5158-06f8e713-7d5111aa-411fd75b-7be2c51e", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-20.2285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "1", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "5faf886f-bd5517cf-1a6ba06e-ac0e6ddb-47bdd8b2", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-101.229\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "19", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "3e8f8ec1-b603f874-825552f1-6fcac7fa-72ca1aa5", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-24.7285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "2", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "7a7c0120-37f6dd58-c46312e6-2559975d-5af4616f", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-87.7285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "16", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "a0ca6802-56c697c3-0205bab8-42217cfc-84ff0de6", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-33.7285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "4", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "efce9ff4-3fe07d83-745846f8-fefe5d64-bfea65e6", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-56.2285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "9", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "fa56f961-d1ae8f6a-989c04f4-7a588e9e-b41b1a13", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-92.2285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "17", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "f5e889ac-c5afdc37-c5b62074-a8bdeef3-c58d9889", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-69.7285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "12", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "c19fb4b6-ad1224f2-2c3a2b28-0ea233be-38eea0de", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-47.2285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "7", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "348efc0a-71ee4758-56bd51fa-9703cbff-9b51d4c9", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-29.2285\\-105.586\\73.7768", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "-0\\1\\0\\-0\\-0\\-1", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "3", false);
  f.AddInstance(tags);
  f.Sort();
  ASSERT_EQ(22u, f.GetFramesCount());
  ASSERT_EQ(f.GetInstanceOfFrame(0).GetSopInstanceUid(), "b9cf5158-06f8e713-7d5111aa-411fd75b-7be2c51e");
  ASSERT_EQ(f.GetInstanceOfFrame(1).GetSopInstanceUid(), "3e8f8ec1-b603f874-825552f1-6fcac7fa-72ca1aa5");
  ASSERT_EQ(f.GetInstanceOfFrame(2).GetSopInstanceUid(), "348efc0a-71ee4758-56bd51fa-9703cbff-9b51d4c9");
  ASSERT_EQ(f.GetInstanceOfFrame(3).GetSopInstanceUid(), "a0ca6802-56c697c3-0205bab8-42217cfc-84ff0de6");
  ASSERT_EQ(f.GetInstanceOfFrame(4).GetSopInstanceUid(), "eaa49a94-b9042041-7f45150b-e414f800-d7232874");
  ASSERT_EQ(f.GetInstanceOfFrame(5).GetSopInstanceUid(), "a8ee83f9-1cc26ad9-ebba3043-8afc47c2-bd784610");
  ASSERT_EQ(f.GetInstanceOfFrame(6).GetSopInstanceUid(), "c19fb4b6-ad1224f2-2c3a2b28-0ea233be-38eea0de");
  ASSERT_EQ(f.GetInstanceOfFrame(7).GetSopInstanceUid(), "e0d82343-9cef01e9-e21df50a-11886a94-1d0216ea");
  ASSERT_EQ(f.GetInstanceOfFrame(8).GetSopInstanceUid(), "efce9ff4-3fe07d83-745846f8-fefe5d64-bfea65e6");
  ASSERT_EQ(f.GetInstanceOfFrame(9).GetSopInstanceUid(), "67b44a5e-8997f88d-6e527bd6-df342483-dab1674c");
  ASSERT_EQ(f.GetInstanceOfFrame(10).GetSopInstanceUid(), "8bdecadd-e3477e28-bbbf0297-22b0b680-37b13a7c");
  ASSERT_EQ(f.GetInstanceOfFrame(11).GetSopInstanceUid(), "f5e889ac-c5afdc37-c5b62074-a8bdeef3-c58d9889");
  ASSERT_EQ(f.GetInstanceOfFrame(12).GetSopInstanceUid(), "b590cc95-55789755-ebd10b76-911e855e-f24e4fe7");
  ASSERT_EQ(f.GetInstanceOfFrame(13).GetSopInstanceUid(), "16606f69-83b48518-ab34304a-c8871b7f-a9298d74");
  ASSERT_EQ(f.GetInstanceOfFrame(14).GetSopInstanceUid(), "23d12f39-e9a4fc21-8da338c4-97feff30-48e95534");
  ASSERT_EQ(f.GetInstanceOfFrame(15).GetSopInstanceUid(), "7a7c0120-37f6dd58-c46312e6-2559975d-5af4616f");
  ASSERT_EQ(f.GetInstanceOfFrame(16).GetSopInstanceUid(), "fa56f961-d1ae8f6a-989c04f4-7a588e9e-b41b1a13");
  ASSERT_EQ(f.GetInstanceOfFrame(17).GetSopInstanceUid(), "63d595f3-327a306d-1709bb8b-2a72e11c-4f7221fe");
  ASSERT_EQ(f.GetInstanceOfFrame(18).GetSopInstanceUid(), "5faf886f-bd5517cf-1a6ba06e-ac0e6ddb-47bdd8b2");
  ASSERT_EQ(f.GetInstanceOfFrame(19).GetSopInstanceUid(), "6824db93-ed4e2740-07be953f-6d0a8fb3-af0a3a0b");
  ASSERT_EQ(f.GetInstanceOfFrame(20).GetSopInstanceUid(), "dc1576ee-25b0b1ef-e038df76-d296fcad-a1456169");
  ASSERT_EQ(f.GetInstanceOfFrame(21).GetSopInstanceUid(), "5a2acb03-063f5063-cac452d1-a55992f9-769900fb");
}


TEST(SortedFrames, Cardiac)  // Created using "SortedFramesCreateTest.py"
{
  Orthanc::DicomMap tags;
  tags.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, "1.3.51.0.1.1.192.168.29.133.1681753.1681732", false);
  tags.SetValue(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, "1.3.12.2.1107.5.2.33.37097.2012041612474981424569674.0.0.0", false);    
  OrthancStone::SortedFrames f;

  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "a468da62-a8a6e0b9-f66b86b0-b15fa30b-93077161", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "14", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "1cf40ac9-e823e677-cbd5db4b-9e48b451-cccbf950", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "21", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "d52d5f21-54f1ad99-4015a995-108f7210-ee157944", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "15", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "b348f629-11d59f98-fb22710b-4964b90a-f44436ff", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "12", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "aac4f2ba-e863f124-6af96709-053258a7-3d39db26", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "13", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "8fefe14c-c4c34152-2c3d3514-04e75747-eb7f01f0", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "20", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "20b42f52-6d5f784b-cdbc0fbe-4bfc6b0c-5a199c75", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "17", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "931d0c36-8fbb4101-70e6d756-edb15431-aaa9a31b", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "19", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "9e3b97ec-25b86a67-2cbb8f77-94e73268-4509d383", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "10", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "caa62568-fdf894fe-08f830a2-5a468967-681d954b", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "18", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "e734c170-96b0a397-95e3b43e-d7a5ed74-025843c8", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "22", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "efc9f411-9f4294e0-66d292a1-b8b6b421-897f1d80", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "11", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "8346a1db-0b08a22b-9045aaad-57098aac-5b2e9159", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "16", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "8c7d1e4d-7936f799-c4b8b56b-32d0d9a6-2b492e98", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "3", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "faec09f9-ca7fe0f0-2b25c370-bb1bfaef-8ccfa560", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "4", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "99c20bcc-115ae447-84d616f2-cb6c5576-9f67aa7a", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "23", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "7906b806-47190031-72c5043c-d42704c1-688a3b23", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "9", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "c9dfc022-7b377063-08bdc5e8-fedcc463-8de22ee6", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "6", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "6570b6c0-7d2f324d-db7cad50-843f62df-d0446352", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "5", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "0be36fe7-6c7a762b-281cf109-fff9d8ea-42e16b7a", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "7", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "ec282396-a8209d00-1c5091f3-f632bf3d-a1bcebba", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "8", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "fda415d4-f1429b07-5d1cd9f0-675059ff-c0ce9e67", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "1", false);
  f.AddInstance(tags);
  tags.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "f555ef96-6b01a90c-bdc2585a-dd17bb3a-75e89920", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, "-37.318577811371\\-157.20910163001\\232.94204104611", false);
  tags.SetValue(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, "0.73931693068262\\0.61320183243991\\-0.2781977510663\\-0.3521819177853\\-3.9073598e-009\\-0.9359315662938", false);
  tags.SetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER, "2", false);
  f.AddInstance(tags);
  f.Sort();
  ASSERT_EQ(23u, f.GetFramesCount());
  ASSERT_EQ(f.GetInstanceOfFrame(0).GetSopInstanceUid(), "fda415d4-f1429b07-5d1cd9f0-675059ff-c0ce9e67");
  ASSERT_EQ(f.GetInstanceOfFrame(1).GetSopInstanceUid(), "f555ef96-6b01a90c-bdc2585a-dd17bb3a-75e89920");
  ASSERT_EQ(f.GetInstanceOfFrame(2).GetSopInstanceUid(), "8c7d1e4d-7936f799-c4b8b56b-32d0d9a6-2b492e98");
  ASSERT_EQ(f.GetInstanceOfFrame(3).GetSopInstanceUid(), "faec09f9-ca7fe0f0-2b25c370-bb1bfaef-8ccfa560");
  ASSERT_EQ(f.GetInstanceOfFrame(4).GetSopInstanceUid(), "6570b6c0-7d2f324d-db7cad50-843f62df-d0446352");
  ASSERT_EQ(f.GetInstanceOfFrame(5).GetSopInstanceUid(), "c9dfc022-7b377063-08bdc5e8-fedcc463-8de22ee6");
  ASSERT_EQ(f.GetInstanceOfFrame(6).GetSopInstanceUid(), "0be36fe7-6c7a762b-281cf109-fff9d8ea-42e16b7a");
  ASSERT_EQ(f.GetInstanceOfFrame(7).GetSopInstanceUid(), "ec282396-a8209d00-1c5091f3-f632bf3d-a1bcebba");
  ASSERT_EQ(f.GetInstanceOfFrame(8).GetSopInstanceUid(), "7906b806-47190031-72c5043c-d42704c1-688a3b23");
  ASSERT_EQ(f.GetInstanceOfFrame(9).GetSopInstanceUid(), "9e3b97ec-25b86a67-2cbb8f77-94e73268-4509d383");
  ASSERT_EQ(f.GetInstanceOfFrame(10).GetSopInstanceUid(), "efc9f411-9f4294e0-66d292a1-b8b6b421-897f1d80");
  ASSERT_EQ(f.GetInstanceOfFrame(11).GetSopInstanceUid(), "b348f629-11d59f98-fb22710b-4964b90a-f44436ff");
  ASSERT_EQ(f.GetInstanceOfFrame(12).GetSopInstanceUid(), "aac4f2ba-e863f124-6af96709-053258a7-3d39db26");
  ASSERT_EQ(f.GetInstanceOfFrame(13).GetSopInstanceUid(), "a468da62-a8a6e0b9-f66b86b0-b15fa30b-93077161");
  ASSERT_EQ(f.GetInstanceOfFrame(14).GetSopInstanceUid(), "d52d5f21-54f1ad99-4015a995-108f7210-ee157944");
  ASSERT_EQ(f.GetInstanceOfFrame(15).GetSopInstanceUid(), "8346a1db-0b08a22b-9045aaad-57098aac-5b2e9159");
  ASSERT_EQ(f.GetInstanceOfFrame(16).GetSopInstanceUid(), "20b42f52-6d5f784b-cdbc0fbe-4bfc6b0c-5a199c75");
  ASSERT_EQ(f.GetInstanceOfFrame(17).GetSopInstanceUid(), "caa62568-fdf894fe-08f830a2-5a468967-681d954b");
  ASSERT_EQ(f.GetInstanceOfFrame(18).GetSopInstanceUid(), "931d0c36-8fbb4101-70e6d756-edb15431-aaa9a31b");
  ASSERT_EQ(f.GetInstanceOfFrame(19).GetSopInstanceUid(), "8fefe14c-c4c34152-2c3d3514-04e75747-eb7f01f0");
  ASSERT_EQ(f.GetInstanceOfFrame(20).GetSopInstanceUid(), "1cf40ac9-e823e677-cbd5db4b-9e48b451-cccbf950");
  ASSERT_EQ(f.GetInstanceOfFrame(21).GetSopInstanceUid(), "e734c170-96b0a397-95e3b43e-d7a5ed74-025843c8");
  ASSERT_EQ(f.GetInstanceOfFrame(22).GetSopInstanceUid(), "99c20bcc-115ae447-84d616f2-cb6c5576-9f67aa7a");
}

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

#include "../OrthancStone/Sources/Toolbox/DicomInstanceParameters.h"
#include "../OrthancStone/Sources/Loaders/DicomSource.h"

#include <OrthancException.h>


static void SetupUids(Orthanc::DicomMap& m)
{
  m.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, "my_study", false);
  m.SetValue(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, "my_series", false);
  m.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "my_sop", false);
}


TEST(DicomInstanceParameters, Basic)
{
  Orthanc::DicomMap m;
  SetupUids(m);

  std::unique_ptr<OrthancStone::DicomInstanceParameters> p;
  p.reset(OrthancStone::DicomInstanceParameters(m).Clone());

  ASSERT_TRUE(p->GetOrthancInstanceIdentifier().empty());
  ASSERT_EQ(3u, p->GetTags().GetSize());
  ASSERT_EQ("my_study", p->GetStudyInstanceUid());
  ASSERT_EQ("my_series", p->GetSeriesInstanceUid());
  ASSERT_EQ("my_sop", p->GetSopInstanceUid());
  ASSERT_EQ(OrthancStone::SopClassUid_Other, p->GetSopClassUid());
  ASSERT_EQ(1u, p->GetNumberOfFrames());
  ASSERT_EQ(0u, p->GetWidth());
  ASSERT_EQ(0u, p->GetHeight());
  ASSERT_TRUE(OrthancStone::LinearAlgebra::IsCloseToZero(p->GetSliceThickness()));
  ASSERT_FLOAT_EQ(1, p->GetPixelSpacingX());
  ASSERT_FLOAT_EQ(1, p->GetPixelSpacingY());
  ASSERT_FALSE(p->GetGeometry().IsValid());
  ASSERT_THROW(p->GetImageInformation(), Orthanc::OrthancException);
  ASSERT_FALSE(p->GetFrameGeometry(0).IsValid());
  ASSERT_THROW(p->IsColor(), Orthanc::OrthancException);  // Accesses DicomImageInformation
  ASSERT_FALSE(p->HasRescale());
  ASSERT_THROW(p->GetRescaleIntercept(), Orthanc::OrthancException);
  ASSERT_THROW(p->GetRescaleSlope(), Orthanc::OrthancException);
  ASSERT_EQ(0u, p->GetWindowingPresetsCount());
  ASSERT_THROW(p->GetWindowingPresetCenter(0), Orthanc::OrthancException);
  ASSERT_THROW(p->GetWindowingPresetWidth(0), Orthanc::OrthancException);

  float c, w;
  p->GetWindowingPresetsUnion(c, w);
  ASSERT_FLOAT_EQ(128.0f, c);
  ASSERT_FLOAT_EQ(256.0f, w);

  ASSERT_THROW(p->GetExpectedPixelFormat(), Orthanc::OrthancException);
  ASSERT_FALSE(p->HasIndexInSeries());
  ASSERT_THROW(p->GetIndexInSeries(), Orthanc::OrthancException);
  ASSERT_TRUE(p->GetDoseUnits().empty());
  ASSERT_DOUBLE_EQ(1.0, p->GetDoseGridScaling());
  ASSERT_DOUBLE_EQ(1.0, p->ApplyRescale(1.0));

  double s;
  ASSERT_FALSE(p->ComputeRegularSpacing(s));
  ASSERT_TRUE(p->GetFrameOfReferenceUid().empty());
}


TEST(DicomInstanceParameters, Windowing)
{
  Orthanc::DicomMap m;
  SetupUids(m);
  m.SetValue(Orthanc::DICOM_TAG_WINDOW_CENTER, "10\\100\\1000", false);
  m.SetValue(Orthanc::DICOM_TAG_WINDOW_WIDTH, "50\\60\\70", false);

  OrthancStone::DicomInstanceParameters p(m);
  ASSERT_EQ(3u, p.GetWindowingPresetsCount());
  ASSERT_FLOAT_EQ(10, p.GetWindowingPresetCenter(0));
  ASSERT_FLOAT_EQ(100, p.GetWindowingPresetCenter(1));
  ASSERT_FLOAT_EQ(1000, p.GetWindowingPresetCenter(2));
  ASSERT_FLOAT_EQ(50, p.GetWindowingPresetWidth(0));
  ASSERT_FLOAT_EQ(60, p.GetWindowingPresetWidth(1));
  ASSERT_FLOAT_EQ(70, p.GetWindowingPresetWidth(2));

  const float a = 10.0f - 50.0f / 2.0f;
  const float b = 1000.0f + 70.0f / 2.0f;
  
  float c, w;
  p.GetWindowingPresetsUnion(c, w);
  ASSERT_FLOAT_EQ((a + b) / 2.0f, c);
  ASSERT_FLOAT_EQ(b - a, w);
}


TEST(DicomSource, Equality)
{
  {
    OrthancStone::DicomSource s1;

    {
      OrthancStone::DicomSource s2;
      ASSERT_TRUE(s1.IsSameSource(s2));

      s2.SetDicomDirSource();
      ASSERT_FALSE(s1.IsSameSource(s2));

      s2.SetDicomWebSource("toto");
      ASSERT_FALSE(s1.IsSameSource(s2));

      s2.SetDicomWebThroughOrthancSource("toto");
      ASSERT_FALSE(s1.IsSameSource(s2));

      s2.SetOrthancSource();
      ASSERT_TRUE(s1.IsSameSource(s2));
    }
  }

  {
    OrthancStone::DicomSource s1;

    {
      Orthanc::WebServiceParameters p;
      p.SetUrl("http://localhost:8042/");

      OrthancStone::DicomSource s2;
      s2.SetOrthancSource(p);
      ASSERT_TRUE(s1.IsSameSource(s2));

      p.SetCredentials("toto", "tutu");
      s2.SetOrthancSource(p);
      ASSERT_FALSE(s1.IsSameSource(s2));
      
      p.ClearCredentials();
      s2.SetOrthancSource(p);
      ASSERT_TRUE(s1.IsSameSource(s2));

      p.SetUrl("http://localhost:8043/");
      s2.SetOrthancSource(p);
      ASSERT_FALSE(s1.IsSameSource(s2));
    }
  }

  {
    OrthancStone::DicomSource s1;
    s1.SetDicomDirSource();

    {
      OrthancStone::DicomSource s2;
      ASSERT_FALSE(s1.IsSameSource(s2));

      s2.SetDicomDirSource();
      ASSERT_TRUE(s1.IsSameSource(s2));

      s2.SetDicomWebSource("toto");
      ASSERT_FALSE(s1.IsSameSource(s2));

      s2.SetDicomWebThroughOrthancSource("toto");
      ASSERT_FALSE(s1.IsSameSource(s2));
    }
  }

  {
    OrthancStone::DicomSource s1;
    s1.SetDicomWebSource("http");

    {
      OrthancStone::DicomSource s2;
      ASSERT_FALSE(s1.IsSameSource(s2));

      s2.SetDicomDirSource();
      ASSERT_FALSE(s1.IsSameSource(s2));

      s2.SetDicomWebSource("http");
      ASSERT_TRUE(s1.IsSameSource(s2));

      s2.SetDicomWebSource("http2");
      ASSERT_FALSE(s1.IsSameSource(s2));

      s2.SetDicomWebThroughOrthancSource("toto");
      ASSERT_FALSE(s1.IsSameSource(s2));
    }
  }

  {
    OrthancStone::DicomSource s1;
    s1.SetDicomWebThroughOrthancSource("server");

    {
      OrthancStone::DicomSource s2;
      ASSERT_FALSE(s1.IsSameSource(s2));

      s2.SetDicomDirSource();
      ASSERT_FALSE(s1.IsSameSource(s2));

      s2.SetDicomWebSource("http");
      ASSERT_FALSE(s1.IsSameSource(s2));

      s2.SetDicomWebThroughOrthancSource("server");
      ASSERT_TRUE(s1.IsSameSource(s2));

      s2.SetDicomWebThroughOrthancSource("server2");
      ASSERT_FALSE(s1.IsSameSource(s2));
    }
  }
}

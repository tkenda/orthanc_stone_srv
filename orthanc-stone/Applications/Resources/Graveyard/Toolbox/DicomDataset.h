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


#pragma once

#include "../../Resources/Orthanc/Plugins/Samples/Common/IOrthancConnection.h"

#include <map>
#include <stdint.h>
#include <json/value.h>

namespace OrthancStone
{
  // This class is NOT thread-safe
  // This is a lightweight alternative to Orthanc::DicomMap
  class DicomDataset : public boost::noncopyable
  {
  public:
    typedef std::pair<uint16_t, uint16_t>  Tag;

  private:
    typedef std::map<Tag, std::string>  Values;

    Values  values_;

    void Parse(const std::string& content);

    void Parse(const Json::Value& content);

  public:
    DicomDataset(const std::string& content)
    {
      Parse(content);
    }

    DicomDataset(const Json::Value& content)
    {
      Parse(content);
    }

    DicomDataset(OrthancPlugins::IOrthancConnection& orthanc,
                 const std::string& instanceId);

    bool HasTag(const Tag& tag) const
    {
      return values_.find(tag) != values_.end();
    }

    std::string GetStringValue(const Tag& tag) const;

    std::string GetStringValue(const Tag& tag,
                               const std::string& defaultValue) const;

    float GetFloatValue(const Tag& tag) const;

    double GetDoubleValue(const Tag& tag) const;

    int GetIntegerValue(const Tag& tag) const;

    unsigned int GetUnsignedIntegerValue(const Tag& tag) const;

    void GetVectorValue(Vector& vector, 
                        const Tag& tag,
                        size_t expectedSize) const;

    void GetVectorValue(Vector& vector, 
                        const Tag& tag) const;

    void Print() const;

    bool IsGrayscale() const;

    void GetPixelSpacing(double& spacingX,
                         double& spacingY) const;
  };


  static const DicomDataset::Tag DICOM_TAG_COLUMNS(0x0028, 0x0011);
  static const DicomDataset::Tag DICOM_TAG_IMAGE_ORIENTATION_PATIENT(0x0020, 0x0037);
  static const DicomDataset::Tag DICOM_TAG_IMAGE_POSITION_PATIENT(0x0020, 0x0032);
  static const DicomDataset::Tag DICOM_TAG_NUMBER_OF_FRAMES(0x0028, 0x0008);
  static const DicomDataset::Tag DICOM_TAG_PIXEL_REPRESENTATION(0x0028, 0x0103);
  static const DicomDataset::Tag DICOM_TAG_PIXEL_SPACING(0x0028, 0x0030);
  static const DicomDataset::Tag DICOM_TAG_RESCALE_INTERCEPT(0x0028, 0x1052);
  static const DicomDataset::Tag DICOM_TAG_RESCALE_SLOPE(0x0028, 0x1053);
  static const DicomDataset::Tag DICOM_TAG_ROWS(0x0028, 0x0010);
  static const DicomDataset::Tag DICOM_TAG_SLICE_THICKNESS(0x0018, 0x0050);
  static const DicomDataset::Tag DICOM_TAG_WINDOW_CENTER(0x0028, 0x1050);
  static const DicomDataset::Tag DICOM_TAG_WINDOW_WIDTH(0x0028, 0x1051);
  static const DicomDataset::Tag DICOM_TAG_PHOTOMETRIC_INTERPRETATION(0x0028, 0x0004);
}

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


#include "DicomDataset.h"

#include "../../Resources/Orthanc/Core/OrthancException.h"
#include "../../Resources/Orthanc/Core/Logging.h"
#include "../../Resources/Orthanc/Core/Toolbox.h"

#include <boost/lexical_cast.hpp>
#include <json/value.h>
#include <json/reader.h>

namespace OrthancStone
{
  static uint16_t GetCharValue(char c)
  {
    if (c >= '0' && c <= '9')
      return c - '0';
    else if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    else
      return 0;
  }


  static uint16_t GetHexadecimalValue(const char* c)
  {
    return ((GetCharValue(c[0]) << 12) + 
            (GetCharValue(c[1]) << 8) + 
            (GetCharValue(c[2]) << 4) + 
            GetCharValue(c[3]));
  }


  static DicomDataset::Tag ParseTag(const std::string& tag)
  {
    if (tag.size() == 9 &&
        isxdigit(tag[0]) &&
        isxdigit(tag[1]) &&
        isxdigit(tag[2]) &&
        isxdigit(tag[3]) &&
        (tag[4] == '-' || tag[4] == ',') &&
        isxdigit(tag[5]) &&
        isxdigit(tag[6]) &&
        isxdigit(tag[7]) &&
        isxdigit(tag[8]))        
    {
      uint16_t group = GetHexadecimalValue(tag.c_str());
      uint16_t element = GetHexadecimalValue(tag.c_str() + 5);
      return std::make_pair(group, element);
    }
    else if (tag.size() == 8 &&
             isxdigit(tag[0]) &&
             isxdigit(tag[1]) &&
             isxdigit(tag[2]) &&
             isxdigit(tag[3]) &&
             isxdigit(tag[4]) &&
             isxdigit(tag[5]) &&
             isxdigit(tag[6]) &&
             isxdigit(tag[7]))        
    {
      uint16_t group = GetHexadecimalValue(tag.c_str());
      uint16_t element = GetHexadecimalValue(tag.c_str() + 4);
      return std::make_pair(group, element);
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);        
    }
  }

  void DicomDataset::Parse(const std::string& content)
  {
    Json::Value json;
    Json::Reader reader;
    if (!reader.parse(content, json))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);        
    }

    Parse(json);
  }


  void DicomDataset::Parse(const Json::Value& content)
  {
    if (content.type() != Json::objectValue)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);        
    }

    Json::Value::Members members = content.getMemberNames();
    for (size_t i = 0; i < members.size(); i++)
    {
      Tag tag = ParseTag(members[i]);

      const Json::Value& item = content[members[i]];

      if (item.type() != Json::objectValue ||
          !item.isMember("Type") ||
          !item.isMember("Value") ||
          !item.isMember("Name") ||
          item["Type"].type() != Json::stringValue ||
          item["Name"].type() != Json::stringValue)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);        
      }

      if (item["Type"].asString() == "String")
      {
        if (item["Value"].type() == Json::stringValue)
        {
          values_[tag] = item["Value"].asString();
        }
        else
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);        
        }
      }
    }
  }


  DicomDataset::DicomDataset(OrthancPlugins::IOrthancConnection& orthanc,
                             const std::string& instanceId)
  {
    std::string content;
    orthanc.RestApiGet(content, "/instances/" + instanceId + "/tags");

    Parse(content);
  }


  std::string DicomDataset::GetStringValue(const Tag& tag) const
  {
    Values::const_iterator it = values_.find(tag);

    if (it == values_.end())
    {
      LOG(ERROR) << "Trying to access a DICOM tag that is not set in a DICOM dataset";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InexistentItem);
    }
    else
    {
      return it->second;
    }
  }


  std::string DicomDataset::GetStringValue(const Tag& tag,
                                           const std::string& defaultValue) const
  {
    Values::const_iterator it = values_.find(tag);

    if (it == values_.end())
    {
      return defaultValue;
    }
    else
    {
      return it->second;
    }
  }


  float DicomDataset::GetFloatValue(const Tag& tag) const
  {
    try 
    {
      return boost::lexical_cast<float>(Orthanc::Toolbox::StripSpaces(GetStringValue(tag)));
    }
    catch (boost::bad_lexical_cast&)
    {
      LOG(ERROR) << "Trying to access a DICOM tag that is not a float";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
  }


  double DicomDataset::GetDoubleValue(const Tag& tag) const
  {
    try 
    {
      return boost::lexical_cast<double>(Orthanc::Toolbox::StripSpaces(GetStringValue(tag)));
    }
    catch (boost::bad_lexical_cast&)
    {
      LOG(ERROR) << "Trying to access a DICOM tag that is not a float";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
  }


  int DicomDataset::GetIntegerValue(const Tag& tag) const
  {
    try 
    {
      return boost::lexical_cast<int>(Orthanc::Toolbox::StripSpaces(GetStringValue(tag)));
    }
    catch (boost::bad_lexical_cast&)
    {
      LOG(ERROR) << "Trying to access a DICOM tag that is not an integer";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
  }


  unsigned int DicomDataset::GetUnsignedIntegerValue(const Tag& tag) const
  {
    int v = GetIntegerValue(tag);

    if (v < 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
    else
    {
      return static_cast<unsigned int>(v);
    }
  }


  void DicomDataset::GetVectorValue(Vector& vector, 
                                    const Tag& tag) const
  {
    if (!GeometryToolbox::ParseVector(vector, Orthanc::Toolbox::StripSpaces(GetStringValue(tag))))
    {
      LOG(ERROR) << "Trying to access a DICOM tag that is not a vector";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
  }


  void DicomDataset::GetVectorValue(Vector& vector, 
                                    const Tag& tag,
                                    size_t expectedSize) const
  {
    GetVectorValue(vector, tag);

    if (vector.size() != expectedSize)
    {
      LOG(ERROR) << "A vector in a DICOM tag has a bad size";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
  }


  void DicomDataset::Print() const
  {
    for (Values::const_iterator it = values_.begin(); it != values_.end(); ++it)
    {
      printf("%04x,%04x = [%s]\n", it->first.first, it->first.second, it->second.c_str());
    }
    printf("\n");
  }


  bool DicomDataset::IsGrayscale() const
  {
    std::string photometric = Orthanc::Toolbox::StripSpaces(GetStringValue(DICOM_TAG_PHOTOMETRIC_INTERPRETATION));

    return (photometric == "MONOCHROME1" ||
            photometric == "MONOCHROME2");
  }


  void DicomDataset::GetPixelSpacing(double& spacingX,
                                     double& spacingY) const
  {
    if (HasTag(DICOM_TAG_PIXEL_SPACING))
    {
      Vector spacing;
      GetVectorValue(spacing, DICOM_TAG_PIXEL_SPACING, 2);
      spacingX = spacing[0];
      spacingY = spacing[1];
    }
    else
    {
      spacingX = 1.0;
      spacingY = 1.0;
    }
  }
}

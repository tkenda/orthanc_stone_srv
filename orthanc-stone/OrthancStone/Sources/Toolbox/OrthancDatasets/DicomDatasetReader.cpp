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


#include "DicomDatasetReader.h"

#include <OrthancException.h>
#include <Toolbox.h>

#include <boost/lexical_cast.hpp>

namespace OrthancStone
{
  DicomDatasetReader::DicomDatasetReader(const IDicomDataset& dataset) :
    dataset_(dataset)
  {
  }
  

  std::string DicomDatasetReader::GetStringValue(const Orthanc::DicomPath& path,
                                                 const std::string& defaultValue) const
  {
    std::string s;
    if (dataset_.GetStringValue(s, path))
    {
      return s;
    }
    else
    {
      return defaultValue;
    }
  }


  std::string DicomDatasetReader::GetMandatoryStringValue(const Orthanc::DicomPath& path) const
  {
    std::string s;
    if (dataset_.GetStringValue(s, path))
    {
      return s;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InexistentTag);
    }
  }


  template <typename T>
  static bool GetValueInternal(T& target,
                               const IDicomDataset& dataset,
                               const Orthanc::DicomPath& path)
  {
    try
    {
      std::string s;

      if (dataset.GetStringValue(s, path))
      {
        target = boost::lexical_cast<T>(Orthanc::Toolbox::StripSpaces(s));
        return true;
      }
      else
      {
        return false;
      }
    }
    catch (boost::bad_lexical_cast&)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);        
    }
  }


  bool DicomDatasetReader::GetIntegerValue(int& target,
                                           const Orthanc::DicomPath& path) const
  {
    return GetValueInternal<int>(target, dataset_, path);
  }


  bool DicomDatasetReader::GetUnsignedIntegerValue(unsigned int& target,
                                                   const Orthanc::DicomPath& path) const
  {
    int value;

    if (!GetIntegerValue(value, path))
    {
      return false;
    }
    else if (value >= 0)
    {
      target = static_cast<unsigned int>(value);
      return true;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  bool DicomDatasetReader::GetFloatValue(float& target,
                                         const Orthanc::DicomPath& path) const
  {
    return GetValueInternal<float>(target, dataset_, path);
  }


  bool DicomDatasetReader::GetDoubleValue(double& target,
                                          const Orthanc::DicomPath& path) const
  {
    return GetValueInternal<double>(target, dataset_, path);
  }
}

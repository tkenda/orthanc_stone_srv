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


#include "LookupTableStyleConfigurator.h"

#include <OrthancException.h>

namespace OrthancStone
{
  static void StringToVector(std::vector<uint8_t>& target,
    const std::string& source)
  {
    target.resize(source.size());

    for (size_t i = 0; i < source.size(); i++)
    {
      target[i] = source[i];
    }
  }

  LookupTableStyleConfigurator::LookupTableStyleConfigurator() :
    revision_(0),
    hasLut_(false),
    hasRange_(false),
    minValue_(0),
    maxValue_(0),
    applyLog_(false)
  {
  }

  void LookupTableStyleConfigurator::SetLookupTable(const std::vector<uint8_t>& lut)
  {
    hasLut_ = true;
    lut_ = lut;
    revision_++;
  }

  void LookupTableStyleConfigurator::SetLookupTable(const std::string& lut)
  {
    std::vector<uint8_t> tmp;
    StringToVector(tmp, lut);
    SetLookupTable(tmp);
  }

  void LookupTableStyleConfigurator::SetRange(float minValue,
                                              float maxValue)
  {
    if (minValue > maxValue)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      if ((!hasRange_) || (minValue_ != minValue) || (maxValue_ != maxValue))
        revision_++;
      hasRange_ = true;
      minValue_ = minValue;
      maxValue_ = maxValue;
    }
  }

  void LookupTableStyleConfigurator::SetApplyLog(bool apply)
  {
    applyLog_ = apply;
    revision_++;
  }

  TextureBaseSceneLayer* LookupTableStyleConfigurator::CreateTextureFromImage(const Orthanc::ImageAccessor& image) const
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
  }

  void LookupTableStyleConfigurator::ApplyStyle(ISceneLayer& layer) const
  {
    LookupTableTextureSceneLayer& l = dynamic_cast<LookupTableTextureSceneLayer&>(layer);
      
    if (hasLut_)
    {
      l.SetLookupTable(lut_);
    }

    if (hasRange_)
    {
      l.SetRange(minValue_, maxValue_);
    }
    else
    {
      l.FitRange();
    }

    l.SetApplyLog(applyLog_);
  }
}

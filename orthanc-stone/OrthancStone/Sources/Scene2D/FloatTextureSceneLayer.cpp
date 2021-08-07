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


#include "FloatTextureSceneLayer.h"

#include "../Toolbox/ImageToolbox.h"

#include <Images/Image.h>
#include <Images/ImageProcessing.h>
#include <OrthancException.h>

namespace OrthancStone
{
  FloatTextureSceneLayer::FloatTextureSceneLayer(const Orthanc::ImageAccessor& texture) :
    inverted_(false),
    applyLog_(false),
    isRangeComputed_(false),
    minValue_(0),
    maxValue_(0)
  {
    {
      std::unique_ptr<Orthanc::ImageAccessor> t(
        new Orthanc::Image(Orthanc::PixelFormat_Float32, 
                           texture.GetWidth(), 
                           texture.GetHeight(), 
                           false));

      Orthanc::ImageProcessing::Convert(*t, texture);

      SetTexture(t.release());
    }

    SetCustomWindowing(128, 256);
  }


  void FloatTextureSceneLayer::SetWindowing(ImageWindowing windowing)
  {
    if (windowing_ != windowing)
    {
      if (windowing == ImageWindowing_Custom)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }
      else
      {
        windowing_ = windowing;
        IncrementRevision();
      }
    }
  }

  void FloatTextureSceneLayer::SetCustomWindowing(float customCenter,
                                                  float customWidth)
  {
    if (customWidth <= 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      windowing_ = ImageWindowing_Custom;
      customCenter_ = customCenter;
      customWidth_ = customWidth;
      IncrementRevision();
    }
  }

  
  void FloatTextureSceneLayer::GetWindowing(float& targetCenter,
                                            float& targetWidth) const
  {
    ComputeWindowing(targetCenter, targetWidth,
                     windowing_, customCenter_, customWidth_);
  }


  void FloatTextureSceneLayer::SetInverted(bool inverted)
  {
    inverted_ = inverted;
    IncrementRevision();
  }


  void FloatTextureSceneLayer::SetApplyLog(bool apply)
  {
    applyLog_ = apply;
    IncrementRevision();
  }


  void FloatTextureSceneLayer::FitRange()
  {
    float minValue, maxValue;
    GetRange(minValue, maxValue);
    assert(minValue <= maxValue);

    float width;

    if (LinearAlgebra::IsCloseToZero(maxValue - minValue))
    {
      width = 1;
    }
    else
    {
      width = maxValue - minValue;
    }

    SetCustomWindowing((minValue + maxValue) / 2.0f, width);
  }

    
  void FloatTextureSceneLayer::GetRange(float& minValue,
                                        float& maxValue)
  {
    if (!isRangeComputed_)
    {
      Orthanc::ImageProcessing::GetMinMaxFloatValue(minValue_, maxValue_, GetTexture());
      isRangeComputed_ = true;
    }

    assert(minValue_ <= maxValue_);

    minValue = minValue_;
    maxValue = maxValue_;
  }


  ISceneLayer* FloatTextureSceneLayer::Clone() const
  {
    std::unique_ptr<FloatTextureSceneLayer> cloned
      (new FloatTextureSceneLayer(GetTexture()));

    cloned->CopyParameters(*this);
    cloned->windowing_ = windowing_;
    cloned->customCenter_ = customCenter_;
    cloned->customWidth_ = customWidth_;
    cloned->inverted_ = inverted_;
    cloned->applyLog_ = applyLog_;
    cloned->isRangeComputed_ = isRangeComputed_;
    cloned->minValue_ = minValue_;
    cloned->maxValue_ = maxValue_;

    return cloned.release();
  }
}

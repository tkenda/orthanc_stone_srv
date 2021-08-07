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


#include "GrayscaleStyleConfigurator.h"

#include "FloatTextureSceneLayer.h"

#include <OrthancException.h>

namespace OrthancStone
{
  GrayscaleStyleConfigurator::GrayscaleStyleConfigurator() :
    revision_(0),
    linearInterpolation_(false),
    hasWindowingOverride_(false),
    windowing_(ImageWindowing_Custom),
    customWindowWidth_(0),
    customWindowCenter_(0),
    hasInversionOverride_(false),
    inverted_(false),
    applyLog_(false)
  {
  }

  void GrayscaleStyleConfigurator::SetWindowing(ImageWindowing windowing)
  {
    hasWindowingOverride_ = true;
    windowing_ = windowing;
    revision_++;
  }

  void GrayscaleStyleConfigurator::SetCustomWindowing(float windowCenter, float windowWidth)
  {
    SetWindowing(ImageWindowing_Custom);
    customWindowCenter_ = windowCenter;
    customWindowWidth_ = windowWidth;
  }

  void GrayscaleStyleConfigurator::GetCustomWindowing(float& windowCenter, float& windowWidth) const
  {
    windowCenter = customWindowCenter_;
    windowWidth  = customWindowWidth_;
  }

  void GrayscaleStyleConfigurator::SetInverted(bool inverted)
  {
    hasInversionOverride_ = true;
    inverted_ = inverted;
    revision_++;
  }

  void GrayscaleStyleConfigurator::SetLinearInterpolation(bool enabled)
  {
    linearInterpolation_ = enabled;
    revision_++;
  }

  void GrayscaleStyleConfigurator::SetApplyLog(bool apply)
  {
    applyLog_ = apply;
    revision_++;
  }

  TextureBaseSceneLayer* GrayscaleStyleConfigurator::CreateTextureFromImage(
    const Orthanc::ImageAccessor& image) const
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
  }

  TextureBaseSceneLayer* GrayscaleStyleConfigurator::CreateTextureFromDicom(
    const Orthanc::ImageAccessor& frame,
    const DicomInstanceParameters& parameters) const
  {
    std::unique_ptr<TextureBaseSceneLayer> layer(parameters.CreateTexture(frame));

    if (layer.get() == NULL ||
        layer->GetTexture().GetFormat() != Orthanc::PixelFormat_Float32)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }
    else
    {
      return layer.release();
    }
  }

  void GrayscaleStyleConfigurator::ApplyStyle(ISceneLayer& layer) const
  {
    FloatTextureSceneLayer& l = dynamic_cast<FloatTextureSceneLayer&>(layer);
    
    l.SetLinearInterpolation(linearInterpolation_);

    if (hasWindowingOverride_)
    {
      if (windowing_ != ImageWindowing_Custom)
      {
        l.SetWindowing(windowing_);
      }
      else
      {
        l.SetCustomWindowing(customWindowCenter_, customWindowWidth_);
      }
    }

    if (hasInversionOverride_)
    {
      l.SetInverted(inverted_);
    }

    l.SetApplyLog(applyLog_);
  }
}

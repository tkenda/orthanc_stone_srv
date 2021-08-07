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


#pragma once

#include "TextureBaseSceneLayer.h"

namespace OrthancStone
{
  class FloatTextureSceneLayer : public TextureBaseSceneLayer
  {
  private:
    ImageWindowing   windowing_;
    float            customCenter_;
    float            customWidth_;
    bool             inverted_;
    bool             applyLog_;
    bool             isRangeComputed_;
    float            minValue_;
    float            maxValue_;

  public:
    // The pixel format must be convertible to "Float32"
    explicit FloatTextureSceneLayer(const Orthanc::ImageAccessor& texture);

    void SetWindowing(ImageWindowing windowing);

    void SetCustomWindowing(float customCenter,
                            float customWidth);

    void GetWindowing(float& targetCenter,
                      float& targetWidth) const;

    ImageWindowing GetWindowingType() const
    {
      return windowing_;
    }

    // To achieve MONOCHROME1 photometric interpretation
    void SetInverted(bool inverted);

    bool IsInverted() const
    {
      return inverted_;
    }

    void FitRange();

    void SetApplyLog(bool apply);

    bool IsApplyLog() const
    {
      return applyLog_;
    }

    void GetRange(float& minValue,
                  float& maxValue);

    virtual ISceneLayer* Clone() const ORTHANC_OVERRIDE;

    virtual Type GetType() const ORTHANC_OVERRIDE
    {
      return Type_FloatTexture;
    }
  };
}

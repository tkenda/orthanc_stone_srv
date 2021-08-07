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
  class LookupTableTextureSceneLayer : public TextureBaseSceneLayer
  {
  private:
    ImageWindowing        windowing_;
    float                 minValue_;
    float                 maxValue_;
    std::vector<uint8_t>  lut_;
    bool                  applyLog_;

    void SetLookupTableRgb(const std::vector<uint8_t>& lut);

  public:
    // The pixel format must be convertible to Float32
    explicit LookupTableTextureSceneLayer(const Orthanc::ImageAccessor& texture);

    void SetLookupTableGrayscale();

    // The vector must contain either 3 * 256 values (RGB), or 4 * 256
    // (RGBA). In the RGB case, an alpha channel will be automatically added.
    void SetLookupTable(const std::vector<uint8_t>& lut);

    void SetRange(float minValue,
                  float maxValue);
    
    void FitRange();

    float GetMinValue() const
    {
      return minValue_;
    }

    float GetMaxValue() const
    {
      return maxValue_;
    }

    // This returns a vector of 4 * 256 values between 0 and 255, in RGBA.
    const std::vector<uint8_t>& GetLookupTable() const
    {
      return lut_;
    }

    void SetApplyLog(bool apply);

    bool IsApplyLog() const
    {
      return applyLog_;
    }

    virtual ISceneLayer* Clone() const ORTHANC_OVERRIDE;

    virtual Type GetType() const ORTHANC_OVERRIDE
    {
      return Type_LookupTableTexture;
    }

    // Render the texture to a color image of format BGRA32 (Cairo
    // surfaces) or RGBA32 (OpenGL)
    void Render(Orthanc::ImageAccessor& target) const;
  };
}

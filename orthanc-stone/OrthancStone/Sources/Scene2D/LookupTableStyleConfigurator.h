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

#include "ILayerStyleConfigurator.h"

namespace OrthancStone
{
  /**
  This configurator supplies an API to set a display range and a LUT.
  */
  class LookupTableStyleConfigurator : public ILayerStyleConfigurator
  {
  private:
    uint64_t              revision_;
    bool                  hasLut_;
    std::vector<uint8_t>  lut_;
    bool                  hasRange_;
    float                 minValue_;
    float                 maxValue_;
    bool                  applyLog_;

  public:
    LookupTableStyleConfigurator();

    void SetLookupTable(const std::string& lut);

    /**
     See the SetLookupTable(const std::vector<uint8_t>& lut) method in the
     LookupTableTextureSceneLayer class.
     */
    void SetLookupTable(const std::vector<uint8_t>& lut);

    void SetRange(float minValue, float maxValue);

    void SetApplyLog(bool apply);

    bool IsApplyLog() const
    {
      return applyLog_;
    }
    
    virtual uint64_t GetRevision() const ORTHANC_OVERRIDE
    {
      return revision_;
    }

    virtual TextureBaseSceneLayer* CreateTextureFromImage(
      const Orthanc::ImageAccessor& image) const ORTHANC_OVERRIDE;

    virtual TextureBaseSceneLayer* CreateTextureFromDicom(
      const Orthanc::ImageAccessor& frame,
      const DicomInstanceParameters& parameters) const ORTHANC_OVERRIDE
    {
      return parameters.CreateLookupTableTexture(frame);
    }

    virtual void ApplyStyle(ISceneLayer& layer) const ORTHANC_OVERRIDE;
  };
}

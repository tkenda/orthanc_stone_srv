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
  class CopyStyleConfigurator : public ILayerStyleConfigurator
  {
  public:
    virtual uint64_t GetRevision() const ORTHANC_OVERRIDE
    {
      return 0; // No parameter for this type of configurator
    }
    
    virtual TextureBaseSceneLayer* CreateTextureFromImage(
      const Orthanc::ImageAccessor& image) const ORTHANC_OVERRIDE;

    virtual TextureBaseSceneLayer* CreateTextureFromDicom(
      const Orthanc::ImageAccessor& frame,
      const DicomInstanceParameters& parameters) const ORTHANC_OVERRIDE
    {
      return CreateTextureFromImage(frame);
    }

    virtual void ApplyStyle(ISceneLayer& layer) const ORTHANC_OVERRIDE
    {
    }
  };
}

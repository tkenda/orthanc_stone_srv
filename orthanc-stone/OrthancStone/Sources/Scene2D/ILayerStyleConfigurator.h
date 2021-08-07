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

#include "../Toolbox/DicomInstanceParameters.h"

namespace OrthancStone
{
  /**
  This interface is implemented by objects able to create an ISceneLayer 
  suitable to display the Orthanc image supplied to the CreateTextureXX 
  factory methods (taking Dicom parameters into account if relevant).

  It can also refresh the style of an existing layer afterwards, to match
  the configurator settings.
  */
  class ILayerStyleConfigurator
  {
  public:
    virtual ~ILayerStyleConfigurator()
    {
    }
    
    virtual uint64_t GetRevision() const = 0;
    
    virtual TextureBaseSceneLayer* CreateTextureFromImage(const Orthanc::ImageAccessor& image) const = 0;

    virtual TextureBaseSceneLayer* CreateTextureFromDicom(const Orthanc::ImageAccessor& frame,
                                                          const DicomInstanceParameters& parameters) const = 0;

    virtual void ApplyStyle(ISceneLayer& layer) const = 0;
  };
}

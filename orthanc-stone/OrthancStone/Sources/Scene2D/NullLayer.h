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

#include "ISceneLayer.h"

#include <Enumerations.h>

#include <stdint.h>

/**
 This layer can be used when a z-index needs to be booked inside a Scene2D.

 It can later be replaced by the actual layer.
*/
namespace OrthancStone
{
  class NullLayer : public ISceneLayer
  {
  public:
    NullLayer()
    {
    }

    virtual ISceneLayer* Clone() const ORTHANC_OVERRIDE
    {
      return new NullLayer();
    }

    virtual Type GetType() const ORTHANC_OVERRIDE
    {
      return Type_NullLayer;
    }

    virtual void GetBoundingBox(Extent2D& target) const ORTHANC_OVERRIDE
    {
      target.Clear();
    }

    virtual uint64_t GetRevision() const ORTHANC_OVERRIDE
    {
      return 0;
    }
  };
}

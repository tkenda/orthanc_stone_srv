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


#include "IVolumeSlicer.h"

#include <Logging.h>
#include <OrthancException.h>

namespace OrthancStone
{
  uint64_t IVolumeSlicer::InvalidSlice::GetRevision()
  {
    LOG(ERROR) << "IVolumeSlicer::InvalidSlice::GetRevision()";
    throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
  }

  ISceneLayer* IVolumeSlicer::InvalidSlice::CreateSceneLayer(const ILayerStyleConfigurator* configurator,
                                                             const CoordinateSystem3D& cuttingPlane)
  {
    LOG(ERROR) << "IVolumeSlicer::InvalidSlice::CreateSceneLayer()";
    throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
  }
}

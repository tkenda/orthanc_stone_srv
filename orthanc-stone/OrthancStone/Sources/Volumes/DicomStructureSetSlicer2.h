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

#ifdef BGO_ENABLE_DICOMSTRUCTURESETLOADER2

#include "../Toolbox/DicomStructureSet2.h"
#include "../Volumes/IVolumeSlicer.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace OrthancStone
{
  class DicomStructureSetSlice2 : public IVolumeSlicer::IExtractedSlice
  {
  public:
    DicomStructureSetSlice2(
      boost::weak_ptr<DicomStructureSet2> structureSet,
      uint64_t revision,
      const CoordinateSystem3D& cuttingPlane);

    virtual bool IsValid() ORTHANC_OVERRIDE
    {
      return isValid_;
    }

    virtual uint64_t GetRevision() ORTHANC_OVERRIDE
    {
      return revision_;
    }

    virtual ISceneLayer* CreateSceneLayer(
      const ILayerStyleConfigurator* configurator,  // possibly absent
      const CoordinateSystem3D& cuttingPlane) ORTHANC_OVERRIDE;

  private:
    boost::shared_ptr<DicomStructureSet2> structureSet_;
    bool isValid_;
    uint64_t revision_;
  };

  class DicomStructureSetSlicer2 : public IVolumeSlicer
  {
  public:
    DicomStructureSetSlicer2(boost::shared_ptr<DicomStructureSet2> structureSet);

    /** IVolumeSlicer impl */
    virtual IExtractedSlice* ExtractSlice(const CoordinateSystem3D& cuttingPlane) ORTHANC_OVERRIDE;
  private:
    boost::weak_ptr<DicomStructureSet2> structureSet_;
  };
}

#endif 
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2


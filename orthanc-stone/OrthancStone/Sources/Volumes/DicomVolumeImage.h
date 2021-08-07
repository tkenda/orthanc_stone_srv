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

#include "../Messages/IMessage.h"
#include "../Toolbox/DicomInstanceParameters.h"
#include "ImageBuffer3D.h"
#include "VolumeImageGeometry.h"

namespace OrthancStone
{
  /**
  This class combines a 3D image buffer, a 3D volume geometry and
  information about the DICOM parameters of the series.
  (MPR means MultiPlanar Reconstruction)
  */ 
  class DicomVolumeImage : public boost::noncopyable
  {
  public:
    // TODO - Are these messages still useful?
    ORTHANC_STONE_DEFINE_ORIGIN_MESSAGE(__FILE__, __LINE__, GeometryReadyMessage, DicomVolumeImage);
    ORTHANC_STONE_DEFINE_ORIGIN_MESSAGE(__FILE__, __LINE__, ContentUpdatedMessage, DicomVolumeImage);

  private:
    uint64_t                                revision_;
    std::unique_ptr<VolumeImageGeometry>      geometry_;
    std::unique_ptr<ImageBuffer3D>            image_;
    std::unique_ptr<DicomInstanceParameters>  parameters_;

    void CheckHasGeometry() const;
    
  public:
    DicomVolumeImage() :
      revision_(0)
    {
    }

    void IncrementRevision()
    {
      revision_ ++;
    }

    void Initialize(const VolumeImageGeometry& geometry,
                    Orthanc::PixelFormat format, 
                    bool computeRange = false);

    // Used by volume slicers
    void SetDicomParameters(const DicomInstanceParameters& parameters);
    
    uint64_t GetRevision() const
    {
      return revision_;
    }

    bool HasGeometry() const;

    ImageBuffer3D& GetPixelData();

    const ImageBuffer3D& GetPixelData() const;

    const VolumeImageGeometry& GetGeometry() const;

    bool HasDicomParameters() const
    {
      return parameters_.get() != NULL;
    }      

    const DicomInstanceParameters& GetDicomParameters() const;
  };
}

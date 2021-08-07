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


#include "DicomVolumeImageMPRSlicer.h"

#include "../StoneException.h"

#include "../Toolbox/ImageToolbox.h"

#include <Logging.h>
#include <OrthancException.h>

namespace OrthancStone
{
  void DicomVolumeImageMPRSlicer::Slice::CheckValid() const
  {
    if (!valid_)
    {
      LOG(ERROR) << "DicomVolumeImageMPRSlicer::Slice::CheckValid(): (!valid_)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  DicomVolumeImageMPRSlicer::Slice::Slice(const DicomVolumeImage& volume,
                                          const CoordinateSystem3D& cuttingPlane) :
    volume_(volume),
    revision_(volume_.GetRevision())
  {
    valid_ = (volume_.HasDicomParameters() &&
              volume_.GetGeometry().DetectSlice(projection_, sliceIndex_, cuttingPlane));
  }


  VolumeProjection DicomVolumeImageMPRSlicer::Slice::GetProjection() const
  {
    CheckValid();
    return projection_;
  }


  unsigned int DicomVolumeImageMPRSlicer::Slice::GetSliceIndex() const
  {
    CheckValid();
    return sliceIndex_;
  }
  

  ISceneLayer* DicomVolumeImageMPRSlicer::Slice::CreateSceneLayer(const ILayerStyleConfigurator* configurator,
                                                                  const CoordinateSystem3D& cuttingPlane)
  {
    CheckValid();

    if (configurator == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer,
                                      "A style configurator is mandatory for textures");
    }

    std::unique_ptr<TextureBaseSceneLayer> texture;
      
    {
      const DicomInstanceParameters& parameters = volume_.GetDicomParameters();
      ImageBuffer3D::SliceReader reader(volume_.GetPixelData(), projection_, sliceIndex_);

      texture.reset(dynamic_cast<TextureBaseSceneLayer*>
                    (configurator->CreateTextureFromDicom(reader.GetAccessor(), parameters)));

      if (texture.get() == NULL)
      {
        return NULL;
      }
    }
    
    const CoordinateSystem3D& system = volume_.GetGeometry().GetProjectionGeometry(projection_);

    Vector pixelSpacing = volume_.GetGeometry().GetVoxelDimensions(projection_);

    texture->SetCuttingPlaneTransform(cuttingPlane, system.GetOrigin(),
                                      system.GetAxisX() * pixelSpacing[0],
                                      system.GetAxisY() * pixelSpacing[1]);

    return texture.release();
  }


  DicomVolumeImageMPRSlicer::~DicomVolumeImageMPRSlicer()
  {
    LOG(TRACE) << "DicomVolumeImageMPRSlicer::~DicomVolumeImageMPRSlicer()";
  }

  IVolumeSlicer::IExtractedSlice*
  DicomVolumeImageMPRSlicer::ExtractSlice(const CoordinateSystem3D& cuttingPlane)
  {
    if (volume_->HasGeometry())
    {
      return new Slice(*volume_, cuttingPlane);
    }
    else
    {
      return new IVolumeSlicer::InvalidSlice;
    }
  }
}

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


#include "DicomVolumeImageReslicer.h"

#include <OrthancException.h>

namespace OrthancStone
{
  class DicomVolumeImageReslicer::Slice : public IVolumeSlicer::IExtractedSlice
  {
  private:
    DicomVolumeImageReslicer&  that_;
    CoordinateSystem3D         cuttingPlane_;
      
  public:
    Slice(DicomVolumeImageReslicer& that,
          const CoordinateSystem3D& cuttingPlane) :
      that_(that),
      cuttingPlane_(cuttingPlane)
    {
    }
      
    virtual bool IsValid() ORTHANC_OVERRIDE
    {
      return true;
    }

    virtual uint64_t GetRevision() ORTHANC_OVERRIDE
    {
      return that_.volume_->GetRevision();
    }

    virtual ISceneLayer* CreateSceneLayer(const ILayerStyleConfigurator* configurator,
                                          const CoordinateSystem3D& cuttingPlane) ORTHANC_OVERRIDE
    {
      VolumeReslicer& reslicer = that_.reslicer_;
        
      if (configurator == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,
                                        "Must provide a layer style configurator");
      }

      reslicer.SetOutputFormat(that_.volume_->GetPixelData().GetFormat());
      reslicer.Apply(that_.volume_->GetPixelData(),
                     that_.volume_->GetGeometry(),
                     cuttingPlane);

      if (reslicer.IsSuccess())
      {
        std::unique_ptr<TextureBaseSceneLayer> texture
          (configurator->CreateTextureFromDicom(reslicer.GetOutputSlice(),
                                                that_.volume_->GetDicomParameters()));
        if (texture.get() == NULL)
        {
          return NULL;
        }

        const double s = reslicer.GetPixelSpacing();
        
        const double x1 = reslicer.GetOutputExtent().GetX1();
        const double y1 = reslicer.GetOutputExtent().GetY1();
        const Vector p1 = cuttingPlane.MapSliceToWorldCoordinates(x1, y1);        

        // The "0.5" shift is to move from the corner of voxel to the center of the voxel
        
        texture->SetCuttingPlaneTransform(
          cuttingPlane, p1 + 0.5 * s * cuttingPlane.GetAxisX() + 0.5 * s * cuttingPlane.GetAxisY(),
          s * cuttingPlane.GetAxisX(),
          s * cuttingPlane.GetAxisY());

        return texture.release();
      }
      else
      {
        return NULL;
      }          
    }
  };
    

  DicomVolumeImageReslicer::DicomVolumeImageReslicer(const boost::shared_ptr<DicomVolumeImage>& volume) :
    volume_(volume)
  {
    if (volume.get() == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
  }

    
  IVolumeSlicer::IExtractedSlice* DicomVolumeImageReslicer::ExtractSlice(const CoordinateSystem3D& cuttingPlane)
  {
    if (volume_->HasGeometry())
    {
      return new Slice(*this, cuttingPlane);
    }
    else
    {
      return new IVolumeSlicer::InvalidSlice;
    }
  }
}

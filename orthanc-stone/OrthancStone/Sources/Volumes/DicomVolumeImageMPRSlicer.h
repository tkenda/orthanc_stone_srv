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

#include "DicomVolumeImage.h"
#include "IVolumeSlicer.h"

#include <boost/shared_ptr.hpp>

namespace OrthancStone
{
  /**
     Implements the IVolumeSlicer on Dicom volume data when the cutting plane
     that is supplied to the slicer is either axial, sagittal or coronal. 
     Arbitrary planes are *not* supported
  */
  class DicomVolumeImageMPRSlicer : public IVolumeSlicer
  {
  public:
    class Slice : public IExtractedSlice
    {
    private:
      const DicomVolumeImage&  volume_;
      uint64_t                 revision_;
      bool                     valid_;
      VolumeProjection         projection_;
      unsigned int             sliceIndex_;

      void CheckValid() const;

    public:
      /**
         Represents a slice of a volume image that is parallel to the 
         coordinate system axis. 
         The constructor initializes the type of projection (axial, sagittal or
         coronal) and the corresponding slice index, from the cutting plane.
      */
      Slice(const DicomVolumeImage& volume,
            const CoordinateSystem3D& cuttingPlane);

      void SetRevision(uint64_t revision)
      {
        revision_ = revision;
      }

      VolumeProjection GetProjection() const;

      unsigned int GetSliceIndex() const;

      virtual bool IsValid()
      {
        return valid_;
      }

      virtual uint64_t GetRevision()
      {
        return revision_;
      }

      virtual ISceneLayer* CreateSceneLayer(const ILayerStyleConfigurator* configurator,
                                            const CoordinateSystem3D& cuttingPlane);
    };

  private:
    boost::shared_ptr<DicomVolumeImage>  volume_;

  public:
    explicit DicomVolumeImageMPRSlicer(const boost::shared_ptr<DicomVolumeImage>& volume) :
      volume_(volume)
    {
    }

    boost::shared_ptr<const DicomVolumeImage> GetVolume() const
    {
      return volume_;
    }

    virtual ~DicomVolumeImageMPRSlicer();

    virtual IExtractedSlice* ExtractSlice(const CoordinateSystem3D& cuttingPlane) ORTHANC_OVERRIDE;
  };
}

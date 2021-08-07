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


#include "DicomVolumeImage.h"

#include <Logging.h>
#include <OrthancException.h>


namespace OrthancStone
{
  void DicomVolumeImage::CheckHasGeometry() const
  {
    if (!HasGeometry())
    {
      LOG(ERROR) << "DicomVolumeImage::CheckHasGeometry(): (!HasGeometry())";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }
    

  void DicomVolumeImage::Initialize(
    const VolumeImageGeometry& geometry,
    Orthanc::PixelFormat format, 
    bool computeRange)
  {
    geometry_.reset(new VolumeImageGeometry(geometry));
    image_.reset(new ImageBuffer3D(format, geometry_->GetWidth(), geometry_->GetHeight(),
                                   geometry_->GetDepth(), computeRange));

    revision_ ++;
  }


  void DicomVolumeImage::SetDicomParameters(const DicomInstanceParameters& parameters)
  {
    parameters_.reset(parameters.Clone());
    revision_ ++;
  }
    

  bool DicomVolumeImage::HasGeometry() const
  {
    return (geometry_.get() != NULL &&
            image_.get() != NULL);
  }


  ImageBuffer3D& DicomVolumeImage::GetPixelData()
  {
    CheckHasGeometry();
    return *image_;
  }


  const ImageBuffer3D& DicomVolumeImage::GetPixelData() const
  {
    CheckHasGeometry();
    return *image_;
  }


  const VolumeImageGeometry& DicomVolumeImage::GetGeometry() const
  {
    CheckHasGeometry();
    return *geometry_;
  }


  const DicomInstanceParameters& DicomVolumeImage::GetDicomParameters() const
  {
    if (HasDicomParameters())
    {
      return *parameters_;
    }
    else
    {
      LOG(ERROR) << "DicomVolumeImage::GetDicomParameters(): (!HasDicomParameters())";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }      
  }
}

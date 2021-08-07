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
#include "VolumeReslicer.h"

#include <boost/shared_ptr.hpp>

namespace OrthancStone
{
  /**
  This class is able to supply an extract slice for an arbitrary cutting
  plane through a volume image
  */
  class DicomVolumeImageReslicer : public IVolumeSlicer
  {
  private:
    class Slice;
    
    boost::shared_ptr<DicomVolumeImage>  volume_;
    VolumeReslicer                       reslicer_;

  public:
    explicit DicomVolumeImageReslicer(const boost::shared_ptr<DicomVolumeImage>& volume);

    ImageInterpolation GetInterpolation() const
    {
      return reslicer_.GetInterpolation();
    }

    void SetInterpolation(ImageInterpolation interpolation)
    {
      reslicer_.SetInterpolation(interpolation);
    }

    bool IsFastMode() const
    {
      return reslicer_.IsFastMode();
    }

    void SetFastMode(bool fast)
    {
      reslicer_.EnableFastMode(fast);
    }
    
    virtual IExtractedSlice* ExtractSlice(const CoordinateSystem3D& cuttingPlane) ORTHANC_OVERRIDE;
  };
}

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

#include "../StoneEnumerations.h"
#include "../Toolbox/CoordinateSystem3D.h"

#include <iosfwd>

namespace OrthancStone
{
  class VolumeImageGeometry
  {
  private:
    unsigned int           width_;
    unsigned int           height_;
    unsigned int           depth_;
    CoordinateSystem3D     axialGeometry_;
    CoordinateSystem3D     coronalGeometry_;
    CoordinateSystem3D     sagittalGeometry_;
    Vector                 voxelDimensions_;
    Matrix                 transform_;
    Matrix                 transformInverse_;

    void Invalidate();

    friend std::ostream& operator<<(std::ostream& s, const VolumeImageGeometry& v);

  public:
    VolumeImageGeometry();

    unsigned int GetWidth() const
    {
      return width_;
    }

    unsigned int GetHeight() const
    {
      return height_;
    }

    unsigned int GetDepth() const
    {
      return depth_;
    }

    const CoordinateSystem3D& GetAxialGeometry() const
    {
      return axialGeometry_;
    }

    const CoordinateSystem3D& GetCoronalGeometry() const
    {
      return coronalGeometry_;
    }

    const CoordinateSystem3D& GetSagittalGeometry() const
    {
      return sagittalGeometry_;
    }

    const CoordinateSystem3D& GetProjectionGeometry(VolumeProjection projection) const;
    
    const Matrix& GetTransform() const
    {
      return transform_;
    }

    const Matrix& GetTransformInverse() const
    {
      return transformInverse_;
    }

    void SetSizeInVoxels(unsigned int width,
                         unsigned int height,
                         unsigned int depth);

    // Set the geometry of the first axial slice (i.e. the one whose
    // depth == 0)
    void SetAxialGeometry(const CoordinateSystem3D& geometry);

    void SetVoxelDimensions(double x,
                            double y,
                            double z);

    Vector GetVoxelDimensions(VolumeProjection projection) const;

    unsigned int GetProjectionWidth(VolumeProjection projection) const;

    unsigned int GetProjectionHeight(VolumeProjection projection) const;

    unsigned int GetProjectionDepth(VolumeProjection projection) const;

    // Get the 3D position of a point in the volume, where x, y and z
    // lie in the [0;1] range
    Vector GetCoordinates(float x,
                          float y,
                          float z) const;

    bool DetectProjection(VolumeProjection& projection,
                          bool& isOpposite,
                          const Vector& planeNormal) const;

    /**
    Being given a cutting plane, this method will determine if it is an
    axial, sagittal or coronal cut and returns 
    the slice number corresponding to this cut.

    If the cutting plane is not parallel to the three x = 0, y = 0 or z = 0
    planes, it is considered as arbitrary and the method returns false. 
    Otherwise, it returns true.
    */
    bool DetectSlice(VolumeProjection& projection,
                     unsigned int& slice,
                     const CoordinateSystem3D& plane) const;

    CoordinateSystem3D GetProjectionSlice(VolumeProjection projection,
                                          unsigned int z) const;
  };
}

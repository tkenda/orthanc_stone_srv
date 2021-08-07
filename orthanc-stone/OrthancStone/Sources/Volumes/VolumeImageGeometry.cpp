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


#include "VolumeImageGeometry.h"

#include "../Toolbox/GeometryToolbox.h"

#include <Logging.h>
#include <OrthancException.h>


namespace OrthancStone
{
  void VolumeImageGeometry::Invalidate()
  {
    Vector p = (axialGeometry_.GetOrigin() +
                static_cast<double>(depth_ - 1) * voxelDimensions_[2] * axialGeometry_.GetNormal());
        
    coronalGeometry_ = CoordinateSystem3D(p,
                                          axialGeometry_.GetAxisX(),
                                          -axialGeometry_.GetNormal());
    
    sagittalGeometry_ = CoordinateSystem3D(p,
                                           axialGeometry_.GetAxisY(),
                                           -axialGeometry_.GetNormal());

    Vector origin = (
      axialGeometry_.MapSliceToWorldCoordinates(-0.5 * voxelDimensions_[0],
                                                -0.5 * voxelDimensions_[1]) -
      0.5 * voxelDimensions_[2] * axialGeometry_.GetNormal());

    LOG(TRACE) << "VolumeImageGeometry::Invalidate() origin = " << origin(0) << "," << origin(1) << "," << origin(2) << " | width_ = " << width_ << " | height_ = " << height_ << " | depth_ = " << depth_;

    Vector scaling;
    
    if (width_ == 0 ||
        height_ == 0 ||
        depth_ == 0)
    {
      LinearAlgebra::AssignVector(scaling, 1, 1, 1);
    }
    else
    {
      scaling = (
        axialGeometry_.GetAxisX() * voxelDimensions_[0] * static_cast<double>(width_) +
        axialGeometry_.GetAxisY() * voxelDimensions_[1] * static_cast<double>(height_) +
        axialGeometry_.GetNormal() * voxelDimensions_[2] * static_cast<double>(depth_));
    }

    transform_ = LinearAlgebra::Product(
      GeometryToolbox::CreateTranslationMatrix(origin[0], origin[1], origin[2]),
      GeometryToolbox::CreateScalingMatrix(scaling[0], scaling[1], scaling[2]));

    LinearAlgebra::InvertMatrix(transformInverse_, transform_);
  }

  
  VolumeImageGeometry::VolumeImageGeometry() :
    width_(0),
    height_(0),
    depth_(0)
  {
    LinearAlgebra::AssignVector(voxelDimensions_, 1, 1, 1);
    Invalidate();
  }


  void VolumeImageGeometry::SetSizeInVoxels(unsigned int width,
                                    unsigned int height,
                                    unsigned int depth)
  {
    width_ = width;
    height_ = height;
    depth_ = depth;
    Invalidate();
  }

  
  void VolumeImageGeometry::SetAxialGeometry(const CoordinateSystem3D& geometry)
  {
    axialGeometry_ = geometry;
    Invalidate();
  }


  void VolumeImageGeometry::SetVoxelDimensions(double x,
                                               double y,
                                               double z)
  {
    if (x <= 0 ||
        y <= 0 ||
        z <= 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      LinearAlgebra::AssignVector(voxelDimensions_, x, y, z);
      Invalidate();
    }
  }


  const CoordinateSystem3D& VolumeImageGeometry::GetProjectionGeometry(VolumeProjection projection) const
  {
    switch (projection)
    {
      case VolumeProjection_Axial:
        return axialGeometry_;

      case VolumeProjection_Coronal:
        return coronalGeometry_;

      case VolumeProjection_Sagittal:
        return sagittalGeometry_;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }

  
  Vector VolumeImageGeometry::GetVoxelDimensions(VolumeProjection projection) const
  {
    switch (projection)
    {
      case VolumeProjection_Axial:
        return voxelDimensions_;

      case VolumeProjection_Coronal:
        return LinearAlgebra::CreateVector(voxelDimensions_[0], voxelDimensions_[2], voxelDimensions_[1]);

      case VolumeProjection_Sagittal:
        return LinearAlgebra::CreateVector(voxelDimensions_[1], voxelDimensions_[2], voxelDimensions_[0]);

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  unsigned int VolumeImageGeometry::GetProjectionWidth(VolumeProjection projection) const
  {
    switch (projection)
    {
      case VolumeProjection_Axial:
        return width_;

      case VolumeProjection_Coronal:
        return width_;

      case VolumeProjection_Sagittal:
        return height_;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  unsigned int VolumeImageGeometry::GetProjectionHeight(VolumeProjection projection) const
  {
    switch (projection)
    {
      case VolumeProjection_Axial:
        return height_;

      case VolumeProjection_Coronal:
        return depth_;

      case VolumeProjection_Sagittal:
        return depth_;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  unsigned int VolumeImageGeometry::GetProjectionDepth(VolumeProjection projection) const
  {
    switch (projection)
    {
      case VolumeProjection_Axial:
        return depth_;

      case VolumeProjection_Coronal:
        return height_;

      case VolumeProjection_Sagittal:
        return width_;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }    
  }


  Vector VolumeImageGeometry::GetCoordinates(float x,
                                             float y,
                                             float z) const
  {
    Vector p = LinearAlgebra::Product(transform_, LinearAlgebra::CreateVector(x, y, z, 1));

    assert(LinearAlgebra::IsNear(p[3], 1));  // Affine transform, no perspective effect

    // Back to non-homogeneous coordinates
    return LinearAlgebra::CreateVector(p[0], p[1], p[2]);
  }


  bool VolumeImageGeometry::DetectProjection(VolumeProjection& projection,
                                             bool& isOpposite,
                                             const Vector& planeNormal) const
  {
    if (GeometryToolbox::IsParallelOrOpposite(isOpposite, planeNormal, axialGeometry_.GetNormal()))
    {
      projection = VolumeProjection_Axial;
      return true;
    }
    else if (GeometryToolbox::IsParallelOrOpposite(isOpposite, planeNormal, coronalGeometry_.GetNormal()))
    {
      projection = VolumeProjection_Coronal;
      return true;
    }
    else if (GeometryToolbox::IsParallelOrOpposite(isOpposite, planeNormal, sagittalGeometry_.GetNormal()))
    {
      projection = VolumeProjection_Sagittal;
      return true;
    }
    else
    {
      return false;
    }
  }

  
  bool VolumeImageGeometry::DetectSlice(VolumeProjection& projection,
                                        unsigned int& slice,
                                        const CoordinateSystem3D& plane) const
  {
    bool isOpposite;
    if (!DetectProjection(projection, isOpposite, plane.GetNormal()))
    {
      return false;
    }

    // Transforms the coordinates of the origin of the plane, into the
    // coordinates of the axial geometry
    const Vector& origin = plane.GetOrigin();
    Vector p = LinearAlgebra::Product(
      transformInverse_,
      LinearAlgebra::CreateVector(origin[0], origin[1], origin[2], 1));

    assert(LinearAlgebra::IsNear(p[3], 1));

    double z;

    switch (projection)
    {
      case VolumeProjection_Axial:
        z = p[2];
        break;

      case VolumeProjection_Coronal:
        z = p[1];
        break;

      case VolumeProjection_Sagittal:
        z = p[0];
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    const unsigned int projectionDepth = GetProjectionDepth(projection);
    
    z *= static_cast<double>(projectionDepth);
    if (z < 0)
    {
      return false;
    }
    else
    {
      unsigned int d = static_cast<unsigned int>(std::floor(z));
      if (d >= projectionDepth)
      {
        return false;
      }
      else
      {
        slice = d;
        return true;
      }
    }
  }


  CoordinateSystem3D VolumeImageGeometry::GetProjectionSlice(VolumeProjection projection,
                                                             unsigned int z) const
  {
    if (z >= GetProjectionDepth(projection))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    Vector dim = GetVoxelDimensions(projection);
    CoordinateSystem3D plane = GetProjectionGeometry(projection);

    Vector normal = plane.GetNormal();
    if (projection == VolumeProjection_Sagittal)
    {
      /**
       * WARNING: In sagittal geometry, the normal points to REDUCING
       * X-axis in the 3D world. This is necessary to keep the
       * right-hand coordinate system. Hence the negation.
       **/
      normal = -normal;
    }
    
    plane.SetOrigin(plane.GetOrigin() + static_cast<double>(z) * dim[2] * normal);

    return plane;
  }

  std::ostream& operator<<(std::ostream& s, const VolumeImageGeometry& v)
  {
    s << "width: " << v.width_ << " height: " << v.height_
      << " depth: "             << v.depth_
      << " axialGeometry: "     << v.axialGeometry_
      << " coronalGeometry: "   << v.coronalGeometry_
      << " sagittalGeometry: "  << v.sagittalGeometry_
      << " voxelDimensions_: "  << v.voxelDimensions_
      << " height: "            << v.height_
      << " transform: "         << v.transform_
      << " transformInverse: "  << v.transformInverse_;
    return s;
  }

}
